#!/usr/bin/perl

########################################################################
# Runs configuration tests to verify that only correct input is accepted
########################################################################

use strict;
use warnings;
use Test::Nginx::Socket 'no_plan';
run_tests();

__DATA__

=== TEST CONFIG_1: NGINX starts OK when the module is deactivated
################################################################################
# Routing to the target does not require config settings or run the module logic
################################################################################

--- config
location /t {
    oauth_proxy off;
    proxy_pass http://localhost:1984/target;
}
location /target {
    return 200;
}

--- request
GET /t

--- error_code: 200

=== TEST CONFIG_2: A deployment with empty configuration is correctly detected and logged
####################################################################################
# Verify that null configuration is handled in a controlled manner and fails to load
####################################################################################

--- config
location /t {
    oauth_proxy on;
}

--- must_die

--- request
GET /t

--- error_code: 500

--- error_log
The cookie_name_prefix configuration directive was not provided

--- response_body_like chomp
{"code":"server_error","message":"Problem encountered processing the request"}

=== TEST CONFIG_3: NGINX quits when no encryption key is configured for a location
##################################################################
# The module correctly validates required parameters for each path
##################################################################

--- config
location /first {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
}
location /second {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
}

--- must_die

--- request
GET /t

--- error_log
The encryption_key configuration directive was not provided

=== TEST CONFIG_4: NGINX quits when the cookie prefix configured is abnormally long
####################################################################################################
# A 64 character limit is used for the prefix to allow stack allocation based on a known buffer size
####################################################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "abcdefghijklmnopqrstuvwxyz-0123456789-abcdefghijklmnopqrstuvwxyz-0123456789";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
}

--- must_die

--- error_log
The cookie_name_prefix configuration directive has a maximum length of 64 characters

=== TEST CONFIG_5: NGINX quits when an invalid length 256 bit encryption key is configured
################################################################
# This ensures that input has an expected length before using it
################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d6";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
}

--- must_die

--- error_log
The encryption_key configuration directive must contain 64 hex characters

=== TEST CONFIG_6: NGINX quits when no trusted web origins are configured
################################################################################
# The module is only used for SPAs so it makes sense to always have at least one
################################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_cors_enabled on;
}

--- must_die

--- error_log
The trusted_web_origin configuration directive was not provided for any web origins

=== TEST CONFIG_7: NGINX starts correctly with a valid configuration
#########################################################################################
# Verifies the most standard happy case, to ensure that the SPA can get data from the API
#########################################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "http://www.example.com";
    oauth_proxy_cors_enabled on;
    return 200;
}
location /target {
    return 200;
}

--- request
GET /t

--- error_code: 200

=== TEST CONFIG_8: NGINX starts correctly with a valid configuration and multiple web origins
###################################################################################
# For cases where a single API potentially serves multiple SPAs that share a cookie
###################################################################################

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://webapp1.example.com";
    oauth_proxy_trusted_web_origin "https://webapp2.example.com";
    oauth_proxy_cors_enabled on;
    return 200;
}

--- request
GET /t

--- error_code: 200

=== TEST CONFIG_9: NGINX starts correctly with two valid root paths
###############################################################################
# For cases where multiple routes serve multiple SPAs in the same reverse proxy
###############################################################################

--- config
location /api1 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.domain1.com";
    oauth_proxy_cors_enabled on;
    return 500;
}
location /api2 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.domain2.com";
    oauth_proxy_cors_enabled on;
    return 200;
}

--- request
GET /api2

--- error_code: 200

=== TEST CONFIG_10: NGINX quits when one of two configurations is invalid
###############################################################
# Verifies that multiple configurations are correctly validated
###############################################################

--- config
location /api1 {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.domain1.com";
    oauth_proxy_cors_enabled on;
    return 200;
}
location /api2 {
    oauth_proxy on;
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.domain2.com";
    oauth_proxy_cors_enabled on;
    return 200;
}

--- must_die

--- error_log
The cookie_name_prefix configuration directive was not provided

=== TEST CONFIG_11: NGINX starts correctly with inherited settings for a child path
#########################################################################
# Verifies that the module runs for child paths when enabled for a parent
#########################################################################

--- config
location /root {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;

    location /root/api {
        proxy_pass http://localhost:1984/target;
    }
}
location /target {
    return 200;
}

--- request
GET /root/api

--- error_code: 401

--- error_log
The request did not have an origin header
