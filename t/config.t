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

=== TEST 2: Plugin with missing encryption key fails to start

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