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

/* Custom types */
typedef struct
{
    ngx_flag_t enabled;
    ngx_flag_t allow_tokens;
    ngx_str_t cookie_prefix;
    ngx_str_t hex_encryption_key;
    ngx_array_t *trusted_web_origins;
} oauth_proxy_configuration_t;

typedef struct
{
    ngx_uint_t done;
    ngx_uint_t status;
} oauth_proxy_module_context_t;

/* Forward declarations of plumbing functions */
static void *create_location_configuration(ngx_conf_t *config);
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child);
static ngx_int_t post_configuration(ngx_conf_t *config);
static ngx_int_t validate_configuration(ngx_conf_t *config, oauth_proxy_configuration_t *module_location_config);

/* Forward declarations of implementation functions */
static ngx_int_t handler(ngx_http_request_t *request);
static ngx_str_t *search_headers_in(ngx_http_request_t *request, u_char *name, size_t len);
static ngx_int_t verify_web_origin(oauth_proxy_configuration_t *config, ngx_str_t *web_origin);
static ngx_int_t get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, ngx_str_t* cookie_prefix, const char *cookie_suffix);
static ngx_int_t add_authorization_header(ngx_http_request_t *request, ngx_str_t* token_value);

/* Constants */
static size_t MAX_COOKIE_PREFIX_LENGTH = 32;
static size_t MAX_COOKIE_SUFFIX_LENGTH = 5; /* The longest cookie suffix is -csrf */

/* Imports from the decryption source file */
extern ngx_int_t oauth_proxy_decrypt(ngx_http_request_t *request, const ngx_str_t* encryption_key_hex, const ngx_str_t* encrypted_hex, ngx_str_t *plain_text);

/* Configuration data */
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
 * Validate the cookie prefix to prevent cryptic problems later
 */
static ngx_int_t validate_configuration(ngx_conf_t *config, oauth_proxy_configuration_t *module_location_config)
{
    ngx_str_t *trusted_web_origins = NULL;
    ngx_str_t trusted_web_origin;

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
        for (ngx_uint_t i = 0; i < module_location_config->trusted_web_origins->nelts; i++)
        {
            trusted_web_origin = trusted_web_origins[i];
            if (trusted_web_origin.len < 7)
            {
                ngx_conf_log_error(NGX_LOG_WARN, config, 0, "An invalid trusted_web_origin configuration directive was provided", &trusted_web_origin);
                return NGX_ERROR;
            }
            
            if (ngx_strncasecmp(trusted_web_origin.data, (u_char*)"http://",  7) != 0 &&
                ngx_strncasecmp(trusted_web_origin.data, (u_char*)"https://", 8) != 0)
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
    
    // Return immediately when the module is disabled
    module_location_config = ngx_http_get_module_loc_conf(request, ngx_curity_http_oauth_proxy_module);
    if (!module_location_config->enabled)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy module is disabled");
        return NGX_DECLINED;
    }

    // Pre-flight requests from SPAs will never return cookies or tokens, so return immediately
    if (ngx_strncasecmp(request->method_name.data, (u_char*)"options", 7) == 0)
    {
        return NGX_OK;
    }

    // Pass the request through if it has an Authorization header, eg from a mobile client that uses the same route as an SPA
    if (module_location_config->allow_tokens && request->headers_in.authorization && request->headers_in.authorization->value.len > 0)
    {
        return NGX_OK;
    }

    // Try to find the web origin
    web_origin = search_headers_in(request, (u_char*)"origin", 6);
    if (web_origin == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The request did not have an origin header");
        return NGX_HTTP_UNAUTHORIZED;
    }

    // Check that it is trusted
    ret_code = verify_web_origin(module_location_config, web_origin);
    if (ret_code != NGX_OK)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "The request was from an untrusted web origin");
        return NGX_HTTP_UNAUTHORIZED;
    }

    // Try to get the access token cookie
    ret_code = get_cookie(request, &at_cookie_encrypted_hex, &module_location_config->cookie_prefix, "-at");

    // When no cookie is provided we return an unauthorized status code
    if (ret_code == NGX_DECLINED)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "No cookie was found in the incoming request");
        return NGX_HTTP_UNAUTHORIZED;
    }

    // Handle other errors getting the cookie
    if (ret_code != NGX_OK)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "Cookie read problem encountered: %d", ret_code);
        return ret_code;
    }

    // Decrypt the secure cookie to get its access token content
    
    ret_code = oauth_proxy_decrypt(request, &module_location_config->hex_encryption_key, &at_cookie_encrypted_hex, &access_token);
    if (ret_code != NGX_OK)
    {   
        return ret_code;
    }

    // Add the cookie to the authorization header
    ret_code = add_authorization_header(request, &access_token);
    if (ret_code != NGX_OK)
    {
        return ret_code;
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

    // Get the first part of the list. There is usual only one part
    part = &request->headers_in.headers.part;
    h = part->elts;

    // Headers list array may consist of more than one part, so loop through all of it
    for (i = 0; /* void */ ; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                /* The last part, search is done. */
                break;
            }

            part = part->next;
            h = part->elts;
            i = 0;
        }

        // Just compare the lengths and then the names case insensitively
        if (len != h[i].key.len || ngx_strcasecmp(name, h[i].key.data) != 0) {
            continue;
        }

        // Stop the search at the first matched header
        return &h[i].value;
    }

    return NULL;
}

/*
 * Ensure that incoming requests have the origin header that all modern browsers send
 */
static ngx_int_t verify_web_origin(oauth_proxy_configuration_t *config, ngx_str_t *web_origin)
{
    ngx_str_t *trusted_web_origins = NULL;
    ngx_str_t trusted_web_origin;

    trusted_web_origins = config->trusted_web_origins->elts;
    for (ngx_uint_t i = 0; i < config->trusted_web_origins->nelts; i++)
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
 * Get a cookie and deal with string manipulation
 */
static ngx_int_t get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, ngx_str_t* cookie_prefix, const char *cookie_suffix)
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
static ngx_int_t add_authorization_header(ngx_http_request_t *request, ngx_str_t* token_value)
{
    size_t header_value_len = 0;
    u_char *header_value = NULL;

    request->headers_in.authorization = ngx_list_push(&request->headers_in.headers);
    if (request->headers_in.authorization == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate memory for the authorization header");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    header_value_len = ngx_strlen("Bearer ") + token_value->len;
    header_value = ngx_pcalloc(request->pool, header_value_len);
    if (header_value == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate memory for the authorization header value");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_snprintf(header_value, header_value_len, "Bearer %V", token_value);

    ngx_str_set(&request->headers_in.authorization->key, "Authorization");
    request->headers_in.authorization->value = (ngx_str_t)ngx_string(header_value);
    request->headers_in.authorization->value.len = header_value_len;
    return NGX_OK;
}
