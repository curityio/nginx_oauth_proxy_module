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

#include <openssl/rand.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>

/* Forward declarations */
// static ngx_int_t hex_to_bytes(const char *hex, size_t hexLen, u_char *bytes);
static void bytes_to_hex(u_char *bytes, size_t bytesLen, u_char *hex);

/* Encryption related constants */
const int GCM_IV_SIZE = 12;
const int GCM_TAG_SIZE = 16;
const int AES_KEY_SIZE_BYTES = 32;

/*
 * Perform AES256-GCM cookie decryption
 */
ngx_int_t oauth_proxy_decrypt(ngx_http_request_t *request, ngx_str_t *output, const ngx_str_t* input)
{
    u_char bytes[16];
    size_t alloc_size = sizeof(bytes) * 2 + 1;
    output->data = ngx_pcalloc(request->pool, alloc_size);
    RAND_bytes(bytes, sizeof(bytes));
    
    bytes_to_hex(bytes, sizeof(bytes), output->data);
    output->len = ngx_strlen(output->data);
    return NGX_OK;
}

/*
 * Convert each pair of hex characters to a byte value
 */
/*static ngx_int_t hex_to_bytes(const char *hex, size_t hexLen, u_char *bytes)
{
    if (hexLen %2 != 0)
    {
       return NGX_HTTP_UNAUTHORIZED;
    }

    for (size_t i = 0; i < hexLen; i++)
    {
        char c = hex[i];
        u_char d;

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
            // high order byte is then added
            bytes[i / 2] += d;
        }
    }

    return NGX_OK;
}*/

/*
 * Convert each byte to a pair of hex characters, and note that the last element adds the null terminator
 */
static void bytes_to_hex(u_char *bytes, size_t bytes_size, u_char *hex)
{
    u_char *pos = hex;
    for (size_t i = 0; i < bytes_size; i++)
    {
        sprintf((char *)pos, "%02x", bytes[i]);
        pos += 2;
    }
}
