# OAuth Proxy macOS Development Setup

It is tricky to get all of NGINX, OpenSSL and the module code playing nicely on macOS.\
The below steps provide an end to end developer setup without breaking Linux based deployment.

## 1. Configure

Run the base configure script, which will download both NGINX and OpenSSL source code.\
We are using openssl-1.1.1m, the latest mainstream release:

```bash
./configure
```

## 2. Build OpenSSL Headers

Both the NGINX build and developer intellisense will build OpenSSL from source so we need to first make headers available.\
This will enable us to build the plugin code during development:

```bash
./development/build-openssl-headers.sh
```

## 3. Prepare OpenSSL Linking

The above build does not produce lib files, yet the NGINX build wants to use this link option:

-L/usr/local/lib/openssl

This will result in a directory not found error during the OpenSSL make command.\
Therefore I installed openssl-1.1.1m for macOS locally:

```bash
brew install openssl
```

Then ran these commands, after which NGINX linking will work:

```bash
mkdir /usr/local/lib/openssl
ln -s /usr/local/opt/openssl/lib/libcrypto.dylib /usr/local/lib/openssl
ln -s /usr/local/opt/openssl/lib/libssl.dylib /usr/local/lib/openssl
```

For completeness I also updated my PATH in .zprofile:

- export PATH=/usr/local/opt/openssl@1.1/bin:${PATH}:...

Running this should then use the up to date 1.1.1m version with best security features:

```bash
openssl version
```

## 4. Build, Deploy and Test

Then, whenever you edit the plugin code, run this script to deploy NGINX from source:

```bash
./development/run.sh
```

Then, run curl commands such as this, which routes to an internet API that echoes headers received:

```bash
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=xxx"
```


