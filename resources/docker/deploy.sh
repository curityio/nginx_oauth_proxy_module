#!/bin/bash

#########################################################
# Deploys NGINX, the shared module and then runs valgrind
#########################################################

cd "$(dirname "${BASH_SOURCE[0]}")"

#
# Do the Docker deployment of multiple NGINX versions
#
echo 'Running NGINX and the OAuth Proxy dynamic module to Docker via valgrind ...'
docker-compose up --force-recreate --remove-orphans
if [ $? -ne 0 ]; then
  echo '*** Problem encountered running Docker compose'
  exit 1
fi
