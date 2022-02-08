# OAuth Proxy Module - Unit Tests

## 1. Test Setup

Add the path to `/usr/local/nginx/sbin` to the system PATH in your .zprofile file.\
Then run the following Perl command to install support for NGINX testing:

```bash
cpan Test::Nginx
```

## 2. Run Tests

Then run this command from the root folder to execute all NGINX tests developed in the `t` folder:

```bash
make test
```

## 3. Understand the Nginx Test Framework

See the [OpenResty Testing Documents](https://openresty.gitbooks.io/programming-openresty/content/testing/preparing-tests.html) to understand syntax.\

## 4. Understand Test Behavior

Each test spins up an instance of NGINX under the `t/servroot` folder which runs on the default test port of 1984.\
Tests that are expected to succeed use proxy_pass to route to a target that runs after the module and simply returns.\
This example returns the decrypted access token as a target API response header, to support assertions.

```nginx
location /t {
    oauth_proxy on;
    oauth_proxy_cookie_name_prefix "mycompany-myproduct";
    oauth_proxy_encryption_key "4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50";
    oauth_proxy_trusted_web_origin "https://www.example.com";
    oauth_proxy_cors_enabled on;
    oauth_proxy_allow_tokens on;
    
    proxy_pass http://localhost:1984/target;
}
location /target {
    add_header 'authorization' $http_authorization;
    return 200;
}
```

## 5. Troubleshoot Failed Tests

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
If required, add `ngx_log_error` statements to C code, then look at test logs at `t/servroot/logs/error.log`.\
If you get cryptic permission errors or locked files, delete the `t/servroot` folder.

## 6. Run NGINX Plus Certification Tests

Before final release, our build system produces a dynamic module for multiple NGINX+ platforms, as described in [deployment](3-deployment.md). To certify that a build is compatible with NGINX+, each shared library needs to be tested with NGINX's certification test suite, then released to GitHub. For some background, refer to the [NGINX Plus Certified Modules Program documentation](https://www.nginx.com/partners/certified-module-program-documentation/#tech-doc-instructions-building). As described there:

- [NGINX+ must be installed](https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus/) on each platform in the build.sh script.
- The dynamic modules must be deployed to each supported platform. 
- The [certification test suite](https://www.nginx.com/partners/certified-module-program-documentation/#tech-doc-instructions-self-cert) and [its dependencies](https://www.nginx.com/partners/certified-module-program-documentation/#appendix) must be installed.
- With the NGINX+ service stopped, run the test suite, like this for example:

```sh
sudo -u nginx \
    TEST_NGINX_BINARY=/usr/sbin/nginx \
    TEST_NGINX_GLOBALS="load_module /tmp/modules/ngx_curity_http_oauth_proxy_module.so;" \
    TEST_NGINX_GLOBALS_HTTP="phantom_token off;" \
    prove -v . > ../nginx-plus-module-prove-test-verbose 2>&1
```