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
Lower level [CFLAGS settings](https://wiki.gentoo.org/wiki/CFLAGS#-O) such as `-std=c99` or `-O2` are dictated by the nginx system.

## 3. OpenSSL Static Linking

The Linux build finds OpenSSL headers by installing `libssl-dev`, then statically links to OpenSSL libraries.\
Static linking is done by specifying the `-lssl -lcrypto` options in the `./configure` script.\
On Linux the linked libraries are `libssl.so`, `libcrypto.so`, and on macOS they have a `.a` extension instead.

```text
$(LINK) -o objs/ngx_curity_http_oauth_proxy_module.so \
	objs/addon/src/oauth_proxy_module.o \
	objs/addon/src/oauth_proxy_configuration.o \
	objs/addon/src/oauth_proxy_handler.o \
	objs/addon/src/oauth_proxy_decryption.o \
	objs/addon/src/oauth_proxy_encoding.o \
	objs/addon/src/oauth_proxy_utils.o \
	objs/ngx_curity_http_oauth_proxy_module_modules.o \
	-lssl -lcrypto \
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

## 5. Troubleshoot Library Dependencies

In the event of a `library hell` issue we can later remote to a deployed container.\
Then use the `ldd` tool to list dynamic dependencies:

```bash
ldd /usr/lib/nginx/modules/ngx_curity_http_oauth_proxy_module.so 
```

This should show only system libraries, with no dependencies on customer versions of OpenSSL.\
The Linux deployment verifies that there are no library issues for any distro.

```text
linux-vdso.so.1 (0x00007ffddb5af000)
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f4e68b24000)
/lib64/ld-linux-x86-64.so.2 (0x00007f4e6911b000)
```