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

## Configuration Directives

All of the directives are required for locations where the module is enabled.\
NGINX will fail to load if the configuration for any locations fail validation:

#### oauth_proxy

> **Syntax**: **`oauth_proxy`** `on` | `off`
>
> **Default**: *`off`*
>
> **Context**: `location`

The module is disabled by default but can be enabled for paths you choose.

#### oauth_proxy_allow_tokens

> **Syntax**: **`oauth_proxy_allow_tokens`** `on` | `off`
>
> **Default**: *`off`*                                                                
>
> **Context**: `location`                                                   

When set to `on`, requests that already have an authorization header can bypass cookie validation.\
This enables the same API routes to be shared between SPAs and mobile clients.\
If set to `off` then all locations for which the module is configured must contain secure cookies.

#### oauth_proxy_cookie_prefix

> **Syntax**: **`oauth_proxy_cookie_prefix`** _`string`_
>
> **Default**: *``*                                                                
>
> **Context**: `location`                                                   

A cookie prefix name must be provided, such as a company and / or product name.\
The value of `example` used in this README can be replaced with your own custom value.\
The maximum allowed length of the prefix is 64 characters.

#### oauth_proxy_hex_encryption_key

> **Syntax**: **`oauth_proxy_hex_encryption_key`** _`string`_
>
> **Default**: *``*                                                                
>
> **Context**: `location`                                                   

This must be exactly 64 hexadecimal characters, representing the 32 bytes of the encryption key.
This provides an encryption key that is compliant with the AES256-GCM standard.\
A random encryption key in the correct format can be generated via the following command:

```bash
openssl rand 32 | xxd -p -c 64
```

#### oauth_proxy_trusted_web_origins

> **Syntax**: **`oauth_proxy_trusted_web_origins`** _`string`_
>
> **Default**: *``*                                                                
>
> **Context**: `location`                                                   

An array of at least one trusted web origins where SPA clients will run in the browser.\
Multiple subdomains can be configured, though a single value is the most common use case:

```nginx
location / {
   ...
   oauth_proxy_trusted_web_origins "https://webapp1.example.com";
   oauth_proxy_trusted_web_origins "https://webapp2.example.com";
}
```

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
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";

    proxy_pass "https://productsapi.example.com";
}
```

#### Inherited Configuration

Parent and child locations can be used, in which case children inherit the parent settings:

```nginx
location /api {

    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    
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
If large JWTs are used, then these NGINX properties may need to use larger than default values:

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

For error responses the module adds these CORS headers so that the SPA can read the response:

```text
Access-Control-Allow-Origin: https://www.example.com
Access-Control-Allow-Credentials: true
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
To use the module, download the .so file and deploy it to the modules folder of your NGINX system.\
This [docker-compose file](./resources/docker=compose.yaml) has some deployment examples.

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

