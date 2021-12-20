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
