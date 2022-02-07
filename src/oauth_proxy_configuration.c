#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>
#include "oauth_proxy.h"

/* Forward declarations */
static ngx_int_t apply_configuration_defaults(ngx_conf_t *main_config, oauth_proxy_configuration_t *config);
static ngx_int_t validate_configuration(ngx_conf_t *main_config, const oauth_proxy_configuration_t *module_location_config);

/*
 * Default and validate the configuration for a location when NGINX starts up
 */
ngx_int_t oauth_proxy_configuration_initialize_location(ngx_conf_t *main_config, oauth_proxy_configuration_t *child_config)
{
    if (apply_configuration_defaults(main_config, child_config) != NGX_OK)
    {
        return NGX_ERROR;
    }

    if (validate_configuration(main_config, child_config) != NGX_OK)
    {
        return NGX_ERROR;
    }
    
    return NGX_OK;
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
            ret_code = oauth_proxy_utils_create_nginx_string_array(main_config, &config->cors_allow_methods, 7,
                                                "OPTIONS", "GET", "HEAD", "POST", "PUT", "PATCH", "DELETE");
            if (ret_code != NGX_OK)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "Unable to allocate memory for cors_allow_methods");
                return ret_code;
            }
        }

        if (config->cors_allow_headers == NULL)
        {
            oauth_proxy_utils_get_csrf_header_name(csrf_header_name, config);
            ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "***CSRF HEADER NAME: %s", csrf_header_name);


            ret_code = oauth_proxy_utils_create_nginx_string_array(main_config, &config->cors_allow_headers, 1, csrf_header_name);
            if (ret_code != NGX_OK)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "Unable to allocate memory for cors_allow_headers");
                return ret_code;
            }
        }

        if (config->cors_expose_headers == NULL)
        {
            ret_code = oauth_proxy_utils_create_nginx_string_array(main_config, &config->cors_expose_headers, 0);
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