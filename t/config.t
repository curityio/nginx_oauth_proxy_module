#!/usr/bin/perl

use FindBin;
use Test::Nginx::Socket 'no_plan';
run_tests();

__DATA__

=== TEST 1: Plugin not active works

--- config
location tt {
    oauth_proxy off;
}

--- request
GET /

--- error_code: 200

=== TEST 2: Plugin with missing cookie prefix fails to start

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origins "https://www.example.com";
}

--- must_die

--- error_log
The cookie_prefix configuration directive was not provided

=== TEST 3: Plugin with too long cookie prefix fails to start

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example-example-example-example-example-example-example-example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origins "https://www.example.com";
}

--- must_die

--- error_log
The cookie_prefix configuration directive has a maximum length of 32 characters

=== TEST 4: Plugin with missing encryption key fails to start

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_trusted_web_origins "https://www.example.com";
}

--- must_die

--- error_log
The hex_encryption_key configuration directive was not provided

=== TEST 5: Plugin with invalid length encryption key fails to start

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d6";
    oauth_proxy_trusted_web_origins "https://www.example.com";
}

--- must_die

--- error_log
The hex_encryption_key configuration directive must contain 64 hex characters