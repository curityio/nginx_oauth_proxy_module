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
#include "oauth_proxy.h"

/* Imports */
ngx_int_t handler(ngx_http_request_t *request);
void get_csrf_header_name(u_char *csrf_header_name, const oauth_proxy_configuration_t *config);

/* Forward declarations */
static void *create_location_configuration(ngx_conf_t *config);
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child);
static ngx_int_t post_configuration(ngx_conf_t *config);
static ngx_int_t apply_configuration_defaults(ngx_conf_t *main_config, oauth_proxy_configuration_t *config);
static ngx_int_t create_nginx_string_array(ngx_conf_t *main_config, ngx_array_t **data, size_t num_values, ...);
static ngx_int_t validate_configuration(ngx_conf_t *main_config, const oauth_proxy_configuration_t *module_location_config);

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
        ngx_string("oauth_proxy_cookie_name_prefix"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cookie_name_prefix),
        NULL
    },
    {
        ngx_string("oauth_proxy_encryption_key"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, encryption_key),
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
    {
        ngx_string("oauth_proxy_cors_enabled"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cors_enabled),
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
        ngx_string("oauth_proxy_remove_cookie_headers"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, remove_cookie_headers),
        NULL
    },
    {
        ngx_string("oauth_proxy_cors_allow_methods"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_array_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cors_allow_methods),
        NULL
    },
    {
        ngx_string("oauth_proxy_cors_allow_headers"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_array_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cors_allow_headers),
        NULL
    },
    {
        ngx_string("oauth_proxy_cors_expose_headers"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_array_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cors_expose_headers),
        NULL
    },
    {
        ngx_string("oauth_proxy_cors_max_age"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(oauth_proxy_configuration_t, cors_max_age),
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

/* This module is exported and must be non static to avoid linker errors */
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
static void *create_location_configuration(ngx_conf_t *main_config)
{
    oauth_proxy_configuration_t *location_config = ngx_pcalloc(main_config->pool, sizeof(oauth_proxy_configuration_t));
    if (location_config == NULL)
    {
        return NGX_CONF_ERROR;
    }

    location_config->enabled               = NGX_CONF_UNSET_UINT;
    location_config->trusted_web_origins   = NGX_CONF_UNSET_PTR;
    location_config->cors_enabled          = NGX_CONF_UNSET_UINT;
    location_config->allow_tokens          = NGX_CONF_UNSET_UINT;
    location_config->remove_cookie_headers = NGX_CONF_UNSET_UINT;
    location_config->cors_allow_methods    = NGX_CONF_UNSET_PTR;
    location_config->cors_allow_headers    = NGX_CONF_UNSET_PTR;
    location_config->cors_expose_headers   = NGX_CONF_UNSET_PTR;
    location_config->cors_max_age          = NGX_CONF_UNSET_UINT;
    return location_config;
}

/*
 * Called when NGINX starts up and finds a parent location that uses the plugin
 */
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child)
{
    oauth_proxy_configuration_t *parent_config = parent, *child_config = child;
    ngx_int_t init_result = NGX_OK;
    ngx_int_t validation_result = NGX_OK;

    ngx_conf_merge_off_value(child_config->enabled,                parent_config->enabled,                0);
    ngx_conf_merge_str_value(child_config->cookie_name_prefix,     parent_config->cookie_name_prefix,     "");
    ngx_conf_merge_str_value(child_config->encryption_key,         parent_config->encryption_key,         "");
    ngx_conf_merge_ptr_value(child_config->trusted_web_origins,    parent_config->trusted_web_origins,    NULL);
    ngx_conf_merge_off_value(child_config->cors_enabled,           parent_config->cors_enabled,           0);
    ngx_conf_merge_off_value(child_config->allow_tokens,           parent_config->allow_tokens,           0);
    ngx_conf_merge_off_value(child_config->remove_cookie_headers,  parent_config->remove_cookie_headers,  0);
    ngx_conf_merge_ptr_value(child_config->cors_allow_methods,     parent_config->cors_allow_methods,   NULL);
    ngx_conf_merge_ptr_value(child_config->cors_allow_headers,     parent_config->cors_allow_headers,   NULL);
    ngx_conf_merge_ptr_value(child_config->cors_expose_headers,    parent_config->cors_expose_headers,   NULL);
    ngx_conf_merge_off_value(child_config->cors_max_age,           parent_config->cors_max_age,           0);
    
    init_result = apply_configuration_defaults(main_config, child_config);
    if (init_result != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

    validation_result = validate_configuration(main_config, child_config);
    if (validation_result != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }
    
    return NGX_CONF_OK;
}

/*
 * Set default options that are not provided in then nginx.conf file
 */
static ngx_int_t apply_configuration_defaults(ngx_conf_t *main_config, oauth_proxy_configuration_t *config)
{
    u_char csrf_header_name[128];
    ngx_int_t ret_code = NGX_OK;

    if (config->cors_enabled)
    {
        if (config->cors_allow_methods == NULL)
        {
            ret_code = create_nginx_string_array(main_config, &config->cors_allow_methods, 7,
                                                "OPTIONS", "GET", "HEAD", "POST", "PUT", "PATCH", "DELETE");
            if (ret_code != NGX_OK)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "Unable to allocate memory for cors_allow_methods");
                return ret_code;
            }
        }

        if (config->cors_allow_headers == NULL)
        {
            get_csrf_header_name(csrf_header_name, config);
            ret_code = create_nginx_string_array(main_config, &config->cors_allow_headers, 1, csrf_header_name);
            if (ret_code != NGX_OK)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "Unable to allocate memory for cors_allow_headers");
                return ret_code;
            }
        }

        if (config->cors_expose_headers == NULL)
        {
            ret_code = create_nginx_string_array(main_config, &config->cors_expose_headers, 0);
            if (ret_code != NGX_OK)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "Unable to allocate memory for cors_expose_headers");
                return ret_code;
            }
        }

        if (config->cors_max_age <= 0)
        {
            config->cors_max_age = 86400;
        }
    }

    return ret_code;
}

