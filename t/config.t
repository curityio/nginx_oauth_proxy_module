#!/usr/bin/perl

########################################################################
# Runs configuration tests to verify that only correct input is accepted
########################################################################

use FindBin;
use Test::Nginx::Socket 'no_plan';
run_tests();

__DATA__

=== TEST C1: NGINX starts OK when the module is deactivated

--- config
location /t {
    oauth_proxy off;
    return 200;
}

--- request
GET /t

--- error_code: 200

=== TEST C2: NGINX fails to start when a missing cookie prefix is configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- must_die

--- error_log
The cookie_prefix configuration directive was not provided

=== TEST C3: NGINX fails to start when a too long cookie prefix is configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example-example-example-example-example-example-example-example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- must_die

--- error_log
The cookie_prefix configuration directive has a maximum length of 32 characters

=== TEST C4: NGINX fails to start when a missing encryption key is configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- must_die

--- error_log
The hex_encryption_key configuration directive was not provided

=== TEST C5: NGINX fails to start when an invalid length 256 bit encryption key is configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d6";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- must_die

--- error_log
The hex_encryption_key configuration directive must contain 64 hex characters

=== TEST C6: NGINX fails to start when no trusted web origins are configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
}

--- must_die

--- error_log
The trusted_web_origin configuration directive was not provided for any web origins

=== TEST C7: NGINX fails to start when an empty trusted web origin is configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "";
}

--- must_die

--- error_log
An invalid trusted_web_origin configuration directive was provided

=== TEST C8: NGINX fails to start when an invalid trusted web origin is configured

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "htt://www.example.com";
}

--- must_die

--- error_log
An invalid trusted_web_origin configuration directive was provided: htt://www.example.com

=== TEST C9: NGINX starts correctly with a valid configuration

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "http://webapp1.example.com";
    oauth_proxy_trusted_web_origin "http://webapp2.example.com";
    return 200;
}

--- request
GET /

--- error_code: 200

=== TEST C10: NGINX starts correctly with a valid configuration with multiple web origins

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "http://webapp1.example.com";
    oauth_proxy_trusted_web_origin "https://webapp2.example.com";
    return 200;
}

--- request
GET /

--- error_code: 200