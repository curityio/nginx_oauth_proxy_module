/* 
 * Adapted from the Apache open source repo:
 * https://svn.apache.org/repos/asf/apr/apr/trunk/encoding/apr_base64.c
 */

/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

/* For base64url use the URL and Filename Safe alphabet
   https://datatracker.ietf.org/doc/html/rfc4648#section-5 */
static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/* TODO: explain bytes properly

   Base64 alphabet: https://datatracker.ietf.org/doc/html/rfc4648#section-4

   Std base64:
   Ascii for + is 43
   Ascii for / is 47

   base64url:
   Ascii for - is 45
   Ascii for _ is 95

 * Characters of interest
   0=A
   25=Z
   26=a
   51=z
   52=0
   61=9
   62=+
   62=/
   64 is =
   Hence the mapping table
*/

/* TODO: explain table properly

   During decoding each byte value is looked up until value 64 (non base64url character) is found
   
   Zero based byte 45 (12th in row 3) below is ascii 45 '-', valid base64url so set to 62
   Byte 46 (14th in row 3) below is ascii 47, '/', not valid base64url so set to 64
   Byte 96 (16th in row 6) below is ascii '_', valid base64url so set to 63 */
static const unsigned char pr2six[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 63,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

/*
 * Calculate an encoded length from a decoded length
 */
int base64_url_encode_len(int len)
{
    // TODO: this is not right
    return ((len + 2) / 3 * 4) + 1;
}

/*
 * Encode bytes to base64url
 */
void base64_url_encode(char *encoded, const unsigned char *string, int len)
{
    int i = 0;
    char *p = NULL;

    p = encoded;
    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        *p++ = basis_64[((string[i] & 0x3) << 4) | ((int) (string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2) | ((int) (string[i + 2] & 0xC0) >> 6)];
        *p++ = basis_64[string[i + 2] & 0x3F];
    }
    
    if (i < len)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        if (i == (len - 1))
        {
            *p++ = basis_64[((string[i] & 0x3) << 4)];

            /* Do not pad base64url
            *p++ = '='; */
        }
        else
        {
            *p++ = basis_64[((string[i] & 0x3) << 4) | ((int) (string[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
        }

        /* Do not pad base64url
	    *p++ = '='; */
    }

    *p++ = '\0';
}

/*
 * Calculate a decoded length from a string
 */
int base64_url_decode_len(const char *bufcoded)
{
    int nbytesdecoded = 0;
    const unsigned char *bufin = NULL;
    int nprbytes = 0;

    bufin = (const unsigned char *) bufcoded;
    printf("start: %s\n", bufcoded);
    printf("*** char %d, mapped to %d\n", *bufin, pr2six[*bufin]);
    while (pr2six[*(bufin++)] <= 63)
        printf("*** char %d, mapped to %d\n", *bufin, pr2six[*bufin]);
    printf("*** end");

    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = (((int)nprbytes + 3) / 4) * 3;

    // TODO: explain truncation logic
    /* https://datatracker.ietf.org/doc/html/rfc4648#section-4 */
    if (nprbytes % 4 == 2)
    /*if (nprbytes % 3 == 1)*/
    {
        /* This compensates for the two == padding characters added to payloads of this form */
        //printf("nprbytes subtracting 2\n");
        nbytesdecoded -= 2;

    } else if (nprbytes % 4 == 3)
    /*} else if (nprbytes % 3 == 2)*/
    {
        /* This compensates for the = padding character added to payloads of this form */
        //printf("nprbytes subtracting 1\n");
        nbytesdecoded -= 1;
    }

    return nbytesdecoded + 1;
}

/*
 * Decode bytes from base64url
 */
void base64_url_decode(char *bufplain, const char *bufcoded)
{
    int nbytesdecoded = 0;
    const unsigned char *bufin = NULL;
    unsigned char *bufout = NULL;
    int nprbytes = 0;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63)
        ;
    
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = (((int)nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4)
    {
        *(bufout++) = (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) = (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) = (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if (nprbytes > 1)
    {
	    *(bufout++) = (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2)
    {
	    *(bufout++) = (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3)
    {
	    *(bufout++) = (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    nbytesdecoded -= (4 - (int)nprbytes) & 3;
    bufplain[nbytesdecoded] = '\0';
}

/*
 * Convert each pair of hex characters to a byte value
 */
int bytes_from_hex(unsigned char *bytes, const unsigned char *hex, size_t hex_len)
{
    size_t i = 0;
    char c = 0;
    unsigned char d = 0;

    if (hex_len %2 != 0)
    {
       return -1;
    }

    for (i = 0; i < hex_len; i++)
    {
        c = hex[i];
        if (c >= '0' && c <= '9')
        {
            d = (c - '0');
        }
        else if (c >= 'A' && c <= 'F')
        {
            d = (10 + (c - 'A'));
        }
        else if (c >= 'a' && c <= 'f')
        {
            d = (10 + (c - 'a'));
        }
        else
        {
            // invalid character
            return 1;
        }
        
        if (i % 2 == 0)
        {
            // low order byte is set first
            bytes[i / 2] = 16 * d;
        }
        else
        {
            // high order byte is then added to the low order byte
            bytes[i / 2] += d;
        }
    }

    return 0;
}
