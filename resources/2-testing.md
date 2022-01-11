# OAuth Proxy Module - Unit Tests

## 1. Test Setup

Add the path to `/usr/local/nginx/sbin` to the system PATH in the .zprofile file.\
Then run the following Perl command to install support for NGINX testing:

```bash
cpan Test::Nginx
```

## 2. Run Tests

Then run this command from the root folder to execute all NGINX tests from the `t` folder:

```bash
make test
```

## 3. Understand the Nginx Test Framework

See the [OpenResty Testing Documents](https://openresty.gitbooks.io/programming-openresty/content/testing/preparing-tests.html) to understand syntax.\
Documentation is limited but the [Headers More Repo](https://github.com/openresty/headers-more-nginx-module/tree/master/t) has a good selection of tests to compare against.

## 4. Understand Test Behavior

Each test spins up an instance of NGINX under the `t/servroot` folder which runs on the test port of 1984.\
Tests that are expected to succeed use proxy_pass to route to a target that runs after the module and simply returns:

```nginx
location /t {
    oauth_proxy on;
    oauth_proxy_allow_tokens off;
    oauth_proxy_cookie_prefix "mycompany-myproduct";
    oauth_proxy_hex_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    
    proxy_pass http://localhost:1984/target;
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}
```

## 4. Troubleshoot Failed Tests

If one test out of many is failing, then edit the Makefile to run a single file instead of `*.t`:

```text
test: all
	PATH=$(NGINX_SRC_DIR)/objs:$$PATH prove -v -f t/http_get.t
```

Then add the `ONLY` directive to limit test execution to the single test that is failing:

```text
--- config
location /t {
    ...
}

--- request
GET /t

--- ONLY
```

View the `t/servroot/conf/nginx.conf` file to see the deployed configuration for a test.\
If required, add `ngx_log_error` statements to C code, then look at test logs at `t/servroot/logs/error.log`.

## 5. Testing Interoperability

The following GitHub repo ensures that encryption is compliant across technologies:

- [Token Handler Encryption Tests](https://github.com/curityio/token-handler-encryption-tests)