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

/* Imports from the encoding module */
int  bytes_from_hex(unsigned char *bytes, const unsigned char *hex, size_t hex_len);
int  base64_url_decode(char *bufplain, const char *bufcoded);

/* For encryption related constants to be used in array sizes, use #defines as valid C */
#define VERSION_SIZE 1
#define GCM_IV_SIZE 12
#define GCM_TAG_SIZE 16
#define AES_KEY_SIZE_BYTES 32
#define CURRENT_VERSION 1

/*
 * Performs AES256-GCM authenticated decryption of secure cookies, using the hex encryption key from configuration
 * https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
 */
ngx_int_t decrypt_cookie(ngx_http_request_t *request, ngx_str_t *plaintext, const ngx_str_t *ciphertext, const ngx_str_t *encryption_key_hex)
{
    EVP_CIPHER_CTX *ctx = NULL;
    u_char encryption_key_bytes[AES_KEY_SIZE_BYTES];
    u_char *ciphertext_bytes = NULL;
    u_char *plaintext_bytes = NULL;
    u_char iv_bytes[GCM_IV_SIZE];
    u_char tag_bytes[GCM_TAG_SIZE];
    int decoded_size = 0;
    int ciphertext_byte_size = 0;
    int plaintext_len  = 0;
    int offset = 0;
    int len = 0;
    int evp_result = 0;
    ngx_int_t ret_code = NGX_OK;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Unable to create the decryption cipher");
        ret_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ret_code == NGX_OK)
    {
        ret_code = bytes_from_hex(encryption_key_bytes, encryption_key_hex->data, encryption_key_hex->len);
        if (ret_code != 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The configured encryption key is not valid hex");
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    /* The cookie ciphertext size could represent a large JWT, so allocate memory dynamically
       In base64url the plaintext is always smaller than the ciphertext, but here we just ensure sufficient size */
    if (ret_code == NGX_OK)
    {
        ciphertext_bytes = ngx_pcalloc(request->pool, ciphertext->len + 1);
        if (ciphertext_bytes == NULL)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered allocating memory for ciphertext bytes");
            ret_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    /* Decode and get the exact encrypted byte sizes */
    if (ret_code == NGX_OK)
    {
        decoded_size = base64_url_decode((char *)ciphertext_bytes, (const char *)ciphertext->data);
        ciphertext_byte_size = decoded_size - (VERSION_SIZE + GCM_IV_SIZE + GCM_TAG_SIZE);
        if (ciphertext_byte_size <= 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Invalid data length after decoding from base64");
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
        else
        {
            if (ciphertext_bytes[0] != CURRENT_VERSION)
            {
                ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The received cookie has an invalid format");
                ret_code = NGX_HTTP_UNAUTHORIZED;
            }
        }
    }

    if (ret_code == NGX_OK)
    {
        plaintext_bytes = ngx_pcalloc(request->pool, ciphertext_byte_size + 1);
        if (plaintext_bytes == NULL)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered allocating memory for plaintext bytes");
            ret_code = NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
    }

    if (ret_code == NGX_OK)
    {
        offset = VERSION_SIZE;
        memcpy(iv_bytes, ciphertext_bytes + offset, GCM_IV_SIZE);

        offset = decoded_size - GCM_TAG_SIZE;
        memcpy(tag_bytes, ciphertext_bytes + offset, GCM_TAG_SIZE);
    }

    if (ret_code == NGX_OK)
    {
        // With this algorithm, this method will read precisely 32 bytes from the 4th parameter and 12 bytes from the 5th
        evp_result = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, encryption_key_bytes, iv_bytes);
        if (evp_result == 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Unable to initialize the decryption context, error number: %d", evp_result);
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
    }

    if (ret_code == NGX_OK)
    {
        offset = VERSION_SIZE + GCM_IV_SIZE;
        evp_result = EVP_DecryptUpdate(ctx, plaintext_bytes, &len, ciphertext_bytes + offset, ciphertext_byte_size);
        if (evp_result == 0)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered processing ciphertext, error number: %d", evp_result);
            ret_code = NGX_HTTP_UNAUTHORIZED;
        }
        else
        {
            plaintext_len = len;
        }
    }

    if (ret_code == NGX_OK)
    {
        // With this algorithm, this method will read precisely 16 bytes from the 4th parameter
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
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Problem encountered decrypting data, error number: %d", evp_result);
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
