# OAuth Proxy Module - Development

## 1. Intellisense Setup

To enable the development IDE to find OpenSSL headers I first build OpenSSL from source: 

```bash
./development/openssl_setup.sh
```

The following locations will then be updated, so that development tools can find headers:

```text
/usr/local/include/openssl/*
/usr/local/lib/libcrypto*
/usr/local/lib/libssl*
/usr/local/bin/openssl
```

## 2. Configure

Run the base configure script as follows, to download NGINX source and point it to the OpenSSL location:

```bash
CONFIG_OPTS='--with-openssl=../openssl-OpenSSL_1_1_1m' ./configure 
```

Select these options to enable tests to run and to enable debugging of the C code:

```text
DYNAMIC_MODULE=n
NGINX_DEBUG=y
```

## 3. Make

Next run the make command to build the NGINX C code whenever it changes:

```bash
./make
```

## 4. Make Install

Next run this to deploy the built NGINX system locally, along with the OAuth proxy module:

```bash
sudo make install
```

The `/usr/local/nginx` location will then be updated with a full NGINX installation.

## 5. Run NGINX Locally

Deploy the development `nginx.conf` file, then start NGINX locally:

```bash
sudo cp ./development/macos_nginx.conf /usr/local/nginx/conf/
sudo /usr/local/nginx/sbin/nginx
```

This nginx.conf file disables the daemon and runs NGINX interactively, so that logs are easily viewable.

## 6. Act as an SPA Client

During development, run curl requests to represent the SPA, which will be routed to a mockbin target API:

```bash
AT_COOKIE='093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389'
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$AT_COOKIE"
```

```bash
AT_COOKIE='093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389'
CSRF_HEADER='pQguFsD6hFjnyYjaeC5KyijcWS6AvkJHiUmY7dLUsuTKsLAITLiJHVqsCdQpaGYO'
CSRF_COOKIE='f61b300a79018b4b94f480086d63395148084af1f20c3e474623e60f34a181656b3a54725c1b4ddaeec9171f0398bde8c6c1e0e12d90bdb13397bf24678cd17a230a3df8e1771f9992e3bf2d6567ad920e1c25dc5e3e015679b5e673'
curl -X POST http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "x-example-csrf: $CSRF_HEADER" \
-H "cookie: example-at=$AT_COOKIE; example-csrf=$CSRF_COOKIE"
```

Failed requests return a JSON error payload and also CORS headers so that the SPA can read the response.\
Successful requests are routed through to a mockbin API that echoes headers, so that we can view the forwarded token:

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

## 7. Debugging

Download and install CLion, and you can sign up for a free trial license if required.
Once nginx is running, select  `Run / Attach to Process`, then set breakpoints.\
Then send curl requests to NGINX and step through the module's code:

SCREENSHOT

Alternatively you can use Visual Studio Code and install the C/C++ Extension Pack.\
Then use `ngx_log_error` calls to do simple print debugging. 

## 8. Check for Memory Leaks

TODO: valgrind