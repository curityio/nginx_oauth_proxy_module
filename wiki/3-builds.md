# OAuth Proxy Module - Builds

## 1. Linux Builds

Run these commands to build the OAuth Proxy as a dynamic module for multiple flavors of Linux:

```bash
NGINX_VERSION=1.21.3  ./build.sh
NGINX_VERSION=1.19.10 ./build.sh
NGINX_VERSION=1.19.5  ./build.sh
```

Shared libraries with a `.so` extension are produced for each Linux distro, in your local `./build` folder:

```text
alpine.ngx_curity_http_oauth_proxy_module_1.21.3.so
debian.buster.ngx_curity_http_oauth_proxy_module_1.21.3.so
ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so
```

## 2. Compiler Settings

When the `./configure` script is called, the main NGINX system's `./configure` script is invoked.\
This can accept custom parameters from [this nginx page](http://nginx.org/en/docs/configure.html) including these:

| Option | Description |
| ------ | ----------- |
| --with-cc-opt | Settings to add to the CFLAGS variable used by the linker |
| --with-ld-opt | Settings to add to LDFLAGS variable used by the linker |

The NGINX configure script uses `automake` to produce the build file at `./nginx-1.21.3/objs/Makefile`.\
Some [CFLAGS settings](https://wiki.gentoo.org/wiki/CFLAGS#-O), such as `-std=c99`, are dictated by the nginx system.

## 3. OpenSSL Dynamic Linking

The Linux build finds OpenSSL headers by installing `libssl-dev`, then dynamcally links to OpenSSL libraries.\
Dynamic linking ensures that any OpenSSL security fixes can be resolved by updating the customer NGINX system.

```text
$(LINK) -o objs/ngx_curity_http_oauth_proxy_module.so \
	objs/addon/src/oauth_proxy_module.o \
	objs/addon/src/oauth_proxy_configuration.o \
	objs/addon/src/oauth_proxy_handler.o \
	objs/addon/src/oauth_proxy_decryption.o \
	objs/addon/src/oauth_proxy_encoding.o \
	objs/addon/src/oauth_proxy_utils.o \
	objs/ngx_curity_http_oauth_proxy_module_modules.o \
	-shared
```

## 4. Troubleshoot Linux Build Failures

A multi-stage Docker build is used, to output built `.so` files to an `nginx-module-builder` image.\
To troubleshoot failures, remote to the most recent Docker image in `docker image list`.\
Build commands can then be run manually if required, to understand the failure cause:

```bash
docker run -it a77962ad4c52
cd /tmp
make
```
