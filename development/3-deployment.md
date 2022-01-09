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

TODO - write up what is happening in the Docker containers

## 3. Deploy a Docker Container

Run the following script to test a local deployment using an official Docker image with the OAuth proxy module.\
This will wait for the reverse proxy to come up and then send a curl command with valid cookies.

```bash
./deploy.sh
```
