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

/* Configuration data */
typedef struct
{
    ngx_flag_t enabled;
    ngx_flag_t allow_tokens;
    ngx_str_t cookie_prefix;
    ngx_str_t hex_encryption_key;
    ngx_array_t *trusted_web_origins;
} oauth_proxy_configuration_t;

/* Configuration directives */
static ngx_command_t oauth_proxy_module_directives[] =
{
    {
        ngx_string("oauth_proxy"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, enabled),
        NULL
    },
    {
        ngx_string("oauth_proxy_allow_tokens"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, allow_tokens),
        NULL
    },
    {
        ngx_string("oauth_proxy_cookie_prefix"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cookie_prefix),
        NULL
    },
    {
        ngx_string("oauth_proxy_hex_encryption_key"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, hex_encryption_key),
        NULL
    },
    {
        ngx_string("oauth_proxy_trusted_web_origin"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_array_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, trusted_web_origins),
        NULL
    },
    ngx_null_command /* command termination */
};

/* Forward declarations of NGINX functions */
static void *create_location_configuration(ngx_conf_t *config);
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child);
static ngx_int_t post_configuration(ngx_conf_t *config);
static ngx_int_t validate_configuration(ngx_conf_t *config, const oauth_proxy_configuration_t *module_location_config);

/* Forward declarations of implementation functions */
static ngx_int_t handler(ngx_http_request_t *request);
static ngx_int_t verify_web_origin(const oauth_proxy_configuration_t *config, const ngx_str_t *web_origin);
static ngx_int_t apply_csrf_checks(ngx_http_request_t *request, const oauth_proxy_configuration_t *config, const ngx_str_t *web_origin);
static ngx_str_t *search_headers_in(ngx_http_request_t *request, u_char *name, size_t len);
static ngx_int_t get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, const ngx_str_t* cookie_prefix, const u_char *cookie_suffix);
static ngx_int_t add_authorization_header(ngx_http_request_t *request, const ngx_str_t* token_value);
static ngx_int_t write_error_response(ngx_http_request_t *request, ngx_int_t status, const ngx_str_t *web_origin);

/* Imports from the decryption module */
extern ngx_int_t decrypt_cookie(ngx_http_request_t *request, ngx_str_t *plain_text, const ngx_str_t* ciphertext, const ngx_str_t* encryption_key_hex);

/* Constants */
static size_t MAX_COOKIE_PREFIX_LENGTH = 64;
static size_t MAX_COOKIE_SUFFIX_LENGTH = 5; /* The longest cookie suffix is -csrf */
static const char *literal_origin = "origin";
static const char *literal_http   = "http://";
static const char *literal_https  = "https://";

/* NGINX integration */
static ngx_http_module_t oauth_proxy_module_context =
{
    NULL, /* pre-configuration */
    post_configuration,

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    create_location_configuration,
    merge_location_configuration
};

ngx_module_t ngx_curity_http_oauth_proxy_module =
{
    NGX_MODULE_V1,
    &oauth_proxy_module_context,
    oauth_proxy_module_directives,
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};

/*
 * Called when NGINX starts up and finds a location that uses the plugin
 */
static void *create_location_configuration(ngx_conf_t *config)
{
    oauth_proxy_configuration_t *location_config = ngx_pcalloc(config->pool, sizeof(oauth_proxy_configuration_t));
    if (location_config == NULL)
    {
        return NGX_CONF_ERROR;
    }

    location_config->enabled             = NGX_CONF_UNSET_UINT;
    location_config->allow_tokens        = NGX_CONF_UNSET_UINT;
    location_config->trusted_web_origins = NGX_CONF_UNSET_PTR;
    return location_config;
}

