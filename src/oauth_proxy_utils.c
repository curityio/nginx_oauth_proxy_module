#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>
#include "oauth_proxy.h"

/*
 * Get the CSRF header name into the supplied buffer
 */
void oauth_proxy_utils_get_csrf_header_name(u_char *csrf_header_name, const oauth_proxy_configuration_t *config)
{
    const char *literal_prefix = "x-";
    const char *literal_suffix = "-csrf";
    size_t prefix_length = 2;
    size_t suffix_length = 5;

    ngx_memcpy(csrf_header_name, literal_prefix, prefix_length);
    ngx_memcpy(csrf_header_name + prefix_length, config->cookie_name_prefix.data, config->cookie_name_prefix.len);
    ngx_memcpy(csrf_header_name + prefix_length + config->cookie_name_prefix.len, literal_suffix, suffix_length);
    csrf_header_name[2 + config->cookie_name_prefix.len + suffix_length] = 0;
}

/*
 * Do the plumbing to populate an array type from the pool
 */
ngx_int_t oauth_proxy_utils_create_nginx_string_array(ngx_conf_t *main_config, ngx_array_t **data, size_t num_values, ...)
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