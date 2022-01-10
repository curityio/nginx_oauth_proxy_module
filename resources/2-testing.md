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

## 3. Understand Test Syntax

See the [OpenResty Testing Documents](https://openresty.gitbooks.io/programming-openresty/content/testing/preparing-tests.html) to understand syntax.\
Documentation is limited but the [Headers More Repo](https://github.com/openresty/headers-more-nginx-module/tree/master/t) has a good selection of tests to compare against.

## 4. Understand Test Behavior

Each test spins up an instance of NGINX under the `t/servroot` folder.\
You can look at `t/servroot/conf/nginx.conf` to see the deployed configuration for the test.


Tests that fail during startup can be expressed like this.\
In this case no HTTP requests are needed:

```text
--- config
location /t {
    oauth_proxy on;
    ...
}

--- must_die

--- error_log
Expected error log text
```

Tests that succeed send a request that routes through to an NGINX path that returns success.\
Note that for Test::Nginx uses the test path of 1984:

Tests that fail 


## 4. Troubleshoot Failed Tests

If one test out of many is failing, then edit the Makefile to run a single file instead of `*.t`:

```text
test: all
	PATH=$(NGINX_SRC_DIR)/objs:$$PATH prove -v -f t/http_get.t
```

Then add the `ONLY` directive to the test that is failing:

```text
--- config
location /t {
    ...
}

--- request
GET /t

--- ONLY
```

If required, add `ngx_log_error` statements to C code, then look at test logs at `t/servroot/logs/error.log`.

## 5. Testing Interoperability

The following GitHub repo ensures that encryption is compliant across technologies:

- [Token Handler Encryption Tests](https://github.com/curityio/token-handler-encryption-tests)