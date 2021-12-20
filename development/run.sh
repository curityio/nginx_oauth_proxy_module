#!/bin/bash

###########################################
# Productive redeploying during development
###########################################

cd "$(dirname "${BASH_SOURCE[0]}")"
set -e
cd ..

#
# Build the C code into a library at ./nginx-1.21.3/objs/ngx_curity_http_oauth_proxy_module.so
# This is currently around 51K and OpenSSL perhaps needs to be deployed separately
#
make

#
# Deploy a complete nginx system, to /usr/local/nginx, with the module in the modules subfolder
#
sudo make install

#
# Override the default NGINX configuration with a development version that runs without a background daemon
#
sudo cp ./development/nginx.conf /usr/local/nginx/conf/

#
# Run NGINX and view logs directly in the terminal
#
sudo /usr/local/nginx/sbin/nginx
exit

#
# NGINX will not currently start when OpenSSL code is included in the plugin, probably caused by a link error
# I need to understand whether to use options such as these:
# --with-ld-opt="-L/my/ssl/location"
# -lcrypto
#
nginx: [emerg] dlopen() "/usr/local/nginx/modules/ngx_curity_http_oauth_proxy_module.so" failed (dlopen(/usr/local/nginx/modules/ngx_curity_http_oauth_proxy_module.so, 10): Symbol not found: _RAND_bytes
  Referenced from: /usr/local/nginx/modules/ngx_curity_http_oauth_proxy_module.so
  Expected in: flat namespace
 in /usr/local/nginx/modules/ngx_curity_http_oauth_proxy_module.so) in /usr/local/nginx/conf/nginx.conf:5