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
