# OAuth Proxy Module - Development and Testing

## 1. Build OpenSSL

First download and build OpenSSL for macOS as a one time operation:

```bash
./development/openssl_setup.sh
```

The following locations will be updated with the latest OpenSSL 1.1.1m, ready for use by the plugin:

```text
/usr/local/include/openssl/*
/usr/local/lib/libcrypto*
/usr/local/lib/libssl*
/usr/local/bin/openssl
```

## 2. Configure

Run the base configure script, and accept all defaults, except set `DYNAMIC_MODULE=n`, which is best for debugging:

```bash
CONFIG_OPTS='--with-openssl=../openssl-OpenSSL_1_1_1m' ./configure 
```

This will download NGINX source code and point it to the OpenSSL locations.

## 2. Make

Next run the make command to build the NGINX C code whenever it changes:

```bash
./make
```

## 3. Make Install

Next run this to deploy the built NGINX system locally, along with the OAuth proxy module:

```bash
sudo make install
```

The `/usr/local/nginx` location will then be updated with a full NGINX installation.

## 4. Run NGINX Locally

Deploy the development `nginx.conf` file, then start NGINX locally:

```bash
sudo cp ./development/nginx.conf /usr/local/nginx/conf/
sudo /usr/local/nginx/sbin/nginx
```

This nginx.conf file disables the daemon and runs NGINX interactively, so that logs are easily viewable.

## 5. Act as an SPA Client

During development, run curl requests to represent the SPA, which will be routed to a mockbin target API:

```bash
ENCRYPTED_ACCESS_TOKEN=$(cat ./development/encrypted_access_token.txt)
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN"
```

The request is routed through to the mockbin API which echoes headers so that we can view the token extracted from the cookie:

```json
"headers": {
    "host": "mockbin.org",
    "origin": "https://www.example.com",
    "cookie": "example-at=093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389",
    "authorization": "Bearer 42665300-efe8-419d-be52-07b53e208f46",
}
```

## 6. Test Setup

Add the path to `/usr/local/nginx/sbin` to the system PATH in the .zprofile file.\
Then run the following command to add support for NGINX testing:

```bash
cpan Test::Nginx
```

## 7. Run Tests

Then run this command from the root folder to execute all NGINX tests from the `t` folder:

```bash
make test
```
