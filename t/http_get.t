#!/usr/bin/perl

#######################################################################
# Runs HTTP tests to verify security behavior from a client's viewpoint
#######################################################################

use strict;
use warnings;
use Test::Nginx::Socket 'no_plan';

SKIP: {
    our $at_opaque = "42665300-efe8-419d-be52-07b53e208f46";
    our $at_opaque_cookie = "093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389";
    
    run_tests();
}

__DATA__

=== TEST HTTP_GET_1: GET with an authorization header is allowed when enabled
# Verify that mobile and SPA clients can use the same routes, where the first sends access tokens directly

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";

    proxy_pass http://localhost:1984/target;
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
GET /t

--- more_headers
authorization: bearer xxx

--- error_code: 200

=== TEST HTTP_GET_2: GET with an authorization header is rejected when not enabled
# Verify that if a company wants to force mobile and SPA clients to use different routes they can do so

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
GET /t

--- more_headers
origin: https://www.example.com
authorization: bearer xxx

--- error_code: 401

--- error_log
No AT cookie was found in the incoming request

=== TEST HTTP_GET_3: GET without an origin header returns 401
# SPA clients are expected to always send the origin header, as supported by all modern browsers

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

--- error_code: 401

--- error_log
The request did not have an origin header

=== TEST HTTP_GET_4: GET with an untrusted web origin header value returns 401
# Verify that the request is rejected if a malicious site calls the plugin

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
origin: https://www.malicious-site.com

--- error_code: 401

--- error_log
The request was from an untrusted web origin

=== TEST HTTP_GET_5: GET without a cookie or token credential returns 401
# Verify that a 401 is received when there is no message credential at all

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

--- error_code: 401

--- error_log
No AT cookie was found in the incoming request

=== TEST HTTP_GET_6: GET errors return CORS headers so that Javascript can read error details
# The SPA will receive a cryptic network error without these CORS headers

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

--- error_code: 401

--- response_headers
Access-Control-Allow-Origin: https://www.example.com
Access-Control-Allow-Credentials: true

=== TEST HTTP_GET_7: GET with a valid cookie returns 200 and an Authorization header
# Ensure that the happy path for a GET request works

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "mycompany-myproduct";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    
    proxy_pass http://localhost:1984/target;
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
GET /t

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "cookie: mycompany-myproduct-at=" . $main::at_opaque_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque

