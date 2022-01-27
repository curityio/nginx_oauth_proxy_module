#!/usr/bin/perl

################################################################
# Runs tests related to decryption and handling error conditions
################################################################

use strict;
use warnings;
use Test::Nginx::Socket 'no_plan';

SKIP: {
    our $at_opaque = "42665300-efe8-419d-be52-07b53e208f46";
    our $at_opaque_cookie = "AUixxnN28w2MjVK7sMZ3GqErPlw15NwIng-V8amEv5eu43Wr1nzhif1hU2QpKbw_L55GVxD0Kz4gKVG539ywk6g";
    
    our $at_jwt = "eyJraWQiOiI2NDQyOTUwMTYiLCJ4NXQiOiJ3S0JrZHB6VmZuaUpTSWIwZ0pWc0VfcU1PN1UiLCJhbGciOiJSUzI1NiJ9.eyJqdGkiOiI4MjVlNGM5MC0xOTc3LTQ4NGYtOWY1MS0yNWFlOGZiMzU0NmYiLCJkZWxlZ2F0aW9uSWQiOiI1NTJiZDhiNy0xOGIwLTQzMTQtODkyZC04OTc1OThkODA5ZGYiLCJleHAiOjE2NDE4MTEwMjEsIm5iZiI6MTY0MTgxMDcyMSwic2NvcGUiOiJwcm9maWxlIiwiaXNzIjoiaHR0cHM6Ly9sb2dpbi1nYXJjaGVyLmV1Lm5ncm9rLmlvL29hdXRoL3YyL29hdXRoLWFub255bW91cyIsInN1YiI6IjRiNmExZDAwNzAyZThkNjA1YzZlOWIwMDQ3Nzk1ZDBmMmM2NjZjZmNlM2UwNDgzNTY5OGJlMzFlYTY1NWM0Y2IiLCJhdWQiOiJqd3QtY2xpZW50IiwiaWF0IjoxNjQxODEwNzIxLCJwdXJwb3NlIjoiYWNjZXNzX3Rva2VuIn0.iSQItk-eScsLifxgFTyF0UQN30couAiczPam4lDTCAHUKjznOPBQJNmbc5tGcog1RcQyvjpAk2wJtObenD0A9Bjk-BQbnhrD0M7In_J1CQaGMmPC6X22lWOmpkJniKudpMqh018xqwec22HmCYWsoUAJQfUfurJa0iin_YkaU-RQAB5rC-JNzY6E4yOrHe6mXiu8iC7c_BNRxcJQ1mAc7WipLzPa047TOL87EXkumwpWeDqH23VB8tXvQHejRYvELDfCxm9CKFvF2QStUFsBB42okkW5HtbmWusR9lqtepXbgM6rRdmbN9kds_tVny394Y6sIU53qx7hyytV88u8_w";
    our $at_jwt_cookie = "AWqpyPNra0Cfa01-Uay9CVY9OjzLMQ8bbOursLK2j5uBRTJPuWUJOMbfhGt2htTCFR9UHN9MKjdfjFwyQPPlD49iFsoq7J8M5Jbi5TwPSvBqdjWAPQHWQiyGBD5BPwY8xQiPN8TY1T6KFQ_eA1cU47l88B-L4TTGkoiI2ESYZwFO9W_8NSEkPy4n3MPmJREHtOKNnxSTEfbWfJmM8sQ3JwfEtmKdpNO3GN_Rr_6HBQ2CYOQaI8wfIrxGRP6FeEgPNwOh2b3Hxj6voFeJUN6vslhyYh7Lw3mxW8FoUOM91lgcEdyDL50ITQnDepegEklBwYjqQUGjCOPf3AyQBXg8AAefd5Z6BGFz4OarIDaMmbLraptvh2LYNhGgil_vdkqHZ5PFVu1ugxjaytA-kjuh8jq_C4vlm4TwSnS34KWjl-Z7_otgRzegFMLOPPuq2BPyfIrfmP8gOyc7t42YSaOyh7ulSbLwqjej4qYT-JWmgQQ7J5D19rx_UiXdrQ2MTCHdrGbymZ3rDJ6Ed3yvY2jlbVbqSlK3WEJh9lKuL4xZOWWAzM6bI31iwcgDDgo8o84xzCgIEHoXyaNK32Om3liHWIydduUsRjQBELdsScHM-CR5F2XpLpWDMR3XcY4Jll2n6-FNrCE0p3czG_PiNJ075StaQz1kAm-Q_L-sfHpHNGQtPWUcIrFO8WK5ibriIo1kMhUgPDOCTQuhTEDNur4T-GmNjlqzqBodiyQs_OhWoBmbggbpjRTv08d3wvngIHjrJDnV7tSSk28fIdC8FIfQiXK0P4HchhGvKzRQ-2AnC5zK6B6eRiULrKcSsFhk_6wFVgHVPb5tgXiaZnlwJonL__53qr3HZkcrGallKCG2Rbu1Sx0_zRYSoL_TEfuzhzZ3-1LQDPe0kDoYoJlUJeSj8iiHyv9DQa5rjq_5eYdL1yxK9riNTQe1ZddssiV86lQp3z805k3r_wG46Gl8AM97Jo0Q0kVtoGbjmmPW0C4g9xWbGVHnSnMetAnPTvnvMQK30lfzBROVGHASe6IJAVNj_7PMdL46o6fU6VrYfjT8meq3PnbExxJwdzQ0S4KqZbNTDA4YpOqjs3246_M0AAQQrMTkCD2dcEvHnwVWSBCtG9ZBZWO20Bgbw0ti3zVZSRxOR4QTmfNhVAiCiRpPI6jaG_1lojJygVXGcHMkc3t7kTMV0fXKXj74eI5R1e3wikXiu4KXxaPG5xeo9F9W6Muvd5N452w3s5Mz";

    our $csrf_token = "pQguFsD6hFjnyYjaeC5KyijcWS6AvkJHiUmY7dLUsuTKsLAITLiJHVqsCdQpaGYO";
    our $csrf_cookie = "AcdY11SVolhDSduFnfe-83_26jWo8zA4K4x-kT2WtjTLal6PAg6GFjnB3CZqWbDHhIfYYTm_ubeDi92bJjc4CTeZXIEFGhZr3jvyXnaHDW-ZlD6Z_KgcRgcViUWa";
    
    run_tests();
}

__DATA__

=== TEST DECRYPTION_1: A request with a cookie too small to be a valid encrypted payload returns 401
# Verify that obviously invalid data is rejected and does not cause the code to fail

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
}

--- request
GET /t

--- more_headers
origin: https://www.example.com
cookie: example-at=x

--- error_code: 401

--- error_log
 Invalid data length after decoding from base64

=== TEST DECRYPTION_2: A POST with 2 valid cookies for an opaque access token succeeds
# Verify that decryption when using opaque access tokens works as expected

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;

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
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;

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
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
}

--- request
GET /t

--- more_headers eval
my $data;
my $encrypted_at = "AcYBf995tTBVsLtQLvOuLUZXHm2c-XqP8t7SKmhBiQtzy5CAw4h_RF6rXyg6kHrvhb8x4WaLQC6h3mw6a3O3Q9A";
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
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
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
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
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

=== TEST DECRYPTION_7: GET with tampered MAC in the encrypted payload is rejected
# Verify that a request with an altered message authenticaton code is rejected when replacing the 5th from last character

--- config
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "7b99279ab87533d3c238db874a842a91ee26a76027f3c03c317504963d2c9926";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
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