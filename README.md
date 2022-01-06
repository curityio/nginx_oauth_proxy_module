# OAuth Proxy NGINX Module

[![Quality](https://img.shields.io/badge/quality-production-green)](https://curity.io/resources/code-examples/status/)
[![Availability](https://img.shields.io/badge/availability-binary-blue)](https://curity.io/resources/code-examples/status/)

An NGINX module that decrypts secure cookies received from Single Page Applications.\
This is used to implement the OAuth Proxy component of the [Token Handler Pattern](https://curity.io/resources/learn/the-token-handler-pattern/).

![Token Handler Pattern](doc/token-handler-pattern.png)

## Security Overview

The OAuth proxy is a forwarder to deal with cookie specific logic and transation.\
A typical flow for an SPA calling an API would work like this:

![Security Handling](doc/security-handling.png)

- The SPA makes an API call with an AES256 encrypted HTTP Only cookie containing an opaque access token
- The OAuth Proxy module decrypts the cookie to get the opaque access token
- The opaque access token is then forwarded in the HTTP Authorization Header
- The [Phantom Token Module](https://github.com/curityio/nginx_phantom_token_module) then swaps the opaque token for a JWT
- The HTTP Authorization Header is then updated with a JWT that is forwarded to the API
- The API must then verify the JWT in the standard zero trust manner, on every request

## Configuration Directives

The following settings can be configured for one or more NGINX routes:

| Setting | Behavior |
| ------- | -------- |
| oauth_proxy | Set to `on` or `off` to enable or disable the plugin |
| oauth_proxy_allow_tokens | Allow disabling of cookie checks for requests that already have an access token |
| oauth_proxy_cookie_prefix | Set the preferred string prefix used in cookies, such as `example` below |
| oauth_proxy_hex_encryption_key | An AES256 hex encryption key, which must be 64 hex characters |
| oauth_proxy_trusted_web_origins | Set trusted web origins that are allowed to call the OAuth Proxy |

```nginx
location /products {

    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origins "https://www.example.com";

    proxy_pass "https://products-api.example.com";
}
```

## Cookies Used

The plugin uses two cookies, with a customizable prefix and known suffixes:

| Cookie | Contains |
| ------ | -------- |
| example-at | An access token cookie, which can contain either an opaque token or a JWT |
| example-csrf | A CSRF cookie containing random hex bytes used for double submit cookie checks |

## Cookie Encryption Details

AES256-GCM is used, with a hex encoding, meaning that each cookie value consists of these parts:

| Cookie Part | Contains |
| ----------- | -------- |
| First 24 hex digits | This contains the 12 byte initialization vector |
| Last 32 hex digits | This contains the 16 byte message authentication code |
| Middle section | This contains the ciphertext and its length matches that of the token being encrypted |

## HTTP Methods

The plugin has three main behaviors, depending on the HTTP method:

| Method | Behavior |
| ------ | -------- |
| OPTIONS | For CORS pre-flight requests from the SPA, the plugin returns immediately |
| GET | Decrypts the cookie and forwards an access token, or returns 401 if there is no valid cookie |
| PUT, POST, PATCH, DELETE | Also applies double submit cookie checks, to verify a matching CSRF request header |

See the [OWASP Best Practices](https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html) for further information on Cross Site Request Forgery checks.

## Implementation Details

To build the code yourself or to adapt it, see the ![Implementation Guide](Implementation.md)

## More Information

Please visit [curity.io](https://curity.io/) for more information about the Curity Identity Server.
