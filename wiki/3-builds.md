# OAuth Proxy Module - Builds

## 1. Linux Builds

Run the following script to build the OAuth Proxy as a dynamic module for multiple flavors of Linux:

```bash
./build.sh
```

A shared library with a `.so` extension is produced for each Linux distro, in your local `./build` folder:

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
Lower level [CFLAGS settings](https://wiki.gentoo.org/wiki/CFLAGS#-O) such as `-std=c99` or `-O2` are dictated by the nginx system.\
We therefore use NGINX recommended build defaults to build the NGINX module.

## 3. OpenSSL Dependency

The Linux build finds OpenSSL headers by installing `libssl-dev` for the Linux platform.\
We also statically link with OpenSSL to ensure no unexpected runtime issues.\
In order for the NGINX autoconf to statically link, we must supply the `--with-http_ssl_module` flag:

```text
-lpcre ../openssl-OpenSSL_1_1_1m/.openssl/lib/libssl.a ../openssl-OpenSSL_1_1_1m/.openssl/lib/libcrypto.a -lz
```

This is only needed at build time, and the actual NGINX system works with or without the HTTP SSL module.

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

This should show only system libraries, with no dependencies on customer versions of OpenSSL:

```text
linux-vdso.so.1 (0x00007ffddb5af000)
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f4e68b24000)
/lib64/ld-linux-x86-64.so.2 (0x00007f4e6911b000)
```