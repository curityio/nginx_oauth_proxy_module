#!/bin/bash

###################################################################################
# Build and deploy one of the supported Linux distributions with the shared module
###################################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"

#
# Control deployment via environment variables
#
if [ "$DISTRO" == '' ]; then
  DISTRO='alpine'
fi
if [ "$NGINX_VERSION" == '' ]; then
  NGINX_VERSION='1.23.2'
fi
echo "Deploying for $DISTRO with NGINX version $NGINX_VERSION ..."

#
# Generate a cookie encryption key for the deployment
#
export ENCRYPTION_KEY=$(openssl rand 32 | xxd -p -c 64)
echo -n $ENCRYPTION_KEY > encryption.key

#
# Validate input to ensure that we have a supported Linux distribution
#
case $DISTRO in

  'ubuntu18')
    MODULE_FILE="ubuntu.18.04.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;
  
  'ubuntu20')
    MODULE_FILE="ubuntu.20.04.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'ubuntu22')
    MODULE_FILE="ubuntu.22.04.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'centos7')
    MODULE_FILE='centos.7.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so'
    MODULE_FOLDER='/etc/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'centosstream9')
    MODULE_FILE="centos.stream.9.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/etc/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'debian10')
    MODULE_FILE="debian.buster.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'debian11')
    MODULE_FILE="debian.bullseye.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'amazon2')
    MODULE_FILE="amzn2.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/etc/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;

  'alpine')
    MODULE_FILE="alpine.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so"
    MODULE_FOLDER='/usr/lib/nginx/modules'
    NGINX_PATH='/usr/sbin/nginx'
    CONF_PATH='/etc/nginx/nginx.conf'
    ;;
  
esac

#
# Check for a valid distro
#
if [ "$MODULE_FILE" == '' ]; then
  >&2 echo 'Please enter a supported Linux distribution as a command line argument'
  exit 1
fi

#
# Check that the image has been built
#
if [ ! -f "../../build/${MODULE_FILE}" ]; then
  >&2 echo "The OAuth Proxy plugin for $DISTRO version $NGINX_VERSION has not been built"
  exit 1
fi

#
# Build the Docker image
#
echo 'Building the NGINX and valgrind Docker image ...'
docker build --no-cache -f "$DISTRO/Dockerfile" --build-arg NGINX_VERSION="$NGINX_VERSION" -t "nginx_$DISTRO:$NGINX_VERSION" .
if [ $? -ne 0 ]; then
  >&2 echo "Problem encountered building the NGINX $DISTRO docker image"
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
export NGINX_VERSION
export MODULE_FILE
export MODULE_FOLDER
export NGINX_PATH
export CONF_PATH
docker-compose up --force-recreate --remove-orphans
