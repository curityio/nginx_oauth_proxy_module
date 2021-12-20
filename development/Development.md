# OAuth Proxy macOS Development Setup

## 1. Configure

Run the base configure script, and accept all defaults.\
This will download both NGINX and OpenSSL source code:

```bash
./configure
```

## 2. Developer PC Updates

The following locations will be updated with the latest OpenSSL:

```text
/usr/local/include/openssl/*
/usr/local/lib/libcrypto*
/usr/local/lib/libssl*
/usr/local/bin/openssl
```

The following location will be updated with a built from source NGINX installation.\
The nginx.conf file and built module will then be deployed under this root folder:

```text
/usr/local/nginx
```

## 3. Build, Deploy and Test

Then, whenever you edit the plugin code, run this script to deploy NGINX from source:

```bash
./development/run.sh
```

Then run curl commands such as this, which routes to an internet API that echoes headers received:

```bash
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=xxx"
```