/*
 * Called when NGINX starts up and finds a parent location that uses the plugin
 */
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child)
{
    oauth_proxy_configuration_t *parent_config = parent, *child_config = child;
    ngx_int_t validation_result = NGX_OK;

    ngx_conf_merge_off_value(child_config->enabled,             parent_config->enabled,             0);
    ngx_conf_merge_off_value(child_config->allow_tokens,        parent_config->allow_tokens,        0);
    ngx_conf_merge_str_value(child_config->cookie_prefix,       parent_config->cookie_prefix,       "");
    ngx_conf_merge_str_value(child_config->hex_encryption_key,  parent_config->hex_encryption_key,  "");
    ngx_conf_merge_ptr_value(child_config->trusted_web_origins, parent_config->trusted_web_origins, NULL);

    validation_result = validate_configuration(main_config, child_config);
    if (validation_result != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }
    
    return NGX_CONF_OK;
}

/*
 * Set up the handler after configuration has been processed
 */
static ngx_int_t post_configuration(ngx_conf_t *config)
{
    ngx_http_core_main_conf_t *main_config = ngx_http_conf_get_module_main_conf(config, ngx_http_core_module);
    ngx_http_handler_pt *h = ngx_array_push(&main_config->phases[NGX_HTTP_ACCESS_PHASE].handlers);

    if (h == NULL)
    {
        return NGX_ERROR;
    }

    *h = handler;
    return NGX_OK;
}

/*
 * Validate the cookie prefix to prevent deeper problems later
 */
static ngx_int_t validate_configuration(ngx_conf_t *config, const oauth_proxy_configuration_t *module_location_config)
{
    ngx_str_t *trusted_web_origins = NULL;
    ngx_str_t trusted_web_origin;
    
    ngx_uint_t i = 0;

    if (module_location_config != NULL && module_location_config->enabled)
    {
        if (module_location_config->cookie_prefix.len == 0)
        {
            ngx_conf_log_error(NGX_LOG_WARN, config, 0, "The cookie_prefix configuration directive was not provided");
            return NGX_ERROR;
        }

        if (module_location_config->cookie_prefix.len > MAX_COOKIE_PREFIX_LENGTH)
        {
            ngx_conf_log_error(NGX_LOG_WARN, config, 0, "The cookie_prefix configuration directive has a maximum length of %d characters", MAX_COOKIE_PREFIX_LENGTH);
            return NGX_ERROR;
        }

        if (module_location_config->hex_encryption_key.len == 0)
        {
            ngx_conf_log_error(NGX_LOG_WARN, config, 0, "The hex_encryption_key configuration directive was not provided");
            return NGX_ERROR;
        }

        if (module_location_config->hex_encryption_key.len != 64)
        {
            ngx_conf_log_error(NGX_LOG_WARN, config, 0, "The hex_encryption_key configuration directive must contain 64 hex characters");
            return NGX_ERROR;
        }

        if (module_location_config->trusted_web_origins == NULL || module_location_config->trusted_web_origins->nelts == 0)
        {
            ngx_conf_log_error(NGX_LOG_WARN, config, 0, "The trusted_web_origin configuration directive was not provided for any web origins");
            return NGX_ERROR;
        }

        trusted_web_origins = module_location_config->trusted_web_origins->elts;
        for (i = 0; i < module_location_config->trusted_web_origins->nelts; i++)
        {
            trusted_web_origin = trusted_web_origins[i];
            if (trusted_web_origin.len < 7)
            {
                ngx_conf_log_error(NGX_LOG_WARN, config, 0, "An invalid trusted_web_origin configuration directive was provided", &trusted_web_origin);
                return NGX_ERROR;
            }
            
            if (ngx_strncasecmp(trusted_web_origin.data, (u_char*)literal_http,  ngx_strlen(literal_http))  != 0 &&
                ngx_strncasecmp(trusted_web_origin.data, (u_char*)literal_https, ngx_strlen(literal_https)) != 0)
            {
                ngx_conf_log_error(NGX_LOG_WARN, config, 0, "An invalid trusted_web_origin configuration directive was provided: %V", &trusted_web_origin);
                return NGX_ERROR;
            }
        }
    }

    return NGX_OK;
}

/*
 * Called during HTTP requests to make cookie related checks and then to decrypt the cookie to get an access token
 */
