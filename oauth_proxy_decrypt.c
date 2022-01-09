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

#include <openssl/evp.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>

/* Forward declarations */
static ngx_int_t bytes_from_hex(u_char *bytes, const u_char *hex, size_t hex_len);

/* Encryption related constants */
const int GCM_IV_SIZE = 12;
const int GCM_TAG_SIZE = 16;
const int AES_KEY_SIZE_BYTES = 32;

/*
 * Performs AES256-GCM authenticated decryption of secure cookies, using the hex encryption key from configuration
 * https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
 */
ngx_int_t oauth_proxy_decrypt(ngx_http_request_t *request, const ngx_str_t *encryption_key_hex, const ngx_str_t *encrypted_hex, ngx_str_t *plaintext)
{
    EVP_CIPHER_CTX *ctx = NULL;
    u_char encryption_key_bytes[AES_KEY_SIZE_BYTES];
    u_char *ciphertext_bytes = NULL;
    u_char *plaintext_bytes = NULL;
    u_char iv_hex[GCM_IV_SIZE * 2 + 1];
    u_char iv_bytes[GCM_IV_SIZE];
    u_char tag_hex[GCM_TAG_SIZE * 2 + 1];
    u_char tag_bytes[GCM_TAG_SIZE];
    int ciphertext_len = 0;
    int plaintext_len  = 0;
    int len            = 0;
    int evp_result     = 0;
    ngx_int_t ret_code = NGX_OK;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Unable to create the decryption cipher");
        ret_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ret_code == NGX_OK)
    {
        ret_code = bytes_from_hex(encryption_key_bytes, encryption_key_hex->data, AES_KEY_SIZE_BYTES * 2);
        if (ret_code != NGX_OK)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The configured encryption key is not valid hex");
        }
    }

    if (ret_code == NGX_OK)
    {
        ciphertext_len = encrypted_hex->len / 2 - (GCM_IV_SIZE + GCM_TAG_SIZE);
        if (ciphertext_len <= 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The encrypted hex payload had an invalid length");
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    if (ret_code == NGX_OK)
    {
        ciphertext_bytes = ngx_pcalloc(request->pool, ciphertext_len);
        if (ciphertext_bytes == NULL)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered allocating memory for ciphertext bytes");
            ret_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    // In AES-GCM the plaintext and ciphertext sizes are the same but we add a character for the null terminator
    if (ret_code == NGX_OK)
    {
        plaintext_bytes = ngx_pcalloc(request->pool, ciphertext_len + 1);
        if (plaintext_bytes == NULL)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered allocating memory for plaintext bytes");
            ret_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    
    // The IV part of the payload is the first 12 hex characters
    if (ret_code == NGX_OK)
    {
        ngx_memcpy(iv_hex, encrypted_hex->data, GCM_IV_SIZE * 2);
        iv_hex[GCM_IV_SIZE * 2] = 0;
        ret_code = bytes_from_hex(iv_bytes, iv_hex, GCM_IV_SIZE * 2);
        if (ret_code != NGX_OK)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The IV section of the encrypted payload is not valid hex");
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    // The actual ciphertext is the large middle part of the payload
    if (ret_code == NGX_OK)
    {   
        ret_code = bytes_from_hex(ciphertext_bytes, encrypted_hex->data + GCM_IV_SIZE * 2, encrypted_hex->len - (GCM_IV_SIZE + GCM_TAG_SIZE) * 2);
        if (ret_code != NGX_OK)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The ciphertext section of the encrypted payload is not valid hex");
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    // The tag part of the payload is the last 16 hex characters
    if (ret_code == NGX_OK)
    {
        ngx_memcpy(tag_hex, encrypted_hex->data + encrypted_hex->len - GCM_TAG_SIZE * 2, GCM_TAG_SIZE * 2);
        tag_hex[GCM_TAG_SIZE * 2] = 0;
        ret_code = bytes_from_hex(tag_bytes, tag_hex, GCM_TAG_SIZE * 2);
        if (ret_code != NGX_OK)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The tag section of the encrypted payload is not valid hex");
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    if (ret_code == NGX_OK)
    {
        evp_result = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, encryption_key_bytes, iv_bytes);
        if (evp_result == 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Unable to initialize the decryption context, error number: %d", evp_result);
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    if (ret_code == NGX_OK)
    {
        evp_result = EVP_DecryptUpdate(ctx, plaintext_bytes, &len, ciphertext_bytes, ciphertext_len);
        if (evp_result == 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered decrypting data, error number: %d", evp_result);
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
        else
        {
            plaintext_len = len;
        }
    }

    if (ret_code == NGX_OK)
    {
        evp_result = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag_bytes);
        if (evp_result == 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered setting the message authentication code, error number: %d", evp_result);
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    if (ret_code == NGX_OK)
    {
        evp_result = EVP_DecryptFinal_ex(ctx, plaintext_bytes + plaintext_len, &len);
        if (evp_result <= 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered finalizing encrypted data, error number: %d", evp_result);
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
        else
        {
            plaintext_len += len;
            plaintext_bytes[plaintext_len] = 0;
        }
    }

    if (ret_code == NGX_OK)
    {
        plaintext->data = plaintext_bytes;
        plaintext->len  = plaintext_len;
    }

    if (ctx != NULL)
    {
        EVP_CIPHER_CTX_free(ctx);
    }

    return ret_code;
}

/*
 * Convert each pair of hex characters to a byte value
 */
static ngx_int_t bytes_from_hex(u_char *bytes, const u_char *hex, size_t hex_len)
{
    if (hex_len %2 != 0)
    {
       return -1;
    }

    for (size_t i = 0; i < hex_len; i++)
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
            return NGX_ERROR;
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

    return NGX_OK;
}
