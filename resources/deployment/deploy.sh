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
    MODULE_PREFIX='ubuntu.18.04'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    ;;
  
  'ubuntu20')
    MODULE_PREFIX='ubuntu.20.04'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    ;;

  'centos7')
    MODULE_PREFIX='centos.7'
    MODULE_FOLDER='/etc/nginx/modules'
    ;;

  'alpine')
    MODULE_PREFIX='alpine'
    MODULE_FOLDER='/usr/lib/nginx/modules'
    ;;
  
esac

if [ "$MODULE_PREFIX" == '' ]; then
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
export MODULE_PREFIX
export MODULE_FOLDER
export ENCRYPTION_KEY
docker-compose up --force-recreate --remove-orphans
