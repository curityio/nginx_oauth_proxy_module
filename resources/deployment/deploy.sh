#!/bin/bash

###################################################################################
# Build and deploy one of the supported Linux distributions with the shared module
###################################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"

export ENCRYPTION_KEY=$(openssl rand 32 | xxd -p -c 64)
echo -n $ENCRYPTION_KEY > encryption.key

#
# Validate input to ensure that we have a supported Linux distribution
#
DISTRO=$1
case $DISTRO in

  'ubuntu18')
    MODULE_FILE='ubuntu.18.04.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;
  
  'ubuntu20')
    MODULE_FILE='ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'centos7')
    MODULE_FILE='centos.7.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/etc/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'centos8')
    MODULE_FILE='centos.8.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/etc/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'debian9')
    MODULE_FILE='debian.stretch.ngx_curity_http_oauth_proxy_module_1.19.5.so'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'debian10')
    MODULE_FILE='debian.buster.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'amazon')
    MODULE_FILE='amzn.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/usr/local/nginx/modules'
    NGINX_PATH='/usr/local/nginx/sbin/nginx'
    CONF_PATH='/usr/local/nginx/conf/nginx.conf'
    ;;

  'amazon2')
    MODULE_FILE='amzn2.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/etc/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'alpine')
    MODULE_FILE='alpine.ngx_curity_http_oauth_proxy_module_1.21.3.so'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;
  
esac

if [ "$MODULE_FILE" == '' ]; then
  echo 'Please enter a supported Linux distribution as a command line argument'
  exit
fi

#
# Build the Docker image
#
echo 'Building the NGINX and valgrind Docker image ...'
docker build --no-cache -f "$DISTRO/Dockerfile" -t "nginx_$DISTRO":v1 .
if [ $? -ne 0 ]; then
  echo "Problem encountered building the NGINX $DISTRO docker image"
  exit 1
fi

#
# Supply a runtime 32 byte AES256 cookie encryption key
#
ENCRYPTION_KEY=$(openssl rand 32 | xxd -p -c 64)
echo -n $ENCRYPTION_KEY > encryption.key

#
# Update the runtime configuration file
#
NGINX_CONF_DATA=$(cat ./nginx.conf.template)
NGINX_CONF_DATA=$(sed "s/ENCRYPTION_KEY/$ENCRYPTION_KEY/g" <<< "$NGINX_CONF_DATA")
echo "$NGINX_CONF_DATA" > ./nginx.conf

#
# Deploy the Docker container for the distro
#
echo 'Deploying the NGINX and valgrind Docker image ...'
export DISTRO
export MODULE_FILE
export MODULE_FOLDER
export NGINX_PATH
export CONF_PATH
docker-compose up --force-recreate --remove-orphans
