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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>
#include <stdlib.h>
#include "oauth_proxy.h"

/* Forward declarations of implementation functions */
static ngx_str_t *get_header(ngx_http_request_t *request, const char *name);
static ngx_int_t verify_web_origin(const oauth_proxy_configuration_t *config, const ngx_str_t *web_origin);
static ngx_int_t apply_csrf_checks(ngx_http_request_t *request, const oauth_proxy_configuration_t *config, const ngx_str_t *web_origin);
static ngx_int_t add_authorization_header(ngx_http_request_t *request, const ngx_str_t* token_value);
static ngx_int_t write_options_response(ngx_http_request_t *request, oauth_proxy_configuration_t *module_location_config);
static ngx_int_t write_error_response(ngx_http_request_t *request, ngx_int_t status, oauth_proxy_configuration_t *config);
static ngx_int_t add_cors_response_headers(ngx_http_request_t *request, oauth_proxy_configuration_t *config, u_char is_error);

/*
 * The main exported handler method, called for each incoming API request
 */
ngx_int_t oauth_proxy_handler_main(ngx_http_request_t *request)
{
    oauth_proxy_configuration_t *module_location_config = NULL;
    ngx_str_t *authorization_header = NULL;
    ngx_str_t *web_origin = NULL;
    ngx_str_t at_cookie_encrypted_hex;
    ngx_str_t access_token;
    ngx_int_t ret_code = NGX_OK;

    /* Return immediately for locations where the module is not used */
    module_location_config = oauth_proxy_module_get_location_configuration(request);
    if (!module_location_config->enabled)
    {
        return NGX_DECLINED;
    }

    if (request->method == NGX_HTTP_OPTIONS)
    {
        if (module_location_config->cors_enabled)
        {
            /* When CORS is enabled, avoid needing to handling pre-flight OPTIONS requests in the API */
            return write_options_response(request, module_location_config);
        }

        /* If CORS is disabled, return immediately and the request will be routed to the target API */
        return NGX_OK;
    }

    /* Pass the request through if it has an Authorization header, eg from a mobile client that uses the same route as an SPA */
    if (module_location_config->allow_tokens)
    {
        authorization_header = get_header(request, "authorization");
        if (authorization_header != NULL)
        {
            return NGX_OK;
        }
    }

    /* Verify the web origin, which is sent by all modern browsers */
    web_origin = get_header(request, "origin");
    if (web_origin == NULL)
    {
        ret_code = NGX_HTTP_UNAUTHORIZED;
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The request did not have an origin header");
        return write_error_response(request, ret_code, module_location_config);
    }

    ret_code = verify_web_origin(module_location_config, web_origin);
    if (ret_code != NGX_OK)
    {
        ret_code = NGX_HTTP_UNAUTHORIZED;
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The request was from an untrusted web origin");
        return write_error_response(request, ret_code, module_location_config);
    }

    /* For data changing commands, apply double submit cookie checks in line with OWASP best practices */
    if (request->method == NGX_HTTP_POST   ||
        request->method == NGX_HTTP_PUT    ||
        request->method == NGX_HTTP_PATCH  ||
        request->method == NGX_HTTP_DELETE)
    {
        ret_code = apply_csrf_checks(request, module_location_config, web_origin);
        if (ret_code != NGX_OK)
        {
            return write_error_response(request, ret_code, module_location_config);
        }
    }

    /* This returns 0 when there is a single cookie header (HTTP 1.1) or > 0 when there are multiple cookie headers (HTTP 2.0) */
    ret_code = oauth_proxy_utils_get_cookie(request, &at_cookie_encrypted_hex, &module_location_config->cookie_name_prefix, (u_char *)"-at");
    if (ret_code == NGX_DECLINED)
    {
        ret_code = NGX_HTTP_UNAUTHORIZED;
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "No AT cookie was found in the incoming request");
        return write_error_response(request, ret_code, module_location_config);
    }

    /* Try to decrypt the cookie to get the access token */
    ret_code = oauth_proxy_decryption_decrypt_cookie(request, &access_token, &at_cookie_encrypted_hex, &module_location_config->encryption_key);
    if (ret_code != NGX_OK)
    {
        return write_error_response(request, ret_code, module_location_config);
    }

    /* Update the authorization header in the headers in, to forward to the API via proxy_pass */
    ret_code = add_authorization_header(request, &access_token);
    if (ret_code != NGX_OK)
    {
        return write_error_response(request, ret_code, module_location_config);
    }
    
    /* Finally update CORS headers, which must be done for both the pre-flight request and also the main API request */
    if (module_location_config->cors_enabled)
    {
        add_cors_response_headers(request, module_location_config, 0);
    }

    return NGX_OK;
}

/*
 * Return the authorization header if it exists
 */