static ngx_int_t handler(ngx_http_request_t *request)
{
    oauth_proxy_configuration_t *module_location_config = NULL;
    ngx_str_t *web_origin = NULL;
    ngx_str_t at_cookie_encrypted_hex;
    ngx_str_t access_token;
    ngx_int_t ret_code = NGX_OK;
    
    /* Return immediately for locations where the module is not used */
    module_location_config = ngx_http_get_module_loc_conf(request, ngx_curity_http_oauth_proxy_module);
    if (!module_location_config->enabled)
    {
        return NGX_DECLINED;
    }

    /* Pre-flight requests from SPAs will never return cookies or tokens, so return immediately */
    if (request->method == NGX_HTTP_OPTIONS)
    {
        return NGX_OK;
    }

    /* Pass the request through if it has an Authorization header, eg from a mobile client that uses the same route as an SPA */
    if (module_location_config->allow_tokens && request->headers_in.authorization && request->headers_in.authorization->value.len > 0)
    {
        return NGX_OK;
    }

    /* Verify the web origin, which is sent by all modern browsers */
    web_origin = search_headers_in(request, (u_char *)literal_origin, ngx_strlen(literal_origin));
    if (web_origin == NULL)
    {
        ret_code = NGX_HTTP_UNAUTHORIZED;
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The request did not have an origin header");
        return write_error_response(request, ret_code, NULL);
    }

    ret_code = verify_web_origin(module_location_config, web_origin);
    if (ret_code != NGX_OK)
    {
        ret_code = NGX_HTTP_UNAUTHORIZED;
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The request was from an untrusted web origin");
        return write_error_response(request, ret_code, NULL);
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
            return write_error_response(request, ret_code, web_origin);
        }
    }

    /* This returns 0 when there is a single cookie header (HTTP 1.1) or > 0 when there are multiple cookie headers (HTTP 2.0) */
    ret_code = get_cookie(request, &at_cookie_encrypted_hex, &module_location_config->cookie_prefix, (u_char *)"-at");
    if (ret_code == NGX_DECLINED)
    {
        ret_code = NGX_HTTP_UNAUTHORIZED;
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "No AT cookie was found in the incoming request");
        return write_error_response(request, ret_code, web_origin);
    }

    /* Try to decrypt the access token cookie to get the access token */
    ret_code = decrypt_cookie(request, &access_token, &at_cookie_encrypted_hex, &module_location_config->hex_encryption_key);
    if (ret_code != NGX_OK)
    {
        return write_error_response(request, ret_code, web_origin);
    }

    /* Finally, update the authorization header in the headers in, to forward to the API via proxy_pass */
    ret_code = add_authorization_header(request, &access_token);
    if (ret_code != NGX_OK)
    {
        return write_error_response(request, ret_code, web_origin);
    }

    return NGX_OK;
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
    u_char csrf_header_name[2 + MAX_COOKIE_PREFIX_LENGTH + MAX_COOKIE_SUFFIX_LENGTH + 1];
    ngx_str_t *csrf_header_value = NULL;
    ngx_str_t csrf_token;
    const char *literal_prefix = "x-";
    const char *literal_suffix = "-csrf";
    size_t prefix_length = 2;
    size_t suffix_length = 5;
    ngx_int_t ret_code = NGX_OK;

    /* This returns 0 when there is a single cookie header or > 0 when there are multiple cookie headers */
    ret_code = get_cookie(request, &csrf_cookie_encrypted_hex, &config->cookie_prefix, (u_char *)literal_suffix);
    if (ret_code == NGX_DECLINED)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "No CSRF cookie was found in the incoming request");
        return NGX_HTTP_UNAUTHORIZED;
    }

    ngx_memcpy(csrf_header_name, literal_prefix, prefix_length);
    ngx_memcpy(csrf_header_name + prefix_length, config->cookie_prefix.data, config->cookie_prefix.len);
    ngx_memcpy(csrf_header_name + prefix_length + config->cookie_prefix.len, literal_suffix, suffix_length);
    csrf_header_name[2 + config->cookie_prefix.len + suffix_length] = 0;
    csrf_header_value = search_headers_in(request, csrf_header_name, ngx_strlen(csrf_header_name));
    if (csrf_header_value == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "A data changing request did not have a CSRF header");
        return NGX_HTTP_UNAUTHORIZED;
    }

    ret_code = decrypt_cookie(request, &csrf_token, &csrf_cookie_encrypted_hex, &config->hex_encryption_key);
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
 * Find a header that is not in the standard headers_in structure
 * https://www.nginx.com/resources/wiki/start/topics/examples/headers_management/
 */
