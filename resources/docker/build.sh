#!/bin/bash

############################################################################
# Builds a custom NGINX Docker image with valgrind to check for memory leaks
############################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"

#
# Build the custom Docker image
#
docker build --no-cache -f Dockerfile -t nginx_valgrind:v1 .
if [ $? -ne 0 ]; then
  echo '*** Problem encountered building the custom Docker image'
  exit 1
fi