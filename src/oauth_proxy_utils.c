#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>
#include <math.h>
#include "oauth_proxy.h"

/* Forward declarations */
static ngx_int_t oauth_proxy_utils_stringarray_to_headerstring(ngx_http_request_t *request, ngx_str_t *output, ngx_array_t *input);
static ngx_int_t oauth_proxy_utils_integer_to_headerstring(ngx_http_request_t *request, ngx_str_t *output, ngx_int_t input);

/*
 * Do the plumbing to populate an array type from the pool
 */
ngx_int_t oauth_proxy_utils_create_nginx_string_array(ngx_conf_t *main_config, ngx_array_t **data, size_t num_values, ...)
{
    va_list args;
    ngx_str_t *array_item = NULL;
    u_char *value = NULL;
    u_char *buffer = NULL;
    size_t len = 0;
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
        
        value = va_arg(args, u_char *);
        len = ngx_strlen(value);
        buffer = ngx_pcalloc(main_config->pool, len + 1);
        if (buffer == NULL)
        {
            return NGX_ERROR;
        }
        
        memcpy(buffer, value, len);
        buffer[len] = 0;

        array_item->data = buffer;
        array_item->len = len;
    }
    va_end(args);

    return NGX_OK;
}

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
 * Find a header that is not in the standard headers_in structure
 * https://www.nginx.com/resources/wiki/start/topics/examples/headers_management/
 */
ngx_str_t *oauth_proxy_utils_get_header_in(ngx_http_request_t *request, u_char *name, size_t len)
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
ngx_int_t oauth_proxy_utils_get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, const ngx_str_t* cookie_name_prefix, const u_char *cookie_suffix)
{
    u_char cookie_name[128];
    size_t suffix_len = 0;
    ngx_str_t cookie_name_str;

    suffix_len = ngx_strlen(cookie_suffix);
    ngx_memcpy(cookie_name, cookie_name_prefix->data, cookie_name_prefix->len);
    ngx_memcpy(cookie_name + cookie_name_prefix->len, cookie_suffix, suffix_len);
    cookie_name[cookie_name_prefix->len + suffix_len] = 0;

    cookie_name_str.data = cookie_name;
    cookie_name_str.len = ngx_strlen(cookie_name);
    return ngx_http_parse_multi_header_lines(&request->headers_in.cookies, &cookie_name_str, cookie_value);
}

/*
 * Add a single outgoing header
 */
ngx_int_t oauth_proxy_utils_add_header_out(ngx_http_request_t *request, const char *name, ngx_str_t *value)
{
    ngx_table_elt_t *header_element = NULL;

    header_element = ngx_list_push(&request->headers_out.headers);
    if (header_element == NULL)
    {
        return NGX_ERROR;
    }

    header_element->key.data = (u_char *)name;
    header_element->key.len = ngx_strlen(name);
    header_element->value.data = value->data;
    header_element->value.len = value->len;
    header_element->hash = 1;
    return NGX_OK;
}

/*
 * Deal with conversions and then add an outgoing header for a string array
 */
ngx_int_t oauth_proxy_utils_add_stringarray_header_out(ngx_http_request_t *request, const char *name, ngx_array_t *values)
{
    ngx_str_t buffer;
    
    if (oauth_proxy_utils_stringarray_to_headerstring(request, &buffer, values) != NGX_OK)
    {
        return NGX_ERROR;
    }
    
    return oauth_proxy_utils_add_header_out(request, name, &buffer);
}

/*
 * Deal with conversions and then add an outgoing header for an integer
 */
ngx_int_t oauth_proxy_utils_add_integer_header_out(ngx_http_request_t *request, const char *name, ngx_int_t value)
{
    ngx_str_t buffer;

    if (oauth_proxy_utils_integer_to_headerstring(request, &buffer, value) != NGX_OK)
    {
        return NGX_ERROR;
    }

    return oauth_proxy_utils_add_header_out(request, name, &buffer);
}

/*
 * Concatenate array elements to a comma separated buffer with a null terminator, then return a ngx_str_t pointing to heap memory
 */
static ngx_int_t oauth_proxy_utils_stringarray_to_headerstring(ngx_http_request_t *request, ngx_str_t *output, ngx_array_t *input)
{
    u_char *buffer = NULL;
    size_t buffer_size = 0;
    ngx_str_t *items = NULL;
    ngx_str_t item;
    size_t offset = 0;
    size_t i = 0;

    items = input->elts;
    for (i = 0; i < input->nelts; i++)
    {
        item = items[i];
        buffer_size += item.len + 1;
    }

    buffer = ngx_pcalloc(request->pool, buffer_size);
    if (buffer == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate stringarray to string memory");
        return NGX_ERROR;
    }

    for (i = 0; i < input->nelts; i++)
    {
        item = items[i];
        ngx_memcpy(buffer + offset, item.data, item.len);
        offset += item.len;
        buffer[offset] = ',';
        offset += 1;
    }
    buffer[offset - 1] = 0;

    output->data = buffer;
    output->len = buffer_size;
    return NGX_OK;
}

/*
 * Convert an integer to a string and point the output to heap memory
 */
static ngx_int_t oauth_proxy_utils_integer_to_headerstring(ngx_http_request_t *request, ngx_str_t *output, ngx_int_t input)
{
    u_char *buffer;
    u_char *result = NULL;
    size_t buffer_size = 128;
    size_t size = 0;

    buffer = ngx_pcalloc(request->pool, buffer_size);
    if (buffer == NULL)
    {
        ngx_log_error(NGX_LOG_WARN, request->connection->log, 0, "OAuth proxy failed to allocate integer to string memory");
        return NGX_ERROR;
    }

    result = ngx_snprintf(buffer, buffer_size - 1, "%d", (int)input);
    size = result - buffer;
    buffer[size] = 0;

    output->data = buffer;
    output->len = size;
    return NGX_OK;
}
