# OAuth Proxy Module - macOS Development Setup

## 1. Configure

Run the base configure script, and accept all defaults, but set `DYNAMIC_MODULE=n`.\
This will download both NGINX and OpenSSL source code:

```bash
./configure
```

## 2. Make

Next run the make command to build the C code whenever it changes.\
This will build and deploy OpenSSL from source the first time it is run:

```bash
./make
```

The following locations will be updated with the latest OpenSSL 1.1, ready for use by the plugin:

```text
/usr/local/include/openssl/*
/usr/local/lib/libcrypto*
/usr/local/lib/libssl*
/usr/local/bin/openssl
```

## 3. Make Install

Next run this to deploy the built NGINX system locally, along with the OAuth proxy module:

```bash
sudo make install
```

The `/usr/local/nginx` location will then be updated with a full NGINX installation.

## 4. Run NGINX Locally

Deploy the development `nginx.conf` file then start NGINX locally:

```bash
sudo cp ./development/nginx.conf /usr/local/nginx/conf/
sudo /usr/local/nginx/sbin/nginx
```

The nginx.conf file disables the daemon and runs NGINX interactively, so that logs are easily viewable.

## 5. Act as a Client

Then run curl commands such as this, which runs the plugin, then routes the request to an internet API.\
The internet API echoes headers so that we can see the token extracted from the secure cookie:

```bash
ENCRYPTED_ACCESS_TOKEN=$(cat ./development/encrypted_access_token.txt)
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN"
```

## 6. Test Setup

Add the path to `/usr/local/nginx/sbin` to the system PATH in the .zprofile file.\
Then run the following command to add macOS support for NGINX testing:

```bash
cpan Test::Nginx
```

## 7. Run Tests

Then run this command from the root folder to execute all NGINX tests from the `t` folder:

```bash
make test
```
