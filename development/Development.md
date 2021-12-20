# OAuth Proxy macOS Development Setup

It is tricky to get all of NGINX, OpenSSL and the module code playing nicely on macOS.\
The below steps provide an end to end developer setup without breaking Linux based deployment.

## 1. Configure

Run the base configure script, which will download both NGINX and OpenSSL source code.\
The script currently gets, builds and deploys OpenSSL:

```bash
./configure
```

## 2. Understand Local OpenSSL

Running this should then use the up to date 1.1.1m version with best security features.\
Libraries and headers are updated accordingly.

```bash
openssl version
```

## 3. Build, Deploy and Test

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

## 4. Finalizations

Any macOS specific logic will need moving out of the configure script, once the system is working.
