# OAuth Proxy Module - Unit Tests

## 1. Test Setup

Add the path to `/usr/local/nginx/sbin` to the system PATH in the .zprofile file.\
Then run the following perl command to add support for NGINX testing:

```bash
cpan Test::Nginx
```

## 2. Run Tests

Then run this command from the root folder to execute all NGINX tests from the `t` folder:

```bash
make test
```

## 3. Understand Test Syntax

See the [Preparing Tests Guide](https://openresty.gitbooks.io/programming-openresty/content/testing/preparing-tests.html) to understand syntax.