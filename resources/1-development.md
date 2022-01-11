# OAuth Proxy Module - Development

## Prerequisites

A C compiler must be installed that meets the ISO C Standard (C99), such as gcc.


## 1. Intellisense Setup

Development IDEs expect to find OpenSSL headers in system locations, in order for intellisense to work.\
To enable this you can build the OpenSSL code from source: 

```bash
./resources/openssl_dev_setup.sh
```

Standard system locations will then be updated, for an improved developer experience:

```text
/usr/local/include/openssl/*
/usr/local/lib/libcrypto*
/usr/local/lib/libssl*
/usr/local/bin/openssl
```

## 2. Configure

Then run the base configure script as follows, to download NGINX source and point it to the OpenSSL location:

```bash
CONFIG_OPTS='--with-openssl=../openssl-OpenSSL_1_1_1m' ./configure 
```

Select these options to enable Perl tests to run and to enable debugging of the C code in an IDE such as CLion:

```text
DYNAMIC_MODULE=n
NGINX_DEBUG=y
```

## 3. Make

Whenever the code in the module changes, run the make command to rebuild NGINX:

```bash
./make
```

## 4. Make Install

Pre-creating the nginx folder for development is recommended, to enable C debugging permissions:

```bash
sudo mkdir /usr/local/nginx
sudo chown myusername /usr/local/nginx
```

Then deploy a working nginx system to the above location with this command:

```bash
make install
```

## 5. Run NGINX Locally

Then deploy the development `nginx.conf` file and start NGINX locally:

```bash
cp ./resources/dev_nginx.conf /usr/local/nginx/conf/nginx.conf
/usr/local/nginx/sbin/nginx
```

This nginx.conf file is configured to disable the daemon, so that logs are easily viewable.

## 6. Debug 

This project can be debugged in CLion 2020.2 or newer by opening its folder.\
Once nginx is running, select  `Run / Attach to Process`, and select the `nginx worker process`.\
Then set breakpoints, after which you can step through code to look at cookie and token values:

![Debugger](resources/debugger.png)

You can use an alternative IDE of your choice if preferred, such as Visual Studio Code with the C/C++ Extension Pack.\
To perform printf debugging you can add `ngx_log_error` statements to the C code and then look at NGINX output. 

## 7. Act as an SPA Client

You can run curl requests against the nginx system in the same manner as the SPA will:

```bash
AT_COOKIE='093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389'
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$AT_COOKIE"
```

The development target API is an internet mockbin API that echoes headers and shows the forwarded token:

```json
{
  "method": "GET"
  "headers": {
    "host": "mockbin.org",
    "origin": "https://www.example.com",
    "cookie": "example-at=093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389",
    "authorization": "Bearer 42665300-efe8-419d-be52-07b53e208f46",
  }
}
```

If the module instead returns an error, response CORS headers enable the SPA to read the response details:

```text
Access-Control-Allow-Origin: https://www.example.com
Access-Control-Allow-Credentials: true
```

## 8. Memory Leak Prevention

TODO: valgrind