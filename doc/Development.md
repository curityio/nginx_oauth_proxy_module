# OAuth Proxy MacOS Development Setup

First configure how NGINX will use the module, and start with the dynamic option:

```bash
./configure
```

Next build NGINX from source, and create the dynamic module at this location:
./nginx-1.21.3/objs/ngx_curity_http_oauth_proxy_module.so

```bash
make
```

Next deploy NGINX to /usr/local/nginx, with the dynamic module in the modules subfolder:

```bash
sudo make install
```

Next copy in the developer configuration, which runs without a background daemon:

```bash
sudo cp nginx.conf /usr/local/nginx/conf/
```

Next start nginx:

```bash
sudo /usr/local/nginx/sbin/nginx
```

Next run an API request via the curl tool, which routes through to an internet mockbin API:

```bash
curl http://localhost:8080/api -H "Authorization: Bearer xxx"
```
