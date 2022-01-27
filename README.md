# OAuth Proxy NGINX Module

[![Quality](https://img.shields.io/badge/quality-production-green)](https://curity.io/resources/code-examples/status/)
[![Availability](https://img.shields.io/badge/availability-binary-blue)](https://curity.io/resources/code-examples/status/)

An NGINX module that decrypts secure cookies in API calls from Single Page Applications.\
This is the OAuth Proxy component of the [Token Handler Pattern](https://curity.io/resources/learn/the-token-handler-pattern/).

![Token Handler Pattern](resources/token-handler-pattern.png)

## High Level Usage

The OAuth proxy is a forwarder placed in front of your business APIs, to deal with cookie authorization.\
A typical flow for an SPA calling an API would work like this:

![Security Handling](resources/security-handling.png)

- The SPA sends an AES256 encrypted HTTP Only cookie containing an opaque access token
- The OAuth Proxy module decrypts the cookie to get the opaque access token
- The opaque access token is then forwarded in the HTTP Authorization Header
- The [Phantom Token Module](https://github.com/curityio/nginx_phantom_token_module) then swaps the opaque token for a JWT access token
- The incoming HTTP Authorization Header is then updated with the JWT access token
- The API must then verify the JWT in a zero trust manner, on every request

## Required Configuration Directives

All of the directives are required for locations where the module is enabled.\
NGINX will fail to load if the configuration for any locations fail validation:

#### oauth_proxy

> **Syntax**: **`oauth_proxy`** `on` | `off`
>
> **Context**: `location`

The module is disabled by default but can be enabled for paths you choose.

#### oauth_proxy_cookie_name_prefix

> **Syntax**: **`oauth_proxy_cookie_name_prefix`** `string`
>
> **Context**: `location`

The prefix used in the SPA's cookie name, typically representing a company or product name.\
The value supplied must not be empty, and `example` would lead to full cookie names such as `example-at`.

#### oauth_proxy_encryption_key

> **Syntax**: **`oauth_proxy_encryption_key`** `string`
>
> **Context**: `location`

This must be a 32 byte encryption key expressed as 64 hex characters.\
It is used to decrypt AES256 encrypted secure cookies.\
The key is initially generated with a tool such as `openssl`, as explained in Curity tutorials.

#### oauth_proxy_trusted_web_origins

> **Syntax**: **`oauth_proxy_trusted_web_origins`** `string[]`
>
> **Context**: `location`

A whitelist of at least one web origin from which the plugin will accept requests.\
Multiple origins could be used in special cases where cookies are shared across subdomains.

#### oauth_proxy_cors_enabled

> **Syntax**: **`oauth_proxy_cors_enabled`** `boolean`
>
> **Default**: *true*
>
> **Context**: `location`

When enabled, the OAuth proxy returns CORS response headers on behalf of the API.\
When an origin header is received that is in the trusted_web_origins whitelist, response headers are written.\
The [access-control-allow-origin](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Origin) header is returned, so that the SPA can call the API.\
The [access-control-allow-credentials](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Credentials) header is returned, so that the SPA can send secured cookies to the API.

## Optional Configuration Directives

#### oauth_proxy_allow_tokens

> **Syntax**: **`oauth_proxy_allow_tokens`** `boolean`
>
> **Default**: *false*
>
> **Context**: `location`

If set to true, then requests that already have a bearer token are passed straight through to APIs.\
This can be useful when web and mobile clients share the same API routes.

#### oauth_proxy_remove_cookie_headers

> **Syntax**: **`oauth_proxy_remove_cookie_headers`** `boolean`
>
> **Default**: *true*
>
> **Context**: `location`

If set to true, then cookie and CSRF headers are not forwarded to APIs.\
This provides cleaner requests to APIs, which only receive a JWT in the HTTP Authorization header.

#### oauth_proxy_cors_allow_methods

> **Syntax**: **`oauth_proxy_cors_allow_methods`** `string[]`
>
> **Default**: *['OPTIONS', 'GET', 'HEAD', 'POST', 'PUT', 'PATCH', 'DELETE']*
>
> **Context**: `location`

When CORS is enabled, these values are returned in the [access-control-allow-methods](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Methods) response header.\
The SPA is then allowed to call a particular API endpoint with those HTTP methods (eg GET, POST).\
A '*' wildcard value should not be configured here, since it will not work with credentialed requests.

#### oauth_proxy_cors_allow_headers

> **Syntax**: **`oauth_proxy_cors_allow_headers`** `string[]`
>
> **Default**: *['x-example-csrf']*
>
> **Context**: `location`

When CORS is enabled, the plugin returns these values in the [access-contol-allow-headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Headers) response header.\
Include here any additional [non-safelisted request headers](https://developer.mozilla.org/en-US/docs/Glossary/CORS-safelisted_request_header) that the SPA needs to send in API requests.\
To implement data changing requests, include the CSRF request header name, eg `x-example-csrf`.\
A '*' wildcard value should not be configured here, since it will not work with credentialed requests.

#### oauth_proxy_cors_expose_headers

> **Syntax**: **`oauth_proxy_cors_expose_headers`** `string[]`
>
> **Default**: *[]*
>
> **Context**: `location`

When CORS is enabled, the plugin returns these values in the [access-contol-expose-headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Expose-Headers) response header.\
Include here any additional [non-safelisted response headers](https://developer.mozilla.org/en-US/docs/Glossary/CORS-safelisted_response_header) that the SPA needs to read from API responses.\
A '*' wildcard value should not be configured here, since it will not work with credentialed requests.

#### oauth_proxy_cors_max_age

> **Syntax**: **`oauth_proxy_cors_max_age`** `number`
>
> **Default**: *86400*
>
> **Context**: `location`

When CORS is enabled, the plugin returns this value in the [access-contol-max-age](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Max-Age) response header.\
When a value is configured, this prevents excessive pre-flight OPTIONS requests to improve efficiency.

## Sample Configurations

#### Loading the Module

In deployed systems the module is loaded using the [load_module](http://nginx.org/en/docs/ngx_core_module.html#load_module) directive.\
This needs to be done in the _main_ part of the NGINX configuration:

```nginx
load_module modules/ngx_curity_http_oauth_proxy_module.so;
```

#### Basic Configuration

The following location decrypts cookies, then forwards an access token to the downstream API:

```nginx
location /products {

    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;

    proxy_pass "https://productsapi.example.com";
}
```

#### Inherited Configuration

Parent and child locations can be used, in which case children inherit the parent settings:

```nginx
location /api {

    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "example";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
    
    location /api/products {
        proxy_pass "https://productsapi.example.com";
    }

    location /api/offers {
        proxy_pass "https://offersapi.example.com";
    }
}
```

## Cookie Details

The plugin expects to receive up to two cookies, which use a custom prefix with fixed suffixes:

| Example Cookie Name | Fixed Suffix | Contains |
| ------------------- | ------------ | -------- |
| example-at | -at | An encrypted cookie containing either an opaque or JWT access token |
| example-csrf | -csrf | A CSRF cookie verified during data changing requests |

Cookies are encrypted using AES256-GCM, with a hex encoding, in this format:

| Cookie Section | Contains |
| -------------- | -------- |
| First 24 hex digits | This contains the 12 byte GCM initialization vector |
| Last 32 hex digits | This contains the 16 byte GCM message authentication code |
| Middle hex digits | This contains the ciphertext, whose length is that of the token being encrypted |

## Security Behavior

The module handles cookies according to [OWASP Cross Site Request Forgery Best Practices](https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html):

#### OPTIONS Requests

The plugin does not perform any logic for pre-flight requests from the SPA and returns immediately.

#### Web Origin Checks

For other methods, the plugin first reads the `Origin HTTP Header`, sent by all modern browsers.\
If this does not contain a trusted value the request is immediately rejected with a 401 response.

#### Cross Site Request Forgery Checks

The process is as follows, though the exact identifiers depend on the configured cookie prefix:

- After a user login the browser receives an `example-csrf` cookie from the main Token Handler API.
- When the SPA loads it receives a `csrf-token`, which stays the same for the authenticated session.
- This is sent as an `x-example-csrf` request header on POST, PUT, PATCH, DELETE commands.
- The cookie and header value must have the same value or the module returns a 401 error response.

#### Access Token Handling

Once other checks have completed, the module process the access token cookie.\
The `-at` cookie is decrypted, after which the token is forwarded to the downstream API:

```text
Authorization Bearer 42665300-efe8-419d-be52-07b53e208f46
```

With opaque reference tokens the encrypted cookies do not exceed NGINX default header sizes.\
If large JWTs are instead used, then these NGINX properties may need to use larger than default values:

- proxy_buffers
- proxy_buffer_size
- large_client_header_buffers

#### Decryption

AES256-GCM uses authenticated encryption, so invalid cookies are rejected with a 401 response:

- Cookies encrypted with a different encryption key
- Cookies where any part of the payload has been tampered with

#### Error Responses

The common failure scenarios are summarized below:

| Failure Type | Description | Error Status |
| ------------ | ----------- | ------------ |
| Invalid Request | Incorrect or malicious details were sent by the client | 401 |
| Incorrect Configuration | Invalid configuration leading to input being rejected | 401 |
| Encryption Key Renewal | Expected reconfiguration leading to input being rejected | 401 |
| Server Error | A technical problem occurs in the module logic | 500 |

For OAuth Proxy errors, the response contains a JSON body and CORS headers so that the SPA can read the details:

```text
{
    "code": "unauthorized_request", 
    "message": "Access denied due to missing or invalid credentials"
}

access-control-allow-origin: https://www.example.com
access-control-allow-credentials: true
```

#### SPA Error Handling

A 401 error response should be handled by the SPA as an access token expiry event.\
The SPA should try a token refresh, and ask the user to re-authenticate if this fails.\
This ensures that the SPA copes resiliently with both expiry and encryption key renewal.

## Compatibility

This module has been tested with these NGINX versions:

- NGINX 1.13.7 (NGINX Plus Release 14)
- NGINX 1.13.10 (NGINX Plus Release 15)

It is likely to work with newer versions of NGINX, but only the above have been verified.
 
### Releases

Pre-built binaries of this module are provided for the following versions of NGINX.\
Download the .so file for your platform and deploy it to the `/usr/lib/nginx/modules` folder of your NGINX servers.

|                                   | NGINX 1.19.5 / NGINX Plus R23 | NGINX 1.19.10 / NGINX Plus R24    | NGINX 1.21.3 / NGINX Plus R25 |
| ----------------------------------|:-----------------------------:|:---------------------------------:|:---------------------------------:|
| Amazon Linux                      | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/amzn.ngx_curity_http_oauth_proxy_module_1.19.5.so)           | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/amzn.ngx_curity_http_oauth_proxy_module_1.19.10.so)           | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/amzn.ngx_curity_http_oauth_proxy_module_1.21.3.so) | 
| Amazon Linux 2                    | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/amzn2.ngx_curity_http_oauth_proxy_module_1.19.5.so)          | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/amzn2.ngx_curity_http_oauth_proxy_module_1.19.10.so)          | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/amzn2.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| CentOS 7.0+                       | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/centos.7.ngx_curity_http_oauth_proxy_module_1.19.5.so)       | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/centos.7.ngx_curity_http_oauth_proxy_module_1.19.10.so)       | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/centos.7.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| CentOS 8.0+                       | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/centos.8.ngx_curity_http_oauth_proxy_module_1.19.5.so)       | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/centos.8.ngx_curity_http_oauth_proxy_module_1.19.10.so)       | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/centos.8.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| Debian 9.0 (Stretch)              | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/debian.stretch.ngx_curity_http_oauth_proxy_module_1.19.5.so) | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/debian.stretch.ngx_curity_http_oauth_proxy_module_1.19.10.so) | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/debian.stretch.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| Debian 10.0 (Buster)              | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/debian.buster.ngx_curity_http_oauth_proxy_module_1.19.5.so)  | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/debian.buster.ngx_curity_http_oauth_proxy_module_1.19.10.so)  | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/debian.buster.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| Alpine                            | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/alpine.ngx_curity_http_oauth_proxy_module_1.19.5.so)         | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/alpine.ngx_curity_http_oauth_proxy_module_1.19.10.so)         | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/alpine.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| Ubuntu 18.04 LTS (Bionic Beaver)  | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/ubuntu.18.04.ngx_curity_http_oauth_proxy_module_1.19.5.so)   | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/ubuntu.18.04.ngx_curity_http_oauth_proxy_module_1.19.10.so)   | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/ubuntu.18.04.ngx_curity_http_oauth_proxy_module_1.21.3.so) |
| Ubuntu 20.04 LTS (Focal Fossa)    | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.19.5.so)   | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.19.10.so)   | [⇓](https://github.com/curityio/nginx_phantom_token_module/releases/download/1.2.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so) |

## Implementation Details

If you wish to customize this module by building from source, see the following resources:

| Guide | Description |
| ----- | ----------- |
| [Development](resources/1-development.md) | How to build and work with the module on a development computer |
| [Testing](resources/2-testing.md) | How to run NGINX tests to verify the module's success and failure behavior |
| [Deployment](resources/3-deployment.md) | How to build and deploy the module to a Docker container |

## Status

This module is fit for production usage.

## Licensing

This software is copyright (C) 2022 Curity AB. It is open source software that is licensed under the [Apache 2 license](LICENSE). For commercial support of this module, please contact [Curity sales](mailto:sales@curity.io).

## More Information

Please visit [curity.io](https://curity.io/) for more information about the Curity Identity Server.

