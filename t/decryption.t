#!/usr/bin/perl

################################################################
# Runs tests related to decryption and handling error conditions
################################################################

use strict;
use warnings;
use Test::Nginx::Socket 'no_plan';

SKIP: {
    our $at_opaque = "42665300-efe8-419d-be52-07b53e208f46";
    our $at_opaque_cookie = "68fa6ae6d1234c50b2f7032e271a8af45a225b404c6bdbd57ac970009cf2c3a875d69aa909a0de1152012c5b08bd23c327e27c5b540ccb8d93eead2e10e8b676";
    
    our $at_jwt = "eyJraWQiOiI2NDQyOTUwMTYiLCJ4NXQiOiJ3S0JrZHB6VmZuaUpTSWIwZ0pWc0VfcU1PN1UiLCJhbGciOiJSUzI1NiJ9.eyJqdGkiOiI4MjVlNGM5MC0xOTc3LTQ4NGYtOWY1MS0yNWFlOGZiMzU0NmYiLCJkZWxlZ2F0aW9uSWQiOiI1NTJiZDhiNy0xOGIwLTQzMTQtODkyZC04OTc1OThkODA5ZGYiLCJleHAiOjE2NDE4MTEwMjEsIm5iZiI6MTY0MTgxMDcyMSwic2NvcGUiOiJwcm9maWxlIiwiaXNzIjoiaHR0cHM6Ly9sb2dpbi1nYXJjaGVyLmV1Lm5ncm9rLmlvL29hdXRoL3YyL29hdXRoLWFub255bW91cyIsInN1YiI6IjRiNmExZDAwNzAyZThkNjA1YzZlOWIwMDQ3Nzk1ZDBmMmM2NjZjZmNlM2UwNDgzNTY5OGJlMzFlYTY1NWM0Y2IiLCJhdWQiOiJqd3QtY2xpZW50IiwiaWF0IjoxNjQxODEwNzIxLCJwdXJwb3NlIjoiYWNjZXNzX3Rva2VuIn0.iSQItk-eScsLifxgFTyF0UQN30couAiczPam4lDTCAHUKjznOPBQJNmbc5tGcog1RcQyvjpAk2wJtObenD0A9Bjk-BQbnhrD0M7In_J1CQaGMmPC6X22lWOmpkJniKudpMqh018xqwec22HmCYWsoUAJQfUfurJa0iin_YkaU-RQAB5rC-JNzY6E4yOrHe6mXiu8iC7c_BNRxcJQ1mAc7WipLzPa047TOL87EXkumwpWeDqH23VB8tXvQHejRYvELDfCxm9CKFvF2QStUFsBB42okkW5HtbmWusR9lqtepXbgM6rRdmbN9kds_tVny394Y6sIU53qx7hyytV88u8_w";
    our $at_jwt_cookie = "c31fe1b83caffc9909f980013e5ca2c4dd92f1d718f345598d49590c1871da61f43a3fc81ce94f13a9ce0266c1433c7ee9c4d6901500b2a9c334b5aecb9507f7e148936d007ba462861ee4e8fec6a015208706aade712e4e4276f8332f36c7a7ee3d0815bc3c5027ace833dd4d91a84e8f4a677b1bc9ec10028a7c4891634738388567ddf78e4d05e831555443b42772c30c11f53fde9baf93de008f110223edd3ba3b2f06ba86ee6441d7a585742bc32dc1afc418a973228c6a9e470e15f3459d12a5b324ee04068ffc84573c6a3381b96df3dc2b174c58d8ddf6c85880dbab966309c0bedc7b0a92ffd2562c94730393f9555aafa91aa295a1fec2c22558b82c893df0d10caaff37f300d2e297288331ee4ce54f30b2bd195cfcca70ee78e08d89e5dad62df4688550d50def54c9153f75d83f58d2b500bbc82bc0f3112c3f504ef4f79f9c37adc8c17135599f979e866673b535a3ade9b76a587d0eacf0cd3e9b87a3c5f6e727c9a2864c7955281846427a5fc10fea43d0c68150496c8d738e6de365f1e9f1b8a14fd1580861c4358e83dc264a9c81c3d2c12c9688e66f51f636e83ad1e9b4deeb6e2c374c3b834c05dc3b1a9d23e173f8014330be161fbb0a04efb3b1fd2f8d21e98bff9887458bc215a9700bbdd6264fde1f634bf8eaaf56860601cfb9c9de7eb11800c803f6c509f3d3b617dab9fa396db187e8c8848b113e0bb873ac2801d687b25219b2f3fe39de7673f1e931c0974ed8c9b5845a97c98569ab8b4baa734e98b99b3ebffe538b0e208a1f8e87926189a5c09aee0440a2e904c2e50e7d12c9d7056b0275b43f0a10290d1e3068239dca76dab1eaa2e65ea55726ef1b3d81b5fde2b50ea632c6faf7ccb8ac5571eb6f9b127e722df4905f8ea16f2729466e0f46f1ef9af9ea405ed8718fdde057e8b0b2395d48ff93ae549e48b17ee625f2fe0309a72d0efdc4ca7e662f0f038e3a4243996be5d2ed8b91cf0339b44f9569ee342a85f8b345d3961bae60ec92db0789bbc50f43174ef72819b3b3c1355928b8f3067910af0f53bd9a2436e32f964f9542dab7676b684baf6e9e772821de19ea3659e0987ab605d16ce42e6ff92653a252762a38b67e85fc88522171c53a139e42bca250de19213fcdf1242432f19f0f4585a06210ca44a5b8ace3a98acef9e2d9d5bf176c38b564a2b385656d909a667191ba9ad8e9c97c7edb9fd1ece1de408c200c5f62acc353d0a645c38dd2ab64ad8d216f834a37787b8453d4236613fe8cb79e5575ba0216bdd5939165306a6eb74ce6a1849557f960ccef2bc36a";

    our $csrf_token = "pQguFsD6hFjnyYjaeC5KyijcWS6AvkJHiUmY7dLUsuTKsLAITLiJHVqsCdQpaGYO";
    our $csrf_cookie = "8eeaa912d27362abfc41866f604696873a628f552ffe36279b88dc405b6c789f83d8a14cbd0302cc9ccf2de6d2ff8793d1d5691e69e1b0b7ee9bddd9dbeb9dd0e63a65b8095adf4d5c8a1f9ed773fb7c0d5e02353ac88b87f656e8d7";
    
    run_tests();
}

