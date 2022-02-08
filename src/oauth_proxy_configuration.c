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
 * Set default options that are not provided in the nginx.conf file
 */
static ngx_int_t apply_configuration_defaults(ngx_conf_t *main_config, oauth_proxy_configuration_t *config)
{
    const char *default_methods = "OPTIONS,HEAD,GET,POST,PUT,PATCH,DELETE";
    ngx_int_t default_max_age = 86400;
    u_char *buffer = NULL;
    size_t len = 0;

    if (config->cors_enabled)
    {
        if (config->cors_allow_methods.data == NULL)
        {
            len = ngx_strlen(default_methods);
            buffer = ngx_pcalloc(main_config->pool, len + 1);
            if (buffer == NULL)
            {
                ngx_conf_log_error(NGX_LOG_WARN, main_config, 0, "Unable to allocate memory for cors_allow_methods");
                return NGX_ERROR;
            }
            
            buffer[len] = 0;
            config->cors_allow_methods.data = buffer;
            config->cors_allow_methods.len = len;
        }

        if (config->cors_max_age == -1)
        {
            config->cors_max_age = default_max_age;
        }
    }

    return NGX_OK;
}

/*
 * Validate the cookie prefix to prevent deeper problems later
 */
static ngx_int_t validate_configuration(ngx_conf_t *main_config, const oauth_proxy_configuration_t *module_location_config)
{
    size_t max_cookie_name_size = 64;

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
    }

    return NGX_OK;
}