/*
 * Do the plumbing to populate an array type from the pool
 */
static ngx_int_t create_nginx_string_array(ngx_conf_t *main_config, ngx_array_t **data, size_t num_values, ...)
{
    va_list args;
    ngx_str_t *array_item = NULL;
    const char *value = NULL;
    size_t i = 0;

    *data = ngx_array_create(main_config->pool, num_values, sizeof(ngx_str_t));
    if (*data == NULL)
    {
        return NGX_ERROR;
    }

    va_start(args, num_values);
    for (i = 0; i < num_values; i++)
    {
        array_item = ngx_array_push(*data);
        if (array_item == NULL)
        {
            return NGX_ERROR;
        }
        
        value = va_arg(args, const char *);
        ngx_str_set(array_item, value);
    }
    va_end(args);

    return NGX_OK;
}

/*
 * Validate the cookie prefix to prevent deeper problems later
 */
static ngx_int_t validate_configuration(ngx_conf_t *main_config, const oauth_proxy_configuration_t *module_location_config)
{
    ngx_str_t *trusted_web_origins = NULL;
    ngx_str_t trusted_web_origin;
    const char *literal_http   = "http://";
    const char *literal_https  = "https://";
    size_t max_cookie_name_size = 64;
    ngx_uint_t i = 0;

    if (module_location_config != NULL && module_location_config->enabled)
    {
        if (module_location_config->cookie_name_prefix.len == 0)
        {
            ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "The cookie_name_prefix configuration directive was not provided");
            return NGX_ERROR;
        }

        ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "*** CHECKING LENGTH");
        if (module_location_config->cookie_name_prefix.len > max_cookie_name_size)
        {
            ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "The cookie_name_prefix configuration directive has a maximum length of %d characters", max_cookie_name_size);
            return NGX_ERROR;
        }

        if (module_location_config->encryption_key.len == 0)
        {
            ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "The encryption_key configuration directive was not provided");
            return NGX_ERROR;
        }

        if (module_location_config->encryption_key.len != 64)
        {
            ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "The encryption_key configuration directive must contain 64 hex characters");
            return NGX_ERROR;
        }

        if (module_location_config->trusted_web_origins == NULL || module_location_config->trusted_web_origins->nelts == 0)
        {
            ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "The trusted_web_origin configuration directive was not provided for any web origins");
            return NGX_ERROR;
        }

        trusted_web_origins = module_location_config->trusted_web_origins->elts;
        for (i = 0; i < module_location_config->trusted_web_origins->nelts; i++)
        {
            trusted_web_origin = trusted_web_origins[i];
            if (trusted_web_origin.len < 7)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "An invalid trusted_web_origin configuration directive was provided", &trusted_web_origin);
                return NGX_ERROR;
            }
            
            if (ngx_strncasecmp(trusted_web_origin.data, (u_char*)literal_http,  ngx_strlen(literal_http))  != 0 &&
                ngx_strncasecmp(trusted_web_origin.data, (u_char*)literal_https, ngx_strlen(literal_https)) != 0)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "An invalid trusted_web_origin configuration directive was provided: %V", &trusted_web_origin);
                return NGX_ERROR;
            }
        }
    }

    return NGX_OK;
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
 * Return a request's location configuration
 */
oauth_proxy_configuration_t* get_location_configuration(ngx_http_request_t *request)
{
    return ngx_http_get_module_loc_conf(request, ngx_curity_http_oauth_proxy_module);
}
