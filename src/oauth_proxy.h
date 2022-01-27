typedef struct
{
    ngx_flag_t enabled;
    ngx_str_t cookie_name_prefix;
    ngx_str_t encryption_key;
    ngx_array_t *trusted_web_origins;
    ngx_flag_t cors_enabled;
    ngx_flag_t allow_tokens;
    ngx_flag_t remove_cookie_headers;
    ngx_array_t *cors_allow_methods;
    ngx_array_t *cors_allow_headers;
    ngx_array_t *cors_expose_headers;
    ngx_int_t cors_max_age;
} oauth_proxy_configuration_t;

#define MAX_COOKIE_PREFIX_LENGTH 64
#define MAX_COOKIE_SUFFIX_LENGTH 5