static ngx_str_t *search_headers_in(ngx_http_request_t *request, u_char *name, size_t len)
{
    ngx_list_part_t *part = NULL;
    ngx_table_elt_t *h = NULL;
    ngx_uint_t i = 0;

    /* Get the first part of the list. There is usual only one part */
    part = &request->headers_in.headers.part;
    h = part->elts;

    /* Headers list array may consist of more than one part, so loop through all of it */
    for (i = 0; ; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                /* The last part, search is done */
                break;
            }

            part = part->next;
            h = part->elts;
            i = 0;
        }

        /* Just compare the lengths and then the names case insensitively */
        if (len != h[i].key.len || ngx_strcasecmp(name, h[i].key.data) != 0) {
            continue;
        }

        /* Stop the search at the first matched header */
        return &h[i].value;
    }

    return NULL;
}

/*
 * Get a cookie and deal with string manipulation
 */
static ngx_int_t get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, const ngx_str_t* cookie_prefix, const u_char *cookie_suffix)
{
    u_char cookie_name[MAX_COOKIE_PREFIX_LENGTH + MAX_COOKIE_SUFFIX_LENGTH + 1];
    size_t suffix_len = 0;
    ngx_str_t cookie_name_str;

    suffix_len = ngx_strlen(cookie_suffix);
    ngx_memcpy(cookie_name, cookie_prefix->data, cookie_prefix->len);
    ngx_memcpy(cookie_name + cookie_prefix->len, cookie_suffix, suffix_len);
    cookie_name[cookie_prefix->len + suffix_len] = 0;

    cookie_name_str.data = cookie_name;
    cookie_name_str.len = ngx_strlen(cookie_name);
    return ngx_http_parse_multi_header_lines(&request->headers_in.cookies, &cookie_name_str, cookie_value);
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
 * Add the error response and write CORS headers so that Javascript can read it
 */
static ngx_int_t write_error_response(ngx_http_request_t *request, ngx_int_t status, const ngx_str_t *web_origin)
{
    ngx_table_elt_t *allow_headers     = NULL;
    ngx_table_elt_t *allow_credentials = NULL;
    ngx_str_t code;
    ngx_str_t message;
    u_char jsonErrorData[256];
    ngx_chain_t output;
    ngx_buf_t *body = NULL;
    const char *errorFormat = NULL;
    size_t errorLen = 0;

    /* When there is a valid web origin, add CORS headers so that Javascript can read the response */
    if (web_origin != NULL)
    {
        allow_headers     = ngx_list_push(&request->headers_out.headers);
        allow_credentials = ngx_list_push(&request->headers_out.headers);
        if (allow_headers == NULL || allow_credentials == NULL)
        {
            ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate memory for error headers");
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        else
        {
            ngx_str_set(&allow_headers->key,   "access-control-allow-origin");
            allow_headers->value.data = web_origin->data;
            allow_headers->value.len = web_origin->len;
            allow_headers->hash = 1;

            ngx_str_set(&allow_credentials->key,   "access-control-allow-credentials");
            ngx_str_set(&allow_credentials->value, "true");
            allow_credentials->hash = 1;
        }
    }

    /* Always add the JSON body unless it is not valid to set one */
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
                ngx_str_set(&code, "unauthorized_request");
                ngx_str_set(&message, "Access denied due to missing or invalid credentials");
            }

            errorFormat = "{\"code\": \"%V\", \"message\": \"%V\"}";
            errorLen = ngx_strlen(errorFormat) + code.len + message.len - 4;
            ngx_snprintf(jsonErrorData, sizeof(jsonErrorData) - 1, errorFormat, &code, &message);
            jsonErrorData[errorLen] = 0;

            request->headers_out.status = status;
            request->headers_out.content_length_n = errorLen;
            ngx_http_send_header(request);

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
