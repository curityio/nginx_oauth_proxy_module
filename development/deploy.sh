#!/bin/bash

#############################################################################################
# Uses Docker Compose to deploy the dynamic module .so file, then run a couple of basic tests
#############################################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"

#
# Do the Docker deployment
#
docker-compose up --force-recreate

#
# Wait for the API
#

#
# Call the API with a secure cookie
#
ENCRYPTED_ACCESS_TOKEN=$(cat ../development/encrypted_access_token.txt)
curl -X GET http://localhost:8080/api \
-H "origin: https://www.example.com" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN"

#
# Assert results
#