# OAuth Proxy NGINX Module

[![Quality](https://img.shields.io/badge/quality-test-yellow)](https://curity.io/resources/code-examples/status/)
[![Availability](https://img.shields.io/badge/availability-binary-blue)](https://curity.io/resources/code-examples/status/)

An NGINX module that decrypts secure cookies in API calls from Single Page Applications.\
This is the OAuth Proxy component of the [Token Handler Pattern](https://curity.io/resources/learn/the-token-handler-pattern/).

![Token Handler Pattern](token-handler-pattern.png)

## High Level Usage

The OAuth proxy is a forwarder placed in front of your business APIs, to deal with cookie authorization.\
A typical flow for an SPA calling an API would work like this:

![Security Handling](security-handling.png)

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

A whitelist of at least one web origin from which the module will accept requests.\
Multiple origins could be used in special cases where cookies are shared across subdomains.

#### oauth_proxy_cors_enabled

> **Syntax**: **`oauth_proxy_cors_enabled`** `boolean`
>
> **Context**: `location`

When enabled, the OAuth proxy returns CORS response headers on behalf of the API.\
When an origin header is received that is in the trusted_web_origins whitelist, response headers are written.\
The [access-control-allow-origin](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Origin) header is returned, so that the SPA can call the API.\
The [access-control-allow-credentials](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Credentials) header is returned, so that the SPA can send secured cookies to the API.\
Default values are provided for other CORS headers that the SPA needs.

## Optional Configuration Directives

#### oauth_proxy_allow_tokens

> **Syntax**: **`oauth_proxy_allow_tokens`** `on` | `off`
>
> **Default**: *off*
>
> **Context**: `location`

If set to true, then requests that already have a bearer token are passed straight through to APIs.\
This can be useful when web and mobile clients share the same API routes.

#### oauth_proxy_cors_allow_methods

> **Syntax**: **`oauth_proxy_cors_allow_methods`** `string`
>
> **Default**: *'OPTIONS,GET,HEAD,POST,PUT,PATCH,DELETE'*
>
> **Context**: `location`

When CORS is enabled, these values are returned in the [access-control-allow-methods](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Methods) response header.\
A '*' wildcard value should not be configured here, since it will not work with credentialed requests.

#### oauth_proxy_cors_allow_headers

> **Syntax**: **`oauth_proxy_cors_allow_headers`** `string`
>
> **Default**: *''*
>
> **Context**: `location`

When CORS is enabled, the module returns these values in the [access-control-allow-headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Headers) response header.\
If no values are configured then at runtime any headers the SPA sends are allowed.\
This is managed by returning the contents of the [access-control-request-headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Request-Headers) field.\
If setting values explicitly, ensure that the token handler CSRF request header is included, eg `x-example-csrf`.\
A '*' wildcard value should not be configured here, since it will not work with credentialed requests.

#### oauth_proxy_cors_expose_headers

> **Syntax**: **`oauth_proxy_cors_expose_headers`** `string`
>
> **Default**: *''*
>
> **Context**: `location`

When CORS is enabled, the module returns these values in the [access-contol-expose-headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Expose-Headers) response header.\
A '*' wildcard value should not be configured here, since it will not work with credentialed requests.

#### oauth_proxy_cors_max_age

> **Syntax**: **`oauth_proxy_cors_max_age`** `number`
>
> **Default**: *86400*
>
> **Context**: `location`

When CORS is enabled, the module returns this value in the [access-contol-max-age](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Max-Age) response header.\
This option prevents excessive pre-flight OPTIONS requests, to improve the efficiency of API calls.

## Example Configurations

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
    
    location /api/products {
        proxy_pass "https://productsapi.example.com";
    }

    location /api/offers {
        proxy_pass "https://offersapi.example.com";
    }
}
```

## Cookie Details

The module expects to receives two cookies, which use a custom prefix with fixed suffixes.\
Cookies are encrypted using AES256-GCM, and received in a base64 URL encoded format.

| Example Cookie Name | Fixed Suffix | Contains |
| ------------------- | ------------ | -------- |
| example-at | -at | An encrypted cookie containing an opaque or JWT access token |
| example-csrf | -csrf | A CSRF cookie verified during data changing requests |

## Security Behavior

The module handles cookies according to [OWASP Cross Site Request Forgery Best Practices](https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html):

#### Options Requests

The module first handles pre-flight OPTIONS requests and writes CORS response headers:

```text
access-control-allow-origin: https://www.example.com
access-control-allow-credentials: true
access-control-allow-cors_allow_methods: OPTIONS,GET,HEAD,POST,PUT,PATCH,DELETE
access-control-allow-cors_allow_headers: x-example-csrf
access-control-max-age: 86400
vary: origin,access-control-request-headers
```

#### Web Origin Checks

On the main request the module first reads the `Origin HTTP Header`, sent by all modern browsers.\
If this does not contain a trusted value the request is immediately rejected with a 401 response.

#### Cross Site Request Forgery Checks

The process is as follows, though the exact identifiers depend on the configured cookie prefix:

- After a user login the browser receives an `example-csrf` cookie from the main Token Handler API.
- When the SPA loads it receives a `csrf-token`, which stays the same for the authenticated session.
- This is sent as an `x-example-csrf` request header on POST, PUT, PATCH, DELETE commands.
- The cookie and header value must have the same value or the module returns a 401 error response.

#### Access Token Handling

Once other checks have completed, the module processes the access token cookie.\
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

Error responses contain a JSON body and CORS headers so that the SPA can read the details:

```text
{
    "code": "unauthorized", 
    "message": "Access denied due to missing or invalid credentials"
}

access-control-allow-origin: https://www.example.com
access-control-allow-credentials: true
```

The code in the [Example SPA](https://github.com/curityio/spa-using-token-handler) shows how to handle error responses.\
The HTTP status code is usually sufficient, and the error code can inform the SPA of specific causes.

## Compatibility

This module has been tested for the Linux NGINX distributions from the [Deployment Resources](/resources/deployment).\
It requires the [NGINX HTTP SSL module](http://nginx.org/en/docs/http/ngx_http_ssl_module.html) to be enabled, so that OpenSSL libraries are available.
 
### Releases

Pre-built binaries of this module are provided for the following versions of NGINX.\
Download the .so file for your platform and deploy it to the `/usr/lib/nginx/modules` folder of your NGINX servers.


|                                    | NGINX 1.27.4 / NGINX Plus R34 | NGINX 1.27.2 / NGINX Plus R33 | NGINX 1.25.5 / NGINX Plus R32 | NGINX 1.25.3 / NGINX Plus R31 | NGINX 1.25.1 / NGINX Plus R30 |
| -----------------------------------|:-----------------------------:|:-----------------------------:|:-----------------------------:|:------------------------------:|:-----------------------------:|
| Alpine                             | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/alpine.ngx_curity_http_oauth_proxy_module_1.27.4.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/alpine.ngx_curity_http_oauth_proxy_module_1.27.2.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/alpine.ngx_curity_http_oauth_proxy_module_1.25.5.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/alpine.ngx_curity_http_oauth_proxy_module_1.25.3.so)          | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/alpine.ngx_curity_http_oauth_proxy_module_1.25.1.so)          |
| Debian 11.0 (Bullseye)             | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bullseye.ngx_curity_http_oauth_proxy_module_1.27.4.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bullseye.ngx_curity_http_oauth_proxy_module_1.27.2.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bullseye.ngx_curity_http_oauth_proxy_module_1.25.5.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bullseye.ngx_curity_http_oauth_proxy_module_1.25.3.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bullseye.ngx_curity_http_oauth_proxy_module_1.25.1.so) |
| Debian 12.0 (Bookworm)             | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bookworm.ngx_curity_http_oauth_proxy_module_1.27.4.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bookworm.ngx_curity_http_oauth_proxy_module_1.27.2.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bookworm.ngx_curity_http_oauth_proxy_module_1.25.5.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bookworm.ngx_curity_http_oauth_proxy_module_1.25.3.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/debian.bookworm.ngx_curity_http_oauth_proxy_module_1.25.1.so) |
| Ubuntu 20.04 LTS (Focal Fossa)     | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.27.4.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.27.2.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.25.5.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.25.3.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.25.1.so)    |
| Ubuntu 22.04 LTS (Jammy Jellyfish) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.22.04.ngx_curity_http_oauth_proxy_module_1.27.4.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.22.04.ngx_curity_http_oauth_proxy_module_1.27.2.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.22.04.ngx_curity_http_oauth_proxy_module_1.25.5.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.22.04.ngx_curity_http_oauth_proxy_module_1.25.3.so)    | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.22.04.ngx_curity_http_oauth_proxy_module_1.25.1.so)    |
| Ubuntu 24.04 LTS (Noble Numbat) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.24.04.ngx_curity_http_oauth_proxy_module_1.27.4.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.24.04.ngx_curity_http_oauth_proxy_module_1.27.2.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/ubuntu.24.04.ngx_curity_http_oauth_proxy_module_1.25.5.so) | X | X |
| Amazon Linux 2                     | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2.ngx_curity_http_oauth_proxy_module_1.27.4.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2.ngx_curity_http_oauth_proxy_module_1.27.2.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2.ngx_curity_http_oauth_proxy_module_1.25.5.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2.ngx_curity_http_oauth_proxy_module_1.25.3.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2.ngx_curity_http_oauth_proxy_module_1.25.1.so)           |
| Amazon Linux 2023                  | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2023.ngx_curity_http_oauth_proxy_module_1.27.4.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2023.ngx_curity_http_oauth_proxy_module_1.27.2.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2023.ngx_curity_http_oauth_proxy_module_1.25.5.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2023.ngx_curity_http_oauth_proxy_module_1.25.3.so)           | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/amzn2023.ngx_curity_http_oauth_proxy_module_1.25.1.so)           |
| CentOS Stream 9.0+                 | x | x | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/centos.stream.9.ngx_curity_http_oauth_proxy_module_1.25.5.so) | [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/centos.stream.9.ngx_curity_http_oauth_proxy_module_1.25.3.so)| [⇓](https://github.com/curityio/nginx_oauth_proxy_module/releases/download/1.6.0/centos.stream.9.ngx_curity_http_oauth_proxy_module_1.25.1.so)|

## Development Setup

If you wish to customize this module by building from source, see the [Development Wiki](https://github.com/curityio/nginx_oauth_proxy_module/wiki) for instructions.

## Licensing

This software is copyright (C) 2022 Curity AB. It is open source software that is licensed under the [Apache 2 license](LICENSE). For commercial support of this module, please contact [Curity sales](mailto:sales@curity.io).

## More Information

Please visit [curity.io](https://curity.io/) for more information about the Curity Identity Server.

