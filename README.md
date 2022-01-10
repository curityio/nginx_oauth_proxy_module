# OAuth Proxy NGINX Module

[![Quality](https://img.shields.io/badge/quality-production-green)](https://curity.io/resources/code-examples/status/)
[![Availability](https://img.shields.io/badge/availability-binary-blue)](https://curity.io/resources/code-examples/status/)

An NGINX module that decrypts secure cookies in API calls from Single Page Applications.\
This is the OAuth Proxy component of the [Token Handler Pattern](https://curity.io/resources/learn/the-token-handler-pattern/).

![Token Handler Pattern](resources/token-handler-pattern.png)

## Security Overview

The OAuth proxy is a forwarder to deal with cookie specific logic and transation.\
A typical flow for an SPA calling an API would work like this:

![Security Handling](resources/security-handling.png)

- The SPA makes an API call with an AES256 encrypted HTTP Only cookie containing an opaque access token
- The OAuth Proxy module decrypts the cookie to get the opaque access token
- The opaque access token is then forwarded in the HTTP Authorization Header
- The [Phantom Token Module](https://github.com/curityio/nginx_phantom_token_module) then swaps the opaque token for a JWT access token
- The HTTP Authorization Header is then updated with the JWT access token when the API is called
- The API must then verify the JWT in the standard zero trust manner, on every request

## Configuration Directives

All the directives in this subsection are required when the module is enabled.

#### oauth_proxy

> **Syntax**: **`oauth_proxy`** `on` | `off`
>
> **Default**: *`off`*
>
> **Context**: `location`

The module can be configured but temporarily disabled for one or more locations.\
If set to `on` then all following configuration settings are strictly validated.

#### oauth_proxy_allow_tokens

> **Syntax**: **`oauth_proxy_allow_tokens`** `on` | `off`
>
> **Default**: *`off`*                                                                
>
> **Context**: `location`                                                   

When set to `on` this allows requests that already have an authorization header to be routed directly to the target API.\
This enables the same API routes to be shared between browser and mobile clients.\
If set to `off` then all locations for which the module is configured must contain secure cookies.

#### oauth_proxy_cookie_prefix

> **Syntax**: **`oauth_proxy_cookie_prefix`** _`string`_
>
> **Default**: *``*                                                                
>
> **Context**: `location`                                                   

This must be set as a cookie prefix, and common conventions are to use company and / or product names.\
The value of `example` used in this README can be replaced with a value such as `mycompany` or `mycompany-product`.\
The maximum allowed length of the prefix is 64 characters.

#### oauth_proxy_hex_encryption_key

> **Syntax**: **`oauth_proxy_hex_encryption_key`** _`string`_
>
> **Default**: *``*                                                                
>
> **Context**: `location`                                                   

This must be exactly 64 hexadecimal characters, representing the 32 bytes of the encryption key.
This enables the key to be used for ES256-GCM encryption.\
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

An array of at least one trusted web origins that SPA clients will execute in.\
This can be repeated for related web subdomains if required, as below, though most commonly there is a single web domain:

```nginx
location / {
   ...
   oauth_proxy_trusted_web_origins "https://webapp1.example.com";
   oauth_proxy_trusted_web_origins "https://webapp2.example.com";
}
```

## Sample Configurations

#### Loading the Module

In deployed systems the module must be loaded using the [load_module](http://nginx.org/en/docs/ngx_core_module.html#load_module) directive.\ This needs to be done in the _main_ part of the NGINX configuration:

```nginx
load_module modules/ngx_curity_http_oauth_proxy_module.so;
```

#### Independent Configuration

The following API locations then process a cookie and then forwards an access token to the downstream API.\
In this setup each location gets an independent configuration:

```nginx
location /products {

    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";

    proxy_pass "https://productsapi.example.com";
}
location /offers {

    oauth_proxy on;
    oauth_proxy_allow_tokens on;
    oauth_proxy_cookie_prefix "example";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";

    proxy_pass "https://offersapi.example.com";
}
```

#### Inherited Configuration

If preferred, parent and child locations can be used instead, in which case children inherit the parent settings:

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

The plugin expects to receive up to two cookies, which use known suffixes:

| Cookie Full Name | Fixed Suffix | Contains |
| ---------------- | ------------ | -------- |
| example-at | -at | An encrypted cookie containing either an opaque or JWT access token |
| example-csrf | -csrf |  A CSRF cookie expected during data changing requests, when extra checks are made |

AES256-GCM encryption is used, with a hex encoding, meaning that each cookie value consists of these parts:

| Cookie Section | Contains |
| -------------- | -------- |
| First 24 hex digits | This contains the 12 byte GCM initialization vector |
| Last 32 hex digits | This contains the 16 byte GCM message authentication code |
| Middle digits | This contains the ciphertext and its length matches that of the token being encrypted |

## Security Behavior

The module receives cookies according to [OWASP Cross Site Request Forgery Best Practices](https://cheatsheetseries.owasp.org/cheatsheets/Cross-Site_Request_Forgery_Prevention_Cheat_Sheet.html).

#### OPTIONS Requests

The plugin does not perform any logic for pre-flight requests from the SPA and returns immediately.

#### Web Origin Checks

For other methods, the plugin first reads the `Origin HTTP Header`, which is sent by all modern browsers.\
If this does not contain a trusted value the request is immediately rejected with a 401 response.\

#### Cross Site Request Forgery Checks

After user login the SPA receives an HTTP Only encrypted cookie called `example-csrf` from the main Token Handler API.\
Whenever the SPA then loads it also receives an `csrf-token` JSON value, which stays the same for the authenticated session.\
This must be sent as a request header called `x-example-csrf` on data changing commands (POST, PUT, PATCH, DELETE).\
Both values must be received by the NGINX module, which verifies that they have the same value, and returns 401 otherwise.\
The exact values of the cookie and header used will be determined by the cookie prefix configured.

#### Access Token Handling

Once other checks have completed, both GETs and data changing commands process the access token.\
The `-at` cookie is decrypted, after which the token is forwarded to the downstream API in the standard header:

```text
Authorization Bearer 42665300-efe8-419d-be52-07b53e208f46
```

It is recommended to use opaque reference tokens so that encrypted cookies do not exceed NGINX max header sizes.\
If large JWTs are used, then the token handler pattern still works but these NGINX properties may need extending:

- proxy_buffers
- proxy_buffer_size
- large_client_header_buffers

#### Decryption

AES256-GCM encryption ensures that if an invalid encryption key is used to send cookies, decryption fails.\
Similarly, if any part of the encrypted payload is tampered with, decryption will also fail.

#### Error Responses

The main failure scenarios are summarized below, along with the error response returned to the SPA client:

| Failure Type | Description | Error Status |
| ------------ | ----------- | ------------ |
| Invalid Request | Incorrect or malicious details were sent by the client | 401 |
| Incorrect Configuration | Configuration prevents decryption or other logic from working | 401 |
| Server Error | A technical problem occurs in the module logic | 500 |

For successful requests, CORS response headers should be set by either the target API or in the reverse proxy.\
For error responses the module adds these response headers so that the SPA can read the error details:

```text
Access-Control-Allow-Origin: https://www.example.com
Access-Control-Allow-Credentials: true
```

## Compatibility

This module has been tested with NGINX 1.13.7 (NGINX Plus Release 14) and NGINX 1.13.10 (NGINX Plus Release 15).\
It is likely to work with other, newish versions of NGINX, but only these have been tested, pre-built and verified.

### Releases

Pre-built binaries of this module are provided for the following versions of NGINX on the corresponding operating system distributions.\
To use the module, download the .so file and deploy it with your instance of NGINX:

TODO

## Implementation Details

If you wish to customize this module by building from source, see the following resources:

| Guide | Description |
| ----- | ----------- |
| [Development](resources/1-development.md) | How to build and work with the module on a macOS development computer |
| [Testing](resources/3-testing.md) | How to run NGINX tests to verify the module's behavior for success and failure cases |
| [Deployment](resources/3-deployment.md) | How to build and deploy the module to a Docker container |

## Status

This module is fit for production usage. 

## Licensing

This software is copyright (C) 2022 Curity AB. It is open source software that is licensed under the [Apache 2 license](LICENSE).\
For commercial support of this module, please contact [Curity sales](mailto:sales@curity.io).

## More Information

Please visit [curity.io](https://curity.io/) for more information about the Curity Identity Server.

