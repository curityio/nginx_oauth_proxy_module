/*
 *  Copyright 2022 Curity AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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

/* Decoding ASCII table with valid entries for base64url
   
   In base64url:
   - Ascii 43 (+) is replaced with Ascii 45 (-)
   - Ascii 47 (/) is replaced with Ascii 95 (_)

   Therefore:
   - The Apache value from zero based position 43 was copied to byte 45, then byte 43 was set to 64
   - The Apache value from zero based position 47 was copied to byte 95, then byte 47 was set to 64
 */
static const unsigned char pr2six[256] =
{
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64,
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
 * Decode bytes from base64url
 */
int base64_url_decode(char *bufplain, const char *bufcoded)
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
    return nbytesdecoded;
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
            /* Invalid character */
            return 1;
        }
        
        if (i % 2 == 0)
        {
            /* Low order byte is set first */
            bytes[i / 2] = 16 * d;
        }
        else
        {
            /* High order byte is then added to the low order byte */
            bytes[i / 2] += d;
        }
    }

    return 0;
}
