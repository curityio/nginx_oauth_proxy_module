/*
 *  Copyright 2021 Curity AB
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
    ngx_flag_t enable;
    ngx_str_t cookie_prefix;
    ngx_str_t hex_encryption_key;
    ngx_str_t trusted_web_origins;
} oauth_proxy_configuration_t;

typedef struct
{
    ngx_uint_t done;
    ngx_uint_t status;
} oauth_proxy_module_context_t;

/* Forward declarations */
static ngx_int_t post_configuration(ngx_conf_t *config);
static ngx_int_t handler(ngx_http_request_t *request);
static void *create_location_configuration(ngx_conf_t *config);
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child);
static ngx_int_t get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, ngx_str_t* cookie_prefix, const char *cookie_suffix);

/* Configuration data */
static ngx_command_t oauth_proxy_module_directives[] =
{
    {
          ngx_string("oauth_proxy"),
          NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
          ngx_conf_set_flag_slot,
          NGX_HTTP_LOC_CONF_OFFSET,
          offsetof(oauth_proxy_configuration_t, enable),
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
        ngx_string("oauth_proxy_trusted_web_origins"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
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
    ngx_conf_log_error(NGX_LOG_WARN, config, 0, "*** OAUTH PROXY: create_location_configuration called");
    oauth_proxy_configuration_t *location_config = ngx_pcalloc(config->pool, sizeof(oauth_proxy_configuration_t));

    if (location_config == NULL)
    {
        return NGX_CONF_ERROR;
    }

    location_config->enable = NGX_CONF_UNSET_UINT;
    return location_config;
}

/*
 * Called when NGINX starts up and finds a parent location that uses the plugin
 */
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child)
{
    ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "*** OAUTH PROXY: merge_location_configuration called");
    oauth_proxy_configuration_t *parent_config = parent, *child_config = child;

    // This shows the input value
    ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "*** OAUTH PROXY: child encryption key is %V", &child_config->hex_encryption_key);

    ngx_conf_merge_off_value(child_config->enable, parent_config->enable, 0)
    ngx_conf_merge_str_value(child_config->hex_encryption_key, parent_config->hex_encryption_key, "")

    return NGX_CONF_OK;
}

/*
 * Receive and validate the configuration data
 */
static ngx_int_t post_configuration(ngx_conf_t *config)
{
    ngx_conf_log_error(NGX_LOG_WARN, config, 0, "*** OAUTH PROXY: post_configuration called");
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
 * Called during HTTP requests to make cookie related checks and then to decrypt the cookie to get an access token
 */
static ngx_int_t handler(ngx_http_request_t *request)
{
    oauth_proxy_configuration_t *module_location_config = ngx_http_get_module_loc_conf(
            request, ngx_curity_http_oauth_proxy_module);

    if (!module_location_config->enable)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy module is disabled");
        return NGX_DECLINED;
    }

    if (ngx_strncasecmp(request->method_name.data, (u_char*)"OPTIONS", 7) == 0)
    {
        return NGX_DECLINED;
    }

    ngx_str_t at_cookie_value;
    ngx_int_t at_cookie_result = get_cookie(request, &at_cookie_value, &module_location_config->cookie_prefix, "-at");
    if (at_cookie_result == NGX_HTTP_INTERNAL_SERVER_ERROR)
    {
        return at_cookie_result;
    }

    if (at_cookie_result == NGX_DECLINED)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "*** OAUTH PROXY HANDLER: cookie not found");
    }
    else
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "*** OAUTH PROXY HANDLER: cookie found: %V", &at_cookie_value);
    }

    return NGX_OK;
}

/*
 * A utility method to get a cookie and deal with strings
 */
static ngx_int_t get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, ngx_str_t* cookie_prefix, const char *cookie_suffix)
{
    size_t cookie_name_len = cookie_prefix->len + ngx_strlen(cookie_suffix);
    u_char *cookie_name = ngx_pcalloc(request->pool, cookie_name_len);
    if (cookie_name == NULL)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_snprintf(cookie_name, cookie_name_len, "%V%s", cookie_prefix, cookie_suffix);
    ngx_str_t cookie_name_str = (ngx_str_t)ngx_string(cookie_name);
    cookie_name_str.len = cookie_name_len;

    return ngx_http_parse_multi_header_lines(&request->headers_in.cookies, &cookie_name_str, cookie_value);
}