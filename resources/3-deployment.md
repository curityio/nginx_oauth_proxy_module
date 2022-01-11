# OAuth Proxy Module - Deployment

## 1. Linux Builds

Run the following script to build the NGINX as a dynamic module for multiple flavors of Linux:

```bash
./build.sh
```

A shared libray with a `.so` extension is produced for each Linux distro, in the local `./build` folder:

```text
alpine.ngx_curity_http_oauth_proxy_module_1.21.3.so
debian.buster.ngx_curity_http_oauth_proxy_module_1.21.3.so
ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so
```

## 2. Troubleshoot Build Failures

A multi-stage Docker build is used, to output built .so files to an `nginx-module-builder` image.\
To troubleshoot failures, first reduce the Dockerfile to a single distro and comment out failing commands:

```dockerfile
WORKDIR /tmp
#RUN wget https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1m.tar.gz && tar xzvf OpenSSL_1_1_1m.tar.gz
#RUN CONFIG_OPTS="--with-openssl=../openssl-OpenSSL_1_1_1m" ./configure && make
```

Then remote to the latest Docker image in `docker image list` and run commnds manually, until resolved:

```bash
docker run -it a77962ad4c52
cd /tmp
wget https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1m.tar.gz
tar xzvf OpenSSL_1_1_1m.tar.gz
RUN CONFIG_OPTS="--with-openssl=../openssl-OpenSSL_1_1_1m" ./configure && make
```

## 3. Test NGINX Open Source

Run the following script to deploy all .so files and run test curl requests with cookies:

```bash
./resources/nginx-opensource/deploy_and_test.sh
```

## 4. Test NGINX Plus

TODO: this will run the official tests and will require some prerequisites:

```bash
./resources/nginx-opensource/deploy_and_test.sh
```