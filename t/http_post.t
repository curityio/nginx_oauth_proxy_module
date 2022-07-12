#!/usr/bin/perl

#######################################################################
# Runs HTTP tests to verify security behavior from a client's viewpoint
#######################################################################

use strict;
use warnings;
use Test::Nginx::Socket 'no_plan';

SKIP: {
    our $at_opaque = "42665300-efe8-419d-be52-07b53e208f46";
    our $at_opaque_cookie = "AcYBf995tTBVsLtQLvOuLUZXHm2c-XqP8t7SKmhBiQtzy5CAw4h_RF6rXyg6kHrvhb8x4WaLQC6h3mw6a3O3Q9A";
    
    our $csrf_token = "pQguFsD6hFjnyYjaeC5KyijcWS6AvkJHiUmY7dLUsuTKsLAITLiJHVqsCdQpaGYO";
    our $csrf_cookie = "AfctuC2zuBeZoQHfbopmpQyOADYU6Tp9raMEA-2EhWp4I3HtoiAtoP-H2U_PIrF7O0ZQ0nwE7VmWcl3BAY6bGlv4_EGqToyh4lOqynkSlBByxixJY-kA3bIFufJl";
    
    run_tests();
}

__DATA__

=== TEST HTTP_POST_1: POST with no CSRF cookie returns 401
###############################################################################
# Data changing commands require CSRF details in line with OWASP best practices
###############################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
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

--- response_headers
content-type: application/json

=== TEST HTTP_POST_2: POST with no CSRF request header returns 401
############################################################
# A request header should be sent along with the CSRF cookie
############################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
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

--- response_headers
content-type: application/json

=== TEST HTTP_POST_3: POST with mismatched CSRF request header and cookie returns 401
##################################################################
# The request header value must match that in the encrypted cookie
##################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
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

--- response_headers
content-type: application/json

=== TEST HTTP_POST_4: POST with 2 valid cookies and a CSRF token returns 200
#############################################################
# Verify that the happy path works for data changing commands
#############################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

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

=== TEST HTTP_POST_5: POST with 2 locations and same details works as expected
######################################################################
# Verify that the happy path works for multiple configuration sections
######################################################################

--- config
location /api1 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

    proxy_pass http://localhost:1984/target;
}
location /api2 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

    proxy_pass http://localhost:1984/target;
}    
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
POST /api2

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "x-example-csrf: " . $main::csrf_token . "\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "; example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque

=== TEST HTTP_POST_6: POST with 2 locations and different details works as expected
########################################################################################
# Verify that the happy path works for data changing commands with independent locations
########################################################################################

--- config
location /api1 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example1";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example1.com";
    oauth_proxy_cors_enabled on;

    proxy_pass http://localhost:1984/target;
}
location /api2 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example2";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example2.com";
    oauth_proxy_cors_enabled on;

    proxy_pass http://localhost:1984/target;
}    
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
POST /api2

--- more_headers eval
my $data;
$data .= "origin: https://www.example2.com\n";
$data .= "x-example2-csrf: " . $main::csrf_token . "\n";
$data .= "cookie: example2-at=" . $main::at_opaque_cookie . "; example2-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque

=== TEST HTTP_POST_7: POST with parent child locations uses inherited properties as expected
########################################################################################
# Verify that the happy path works for data changing commands with independent locations
########################################################################################

--- config
location /api {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

    location /api/products {
        proxy_pass http://localhost:1984/target;
    } 
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
POST /api/products

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "x-example-csrf: " . $main::csrf_token . "\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "; example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque

=== TEST HTTP_POST_8: POST with HTTP/2 and multiple cookie headers
#################################################################################################
# When running with HTTP/2 the cookie header can be sent multiple times so verify that this works
#################################################################################################

--- config
location /api {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

    location /api/products {
        proxy_pass http://localhost:1984/target;
    } 
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
POST /api/products

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "x-example-csrf: " . $main::csrf_token . "\n";
$data .= "cookie: example-at=" . $main::at_opaque_cookie . "\n";
$data .= "cookie: example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque

=== TEST HTTP_POST_9: POST with a cookie prefix on the maximum length boundary
###########################################################
# This ensures no overflows if a long cookie prefix is used
###########################################################

--- config
location /api {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "myveryveryverylongcompanyname-myveryveryveryverylongproductname";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

    location /api/products {
        proxy_pass http://localhost:1984/target;
    } 
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}

--- request
POST /api/products

--- more_headers eval
my $data;
$data .= "origin: https://www.example.com\n";
$data .= "x-myveryveryverylongcompanyname-myveryveryveryverylongproductname-csrf: " . $main::csrf_token . "\n";
$data .= "cookie: myveryveryverylongcompanyname-myveryveryveryverylongproductname-at=" . $main::at_opaque_cookie . "; myveryveryverylongcompanyname-myveryveryveryverylongproductname-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_opaque