static ngx_str_t *get_header(ngx_http_request_t *request, const char *name)
{
    return oauth_proxy_utils_get_header_in(request, (u_char *)name, ngx_strlen(name));
}

/*
 * Ensure that incoming requests have the origin header that all modern browsers send
 */
static ngx_int_t verify_web_origin(const oauth_proxy_configuration_t *config, const ngx_str_t *web_origin)
{
    ngx_str_t *trusted_web_origins = NULL;
    ngx_str_t trusted_web_origin;
    ngx_uint_t i = 0;

    trusted_web_origins = config->trusted_web_origins->elts;
    for (i = 0; i < config->trusted_web_origins->nelts; i++)
    {
        trusted_web_origin = trusted_web_origins[i];

        if (web_origin->len == trusted_web_origin.len && 
            ngx_strncasecmp(web_origin->data, trusted_web_origin.data, web_origin->len) == 0)
        {
            return NGX_OK;
        }
    }

    return NGX_ERROR;
}

/*
 * For data changing commands we make extra CSRF checks in line with OWASP best practices
 */
static ngx_int_t apply_csrf_checks(ngx_http_request_t *request, const oauth_proxy_configuration_t *config, const ngx_str_t *web_origin)
{
    ngx_str_t csrf_cookie_encrypted_hex;
    u_char csrf_header_name[128];
    ngx_str_t *csrf_header_value = NULL;
    ngx_str_t csrf_token;
    ngx_int_t ret_code = NGX_OK;

    /* This returns 0 when there is a single cookie header or > 0 when there are multiple cookie headers */
    ret_code = oauth_proxy_utils_get_cookie(request, &csrf_cookie_encrypted_hex, &config->cookie_name_prefix, (u_char *)"-csrf");
    if (ret_code == NGX_DECLINED)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "No CSRF cookie was found in the incoming request");
        return NGX_HTTP_UNAUTHORIZED;
    }

    oauth_proxy_utils_get_csrf_header_name(csrf_header_name, config);
    csrf_header_value = oauth_proxy_utils_get_header_in(request, csrf_header_name, ngx_strlen(csrf_header_name));
    if (csrf_header_value == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "A data changing request did not have a CSRF header");
        return NGX_HTTP_UNAUTHORIZED;
    }

    ret_code = oauth_proxy_decryption_decrypt_cookie(request, &csrf_token, &csrf_cookie_encrypted_hex, &config->encryption_key);
    if (ret_code != NGX_OK)
    {
        return ret_code;
    }

    if (ngx_strcmp(csrf_token.data, csrf_header_value->data) != 0)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The CSRF request header did not match the value in the encrypted CSRF cookie");
        return NGX_HTTP_UNAUTHORIZED;
    }

    return NGX_OK;
}

/*
 * Set the authorization header and deal with string manipulation
 */
