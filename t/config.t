#!/usr/bin/perl

use FindBin;
use Test::Nginx::Socket 'no_plan';
run_tests();

__DATA__

=== TEST 1: Plugin not active

--- config
location tt {
    oauth_proxy off;
}

--- request
GET /

--- error_code: 200
