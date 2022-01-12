#!/bin/bash

###############################################################################
# Run some commands against the deployed system and then check for memory leaks
###############################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"
PORT_NUMBER=8081
cd ../..

#
# Ensure that the NGINX instance is up
#
echo "Waiting for NGINX on port $PORT_NUMBER ..."
while [ "$(curl -s -o /dev/null -w ''%{http_code}'' http://localhost:$PORT_NUMBER)" != "200" ]; do
  sleep 2
done

#
# Run some HTTP requests
#
for i in {1..10}
do
  #
  # Make a GET request to the API via the OAuth proxy module with a secure cookie
  #
  echo 'Testing a GET request with a secure cookie ...'
  ENCRYPTED_ACCESS_TOKEN='093d3fb879767f6ec2b1e7e359040fe6ba875734ee043c5cc484d3da8963a351e9aba1c5e273f3d1ea2914f83836fa434474d1720b3040f5f7237f34536b7389'
  HTTP_STATUS=$(curl -s -X GET "http://localhost:$PORT_NUMBER/api" \
  -H "origin: https://www.example.com" \
  -H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
  -o /dev/null -w '%{http_code}')
  if [ "$HTTP_STATUS" != '200' ]; then
    echo "*** Problem encountered calling the API with a secure cookie and a GET request: $HTTP_STATUS"
  else
    echo "*** GET succeeded ..."
  fi

  #
  # Make a POST request to the API via the OAuth proxy module with a secure cookie
  #
  echo 'Testing a POST request with a secure cookie ...'
  CSRF_HEADER='pQguFsD6hFjnyYjaeC5KyijcWS6AvkJHiUmY7dLUsuTKsLAITLiJHVqsCdQpaGYO'
  ENCRYPTED_CSRF_TOKEN='f61b300a79018b4b94f480086d63395148084af1f20c3e474623e60f34a181656b3a54725c1b4ddaeec9171f0398bde8c6c1e0e12d90bdb13397bf24678cd17a230a3df8e1771f9992e3bf2d6567ad920e1c25dc5e3e015679b5e673'
  HTTP_STATUS=$(curl -s -X POST "http://localhost:$PORT_NUMBER/api" \
  -H "origin: https://www.example.com" \
  -H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN; example-csrf=$ENCRYPTED_CSRF_TOKEN" \
  -H "x-example-csrf: $CSRF_HEADER" \
  -o /dev/null -w '%{http_code}')
  if [ "$HTTP_STATUS" != '200' ]; then
    echo "*** Problem encountered calling the API with a secure cookie and a POST request: $HTTP_STATUS"
  else
    echo "*** POST succeeded ..."
  fi
done

#
# Inspect valgrind results once finished
#
echo 'Retrieving valgrind memory results ...'
DOCKER_CONTAINER_ID=$(docker container ls | grep nginx_valgrind | awk '{print $1}')
echo $DOCKER_CONTAINER_ID
docker cp "$DOCKER_CONTAINER_ID:/valgrind-results.txt" .
cat valgrind-results.txt