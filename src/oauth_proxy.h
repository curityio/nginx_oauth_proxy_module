/* Exported types */
typedef struct
{
    ngx_flag_t enabled;
    ngx_str_t cookie_name_prefix;
    ngx_str_t encryption_key;
    ngx_array_t *trusted_web_origins;
    ngx_flag_t cors_enabled;
    ngx_flag_t allow_tokens;
    ngx_array_t *cors_allow_methods;
    ngx_array_t *cors_allow_headers;
    ngx_array_t *cors_expose_headers;
    ngx_int_t cors_max_age;
} oauth_proxy_configuration_t;

/* Exported functions */
oauth_proxy_configuration_t* oauth_proxy_module_get_location_configuration(ngx_http_request_t *request);
ngx_int_t oauth_proxy_configuration_initialize_location(ngx_conf_t *main_config, oauth_proxy_configuration_t *child_config);
ngx_int_t oauth_proxy_handler_main(ngx_http_request_t *request);
ngx_int_t oauth_proxy_decryption_decrypt_cookie(ngx_http_request_t *request, ngx_str_t *plain_text, const ngx_str_t* ciphertext, const ngx_str_t* encryption_key_hex);
int oauth_proxy_encoding_bytes_from_hex(u_char *bytes, const u_char *hex, size_t hex_len);
int oauth_proxy_encoding_base64_url_decode(u_char *bufplain, const u_char *bufcoded);
ngx_int_t oauth_proxy_utils_create_nginx_string_array(ngx_conf_t *main_config, ngx_array_t **data, size_t num_values, ...);
void oauth_proxy_utils_get_csrf_header_name(u_char *csrf_header_name, const oauth_proxy_configuration_t *config);
ngx_str_t *oauth_proxy_utils_get_header_in(ngx_http_request_t *request, u_char *name, size_t len);
ngx_int_t oauth_proxy_utils_get_cookie(ngx_http_request_t *request, ngx_str_t* cookie_value, const ngx_str_t* cookie_name_prefix, const u_char *cookie_suffix);
ngx_int_t oauth_proxy_utils_add_header_out(ngx_http_request_t *request, const char *name, ngx_str_t *value);
ngx_int_t oauth_proxy_utils_add_stringarray_header_out(ngx_http_request_t *request, const char *name, ngx_array_t *values);
ngx_int_t oauth_proxy_utils_add_integer_header_out(ngx_http_request_t *request, const char *name, ngx_int_t value);
