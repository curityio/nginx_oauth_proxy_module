#!/usr/bin/perl

################################################################
# Runs tests related to decryption and handling error conditions
################################################################

use FindBin;
use Test::Nginx::Socket 'no_plan';
run_tests();

__DATA__

=== TEST DECRYPTION_1: A request with a cookie too small to be a valid encrypted payload returns 401

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
GET /t

--- more_headers
origin: https://www.example.com
cookie: example-at=x

--- error_code: 401

--- error_log
The encrypted hex payload had an invalid length