static ngx_int_t add_authorization_header(ngx_http_request_t *request, const ngx_str_t* token_value)
{
    ngx_table_elt_t *authorization_header = NULL;
    u_char *header_value = NULL;
    size_t header_value_len = 0;

    authorization_header = ngx_list_push(&request->headers_in.headers);
    if (authorization_header == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate memory for the authorization header");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* The header size is unknown and could represent a large JWT, so allocate memory dynamically */
    header_value_len = ngx_strlen("Bearer ") + token_value->len;
    header_value = ngx_pcalloc(request->pool, header_value_len + 1);
    if (header_value == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate memory for the authorization header value");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_str_set(&authorization_header->key, "authorization");
    ngx_snprintf(header_value, header_value_len, "Bearer %V", token_value);
    header_value[header_value_len] = 0;

    authorization_header->value.data = header_value;
    authorization_header->value.len  = header_value_len;
    authorization_header->hash = 1;
    request->headers_in.authorization = authorization_header;

    return NGX_OK;
}

/*
 * Write an empty CORS response
 */
static ngx_int_t write_options_response(ngx_http_request_t *request, oauth_proxy_configuration_t *module_location_config)
{
    add_cors_response_headers(request, module_location_config, 0);
    return NGX_HTTP_NO_CONTENT;
}

/*
 * Add the error response and write CORS headers so that Javascript can read it
 */
static ngx_int_t write_error_response(ngx_http_request_t *request, ngx_int_t status, oauth_proxy_configuration_t *module_location_config)
{
    ngx_str_t code;
    ngx_str_t message;
    u_char jsonErrorData[256];
    ngx_chain_t output;
    ngx_buf_t *body = NULL;
    const char *errorFormat = NULL;
    size_t errorLen = 0;

    add_cors_response_headers(request, module_location_config, 1);
    if (request->method != NGX_HTTP_HEAD)
    {
        body = ngx_calloc_buf(request->pool);
        if (body == NULL)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate memory for error body");
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        else
        {
            /* The error interface supports only two responses, though more error codes will be added in futrure if needed by SPAs */
            if (status == NGX_HTTP_INTERNAL_SERVER_ERROR)
            {
                ngx_str_set(&code, "server_error");
                ngx_str_set(&message, "Problem encountered processing the request");
            }
            else
            {
                ngx_str_set(&code, "unauthorized");
                ngx_str_set(&message, "Access denied due to missing or invalid credentials");
            }

            /* The string length calculation replaces the two '%V' markers with their actual values */
            errorFormat = "{\"code\":\"%V\", \"message\":\"%V\"}";
            errorLen = ngx_strlen(errorFormat) + code.len + message.len - 4;
            ngx_snprintf(jsonErrorData, sizeof(jsonErrorData) - 1, errorFormat, &code, &message);
            jsonErrorData[errorLen] = 0;

            request->headers_out.status = status;
            request->headers_out.content_length_n = errorLen;
            ngx_http_send_header(request);

            /* http://nginx.org/en/docs/dev/development_guide.html#http_response_body */
            ngx_str_set(&request->headers_out.content_type, "application/json");
            body->pos = jsonErrorData;
            body->last = jsonErrorData + errorLen;
            body->memory = 1;
            body->last_buf = 1;
            body->last_in_chain = 1;
            output.buf = body;
            output.next = NULL;

            /* When setting a body ourself we must return the result of the filter, to prevent a 'header already sent' error */
            return ngx_http_output_filter(request, &output);
        }
    }

    return status;
}


/*
 * When there is a valid web origin, add CORS headers so that Javascript can read the response
 */
static ngx_int_t add_cors_response_headers(ngx_http_request_t *request, oauth_proxy_configuration_t *config, u_char is_error)
{
    ngx_str_t *web_origin = NULL;
    ngx_str_t *allow_headers = NULL;
    ngx_str_t allow_credentials_str;
    ngx_str_t vary_str;
    const char *literal_request_headers = "access-control-request-headers";

    web_origin = get_header(request, "origin");
    if (web_origin != NULL && verify_web_origin(config, web_origin) == NGX_OK)
    {
        /* These are always needed in order for the SPA to be able to read error responses from the module */
        if (config->cors_enabled || is_error != 0)
        {
            if (oauth_proxy_utils_add_header_out(request,  "access-control-allow-origin", web_origin) != NGX_OK)
            {
                ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS allow_origin response header");
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }

            ngx_str_set(&allow_credentials_str, "true");
            if (oauth_proxy_utils_add_header_out(request,  "access-control-allow-credentials", &allow_credentials_str) != NGX_OK)
            {
                ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS allow_credentials response header");
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }
        }

        if (config->cors_enabled)
        {
            /* Write headers only needed in responses to pre-flight requests */
            ngx_str_set(&vary_str, "origin");
            if (request->method == NGX_HTTP_OPTIONS)
            {
                if (config->cors_allow_methods.len > 0)
                {
                    if (oauth_proxy_utils_add_header_out(request,  "access-control-allow-methods", &config->cors_allow_methods) != NGX_OK)
                    {
                        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS allow_methods response header");
                        return NGX_HTTP_INTERNAL_SERVER_ERROR;
                    }
                }

                /* If no headers are set explicitly then return any headers the browser requests at runtime
                   This ensures that the API gateway does not need reconfiguration whenever a new header is sent */
                allow_headers = &config->cors_allow_headers;
                if (allow_headers->len == 0)
                {
                    allow_headers = oauth_proxy_utils_get_header_in(request, (u_char *)literal_request_headers, ngx_strlen(literal_request_headers));
                }

                if (allow_headers != NULL)
                {
                    if (oauth_proxy_utils_add_header_out(request,  "access-control-allow-headers", allow_headers) != NGX_OK)
                    {
                        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS allow_headers response header");
                        return NGX_HTTP_INTERNAL_SERVER_ERROR;
                    }
                }

                if (config->cors_max_age > 0)
                {
                    if (oauth_proxy_utils_add_integer_header_out(request,  "access-control-max-age", config->cors_max_age) != NGX_OK)
                    {
                        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS max_age header");
                        return NGX_HTTP_INTERNAL_SERVER_ERROR;
                    }
                }

                ngx_str_set(&vary_str, "origin,access-control-request-headers");
            }

            /* These headers are needed in both pre-flight requests and the main request */
            if (config->cors_expose_headers.len > 0)
            {
                if (oauth_proxy_utils_add_header_out(request,  "access-control-expose-headers", &config->cors_expose_headers) != NGX_OK)
                {
                    ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS expose_headers response header");
                    return NGX_HTTP_INTERNAL_SERVER_ERROR;
                }
            }

            if (oauth_proxy_utils_add_header_out(request,  "vary", &vary_str) != NGX_OK)
            {
                ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to add CORS vary response header");
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
            }
        }
    }

    return NGX_OK;
}