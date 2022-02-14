# OAuth Proxy Module - Deployment

## 1. Deploying the Module

Once a build has completed, basic verification of each Linux distro is done using open source NGINX.\
The following deployment script is used, which uses a custom Dockerfile to build each image:

```bash
./resources/deployment/deploy.sh ubuntu18
```

A series of custom Dockerfiles are used, in the [Deployment Resources](https://github.com/curityio/nginx_oauth_proxy_module/tree/main/resources/deployment).\
These follow the instructions from [NGINX Linux Packages](http://nginx.org/en/linux_packages.html) document.

## 2. HTTP Tests

Next we can run the following script to run some HTTP requests to call the module:

```bash
./resources/deployment/test.sh
```

This runs some requests that call the module and verify the expected behavior:

```bash
5. Testing CORS headers for error responses to the SPA ...
5. CORS error responses returned to the SPA have the correct CORS headers
6. Testing POST with no credential ...
6. POST with no credential failed with the expected error
{
  "code": "unauthorized",
  "message": "Access denied due to missing or invalid credentials"
}
7. Testing POST from mobile client with an access token ...
7. POST from mobile client was successfully routed to the API
parse error: Unfinished JSON term at EOF at line 2, column 0
8. Testing GET with a valid encrypted cookie ...
8. GET with a valid encrypted cookie was successfully routed to the API
```

## 3. Memory Leak Testing

The above Dockerfiles run nginx using the `valgrind` tool, to test for memory leaks.\
Healthy output should indicate no memory leaks for the deployed system.

```bash
--8-- REDIR: 0x611eb70 (libc.so.6:memmove) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611de80 (libc.so.6:strncpy) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611ee50 (libc.so.6:strcasecmp) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611d8d0 (libc.so.6:strcat) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611deb0 (libc.so.6:rindex) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x61205a0 (libc.so.6:rawmemchr) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611ece0 (libc.so.6:mempcpy) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611eb10 (libc.so.6:bcmp) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611de40 (libc.so.6:strncmp) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
--8-- REDIR: 0x611d940 (libc.so.6:strcmp) redirected to 0x4a2c6e0 (_vgnU_ifunc_wrapper)
```
