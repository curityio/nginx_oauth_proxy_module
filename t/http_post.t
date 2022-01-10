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
    
    our $csrf_token = "pQguFsD6hFjnyYjaeC5KyijcWS6AvkJHiUmY7dLUsuTKsLAITLiJHVqsCdQpaGYO";
    our $csrf_cookie = "f61b300a79018b4b94f480086d63395148084af1f20c3e474623e60f34a181656b3a54725c1b4ddaeec9171f0398bde8c6c1e0e12d90bdb13397bf24678cd17a230a3df8e1771f9992e3bf2d6567ad920e1c25dc5e3e015679b5e673";
    
    run_tests();
}

__DATA__

=== TEST HTTP_POST_1: POST with no CSRF cookie returns 401
# Data changing commands require CSRF details in line with OWASP best practices

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
POST /t

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "\n";
$data;

--- error_code: 401

--- error_log
No CSRF cookie was found in the incoming request

=== TEST HTTP_POST_2: POST with no CSRF request header returns 401
# A request header should be sent along with the CSRF cookie

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
POST /t

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "; example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 401

--- error_log
A data changing request did not have a CSRF header

=== TEST HTTP_POST_3: POST with mismatched CSRF request header and cookie returns 401
# The request header value must match that in the encrypted cookie

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
POST /t

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "x-example-csrf: x" . $main::csrf_token . "\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "; example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 401

--- error_log
The CSRF request header did not match the value in the encrypted CSRF cookie

=== TEST HTTP_POST_4: POST with 2 valid cookies and a CSRF token returns 200
# Verify that the happy path works for data changing commands

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
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
POST /t

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "x-example-csrf: " . $main::csrf_token . "\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "; example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque