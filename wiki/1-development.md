# OAuth Proxy Module - Development

## Prerequisites

A C compiler must be installed that meets the ISO C Standard (C99), such as gcc.\
The default compiler on macOS, or that installed by XCode, is usually fine.

## 1. Install an IDE

This project can be run in a basic manner with Visual Studio Code and the C/C++ Extension Pack.\
When developing we recommend a more specialist tool such as CLion 2020.2 or newer.

## 2. OpenSSL Setup

On Docker images we build the code as a dynamic module using libssl-dev.\
This does not work on macOS so instead build OpenSSL from source.\
Development IDEs can then find OpenSSL headers in the correct system locations: 

```bash
./resources/localhost/openssl_install.sh
```

Standard system locations will then be updated:

```text
/usr/local/include/openssl/*
/usr/local/lib/libcrypto*
/usr/local/lib/libssl*
/usr/local/bin/openssl
/usr/local/share/man
```

## 3. Configure

Then run the base configure script as follows, to download NGINX source and use these options on macOS:

```bash
CONFIG_OPTS='--with-http_ssl_module --with-openssl=../openssl-OpenSSL_1_1_1m' ./configure
```

Select these options to enable Perl tests to run and to enable debugging of the C code in CLion:

```text
DYNAMIC_MODULE=n
NGINX_DEBUG=y
```

## 4. Make

Whenever the code in the module changes, run this command to rebuild NGINX:

```bash
make
```

## 5. Compiler and Linker Settings

The above  `./configure` script calls the main NGINX configure script and provides build options.\
This can include custom parameters from [this nginx page](http://nginx.org/en/docs/configure.html) including these:

| Option | Description |
| ------ | ----------- |
| --with-cc-opt | Settings to add to the CFLAGS variable used by the linker |
| --with-ld-opt | Settings to add to LDFLAGS variable used by the linker |

Within the NGINX project, `./auto/make` is then called, to create the main `./objs/Makefile`.\
When `--with-http_ssl_module` is used, the makefile indicates static linking:

```text
-lpcre ../openssl-OpenSSL_1_1_1m/.openssl/lib/libssl.a ../openssl-OpenSSL_1_1_1m/.openssl/lib/libcrypto.a -lz
```

However, lower level [CFLAGS settings](https://wiki.gentoo.org/wiki/CFLAGS#-O) are dictated by the nginx system.\
Settings such as `-std=c99` or `-O2` do not seem to be honored.

## 6. Make Install

Pre-creating the nginx folder for development is recommended.\
This enables nginx to be run as your own user account, which works better later when debugging:

```bash
sudo mkdir /usr/local/nginx
sudo chown yourusername /usr/local/nginx
```

Whenever you want to update the local system after building code, this is run.\
It deploys an entire NGINX system under the `/usr/local/nginx` folder:

```bash
make install
```

## 7. Run NGINX Locally

Deploy the development configuration in the `nginx.conf` file and start NGINX locally:

```bash
cp ./resources/localhost/nginx.conf /usr/local/nginx/conf/nginx.conf
/usr/local/nginx/sbin/nginx
```

This nginx.conf file is configured to disable the daemon, so that logs are easily viewable.

## 8. Debug Code

To perform printf debugging you can add `ngx_log_error` statements to the C code and then look at NGINX output.\
Once nginx is running, select  `Run / Attach to Process`, and choose the `nginx worker process`.\
Then set breakpoints, after which you can step through code to check variable state carefully:

![Debugger](debugging.png)

## 9. Act as an SPA Client

You can run curl requests against the nginx system in the same manner as the SPA:

```bash
AT_COOKIE='AcYBf995tTBVsLtQLvOuLUZXHm2c-XqP8t7SKmhBiQtzy5CAw4h_RF6rXyg6kHrvhb8x4WaLQC6h3mw6a3O3Q9A'
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$AT_COOKIE"
```

The access token received by the target API is then echoed back to the caller.\
The `curl -i` option can be used to view CORS headers that are returned to single page apps:

```json
{
    "message": "API was called successfully with Bearer 42665300-efe8-419d-be52-07b53e208f46"
}
```
