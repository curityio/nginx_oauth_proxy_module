#!/usr/bin/perl

use FindBin;
use Test::Nginx::Socket 'no_plan';
run_tests();

__DATA__

=== TEST 1: Client ID configured without secret fails

--- config
location = /t {
    phantom_token on;
    phantom_token_client_credential "client_id"; # Missing secret as 2nd arg
}

--- must_die

--- error_log
invalid number of arguments in "phantom_token_client_credential" directive