__DATA__

=== TEST DECRYPTION_1: A request with a cookie too small to be a valid encrypted payload returns 401
# Verify that obviously invalid data is rejected and does not cause the code to fail

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
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

=== TEST DECRYPTION_2: A POST with 2 valid cookies for an opaque access token succeeds
# Verify that decryption when using opaque access tokens works as expected

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
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

=== TEST DECRYPTION_3: A POST with 2 valid cookies for a JWT access token succeeds
# Verify that decryption when using larger JWT access tokens works as expected

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
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
$data .= "cookie: example-at=" . $main::at_jwt_cookie . "; example-csrf=" . $main::csrf_cookie . "\n";
$data;

--- error_code: 200

--- response_headers eval
"authorization: Bearer " . $main::at_jwt

=== TEST DECRYPTION_4: Handling a request encrypted with a malicious key fails with a 401
# Verify that a request from an attacker sending their own encrypted JWT is not accepted

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
GET /t

--- more_headers eval
my $data;
my $encrypted_at = "3e3e07e23bc88c14b6eb234a135104d391d09a530ce2e3f0b018a3d4c923691c2aa48223cca5c925b47538c540d3ee71c16fa628cbc0ea403756298f8bf56c82";
$data .= "origin: https://www.example.com\n";
$data .= "cookie: example-at=" . $encrypted_at . "\n";
$data;

--- error_code: 401

--- error_log
Problem encountered decrypting data

=== TEST DECRYPTION_5: GET with a tampered IV in the encrypted payload is rejected
# Verify that a request with an altered initialization vector is rejected when replacing the 5th character

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
GET /t

--- more_headers eval
my $replaced_char = substr($main::at_opaque_cookie, 4, 1);
if ($replaced_char eq "a") { $replaced_char = "b"; } else { $replaced_char = "a"; }
my $tampered_cookie = substr($main::at_opaque_cookie, 0, 4) . $replaced_char . substr($main::at_opaque_cookie, 5, length($main::at_opaque_cookie) - 5);

my $data;
$data .= "origin: https://www.example.com\n";
$data .= "cookie: example-at=" . $tampered_cookie . "\n";
$data;

--- error_code: 401

--- error_log
Problem encountered decrypting data

=== TEST DECRYPTION_6: GET with tampered ciphertext in the encrypted payload is rejected
# Verify that a request with an altered ciphertext is rejected when replacing the 45th character

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
GET /t

--- more_headers eval
my $replaced_char = substr($main::at_opaque_cookie, 44, 1);
if ($replaced_char eq "a") { $replaced_char = "b"; } else { $replaced_char = "a"; }
my $tampered_cookie = substr($main::at_opaque_cookie, 0, 44) . $replaced_char . substr($main::at_opaque_cookie, 45, length($main::at_opaque_cookie) - 45);

my $data;
$data .= "origin: https://www.example.com\n";
$data .= "cookie: example-at=" . $tampered_cookie . "\n";
$data;

--- error_code: 401

--- error_log
Problem encountered decrypting data

=== TEST DECRYPTION_6: GET with tampered MAC in the encrypted payload is rejected
# Verify that a request with an altered message authenticaton code is rejected when replacing the 5th from last character

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
}

--- request
GET /t

--- more_headers eval
my $replaced_char = substr($main::at_opaque_cookie, length($main::at_opaque_cookie) - 5, 1);
if ($replaced_char eq "a") { $replaced_char = "b"; } else { $replaced_char = "a"; }
my $tampered_cookie = substr($main::at_opaque_cookie, 0, length($main::at_opaque_cookie) - 5) . $replaced_char . substr($main::at_opaque_cookie, length($main::at_opaque_cookie) - 4, 4);

my $data;
$data .= "origin: https://www.example.com\n";
$data .= "cookie: example-at=" . $tampered_cookie . "\n";
$data;

--- error_code: 401

--- error_log
Problem encountered decrypting data