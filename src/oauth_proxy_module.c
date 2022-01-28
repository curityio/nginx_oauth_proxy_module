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

/* Forward declarations */
static void *create_location_configuration(ngx_conf_t *config);
static char *merge_location_configuration(ngx_conf_t *main_config, void *parent, void *child);
static ngx_int_t post_configuration(ngx_conf_t *config);

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
 * An export to return configuration to the handler
 */
oauth_proxy_configuration_t* oauth_proxy_module_get_location_configuration(ngx_http_request_t *request)
{
    return ngx_http_get_module_loc_conf(request, ngx_curity_http_oauth_proxy_module);
}

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
    
    if (oauth_proxy_configuration_initialize_location(main_config, child_config) != NGX_OK)
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

    *h = oauth_proxy_handler_main;
    return NGX_OK;
}
