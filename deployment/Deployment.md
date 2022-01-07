# OAuth Proxy Module - Builds and Deployment

## 1. Linux Builds

Run the following script to build the NGINX as a dynamic module for multiple flavors of Linux:

```bash
./build.sh
```

Each build of the module produces a shared libray with an `.so` extension within the ./build folder.\
There are 10 output files in total, including this one:

- ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so

## 2. Understand Build Process

TODO

## 3. Deploy a Docker Container

Run the following script to test a local deployment using an official Docker image with the OAuth proxy module:

```bash
./deploy.sh
```
Test calling the API and verify that cookie decryption works and that you see the forwarded token in the output:

```bash
```bash
ENCRYPTED_ACCESS_TOKEN=$(cat ../development/encrypted_access_token.txt)
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN"
```
