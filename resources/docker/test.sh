#!/bin/bash

####################################################################################
#Run some integration tests against the deployed NGINX system with the custom module
####################################################################################

API_URL='http://localhost:8081/api'
WEB_ORIGIN='https://www.example.com'
ACCESS_TOKEN='42665300-efe8-419d-be52-07b53e208f46'
CSRF_TOKEN='njowdfew098723rhjl'
RESPONSE_FILE=response.txt

#
# Ensure that we are in the folder containing this script
#
cd "$(dirname "${BASH_SOURCE[0]}")"
ENCRYPT_UTIL=$(pwd)/encrypt.js

#
# Get encrypted values for testing
#
ENCRYPTED_ACCESS_TOKEN=$(node $ENCRYPT_UTIL "$ACCESS_TOKEN")
ENCRYPTED_CSRF_TOKEN=$(node $ENCRYPT_UTIL "$CSRF_TOKEN")

#
# Get a header value from the HTTP response file
#
function getHeaderValue(){
  local _HEADER_NAME=$1
  local _HEADER_VALUE=$(cat $RESPONSE_FILE | grep -i "^$_HEADER_NAME" | sed -r "s/^$_HEADER_NAME: (.*)$/\1/i")
  local _HEADER_VALUE=${_HEADER_VALUE%$'\r'}
  echo $_HEADER_VALUE
}

#
# Verify that browser pre-flight OPTIONS requests from a malicious site are denied CORS access
#
echo '1. Testing OPTIONS request for an untrusted web origin ...'
HTTP_STATUS=$(curl -i -s -X OPTIONS "$API_URL" \
-H "origin: https://malicious-site.com" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '204' ]; then
  echo "*** OPTIONS request failed, status: $HTTP_STATUS"
  exit
fi

ORIGIN=$(getHeaderValue 'access-control-allow-origin')
if [ "$ORIGIN" != '' ]; then
  echo '*** The CORS access-control-allow-origin response header was granted incorrectly'
  exit
fi

CREDENTIALS=$(getHeaderValue 'access-control-allow-credentials')
if [ "$CREDENTIALS" != '' ]; then
  echo '*** The CORS access-control-allow-credentials response header was granted incorrectly'
  exit
fi
echo '1. OPTIONS request successfully denied access to an untrusted web origin'

#
# Verify that browser pre-flight requests from a valid origin succeed and return the correct headers
#
echo '2. Testing OPTIONS request for a valid web origin ...'
HTTP_STATUS=$(curl -i -s -X OPTIONS "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "access-control-request-headers: x-example-csrf" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '204' ]; then
  echo "*** OPTIONS request failed, status: $HTTP_STATUS"
  exit
fi

ORIGIN=$(getHeaderValue 'access-control-allow-origin')
if [ "$ORIGIN" != "$WEB_ORIGIN" ]; then
  echo '*** The CORS access-control-allow-origin response header was not set correctly'
  exit
fi

CREDENTIALS=$(getHeaderValue 'access-control-allow-credentials')
if [ "$CREDENTIALS" != 'true' ]; then
  echo '*** The CORS access-control-allow-credentials response header was not set correctly'
  exit
fi

VARY=$(getHeaderValue 'vary')
if [ "$VARY" != 'origin,access-control-request-headers' ]; then
  echo '*** The CORS vary response header was not set correctly'
  exit
fi

METHODS=$(getHeaderValue 'access-control-allow-methods')
if [ "$METHODS" != 'OPTIONS,HEAD,GET,POST,PUT,PATCH,DELETE' ]; then
  echo '*** The CORS access-control-allow-methods response header was not set correctly'
  exit
fi

HEADERS=$(getHeaderValue 'access-control-allow-headers')
if [ "$HEADERS" != 'x-example-csrf' ]; then
  echo '*** The CORS access-control-allow-headers response header was not set correctly'
  exit
fi

MAXAGE=$(getHeaderValue 'access-control-max-age')
if [ "$MAXAGE" != '86400' ]; then
  echo '*** The CORS access-control-max-age response header was not set correctly'
  exit
fi
echo '2. OPTIONS request returned all correct CORS headers for a valid web origin'

#
# Verify that the main browser request from a malicious site is denied CORS access
#
echo '3. Testing GET request for an untrusted web origin ...'
HTTP_STATUS=$(curl -i -s -X GET "$API_URL" \
-H "origin: https://malicious-site.com" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
 -o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo "*** GET request failed, status: $HTTP_STATUS"
  exit
fi
ORIGIN=$(getHeaderValue 'access-control-allow-origin')
if [ "$ORIGIN" != '' ]; then
  echo '*** The CORS access-control-allow-origin response header was granted incorrectly'
  exit
fi

CREDENTIALS=$(getHeaderValue 'access-control-allow-credentials')
if [ "$CREDENTIALS" != '' ]; then
  echo '*** The CORS access-control-allow-credentials response header was granted incorrectly'
  exit
fi
echo '3. GET request successfully denied access to an untrusted web origin'

#
# Verify that the main browser request from a valid origin succeeds and returns the correct headers
#
echo '4. Testing GET request for a valid web origin ...'
HTTP_STATUS=$(curl -i -s -X GET "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '200' ]; then
  echo "*** GET request failed, status: $HTTP_STATUS"
  exit
fi

ORIGIN=$(getHeaderValue 'access-control-allow-origin')
if [ "$ORIGIN" != "$WEB_ORIGIN" ]; then
  echo '*** The CORS access-control-allow-origin response header was not set correctly'
  exit
fi

CREDENTIALS=$(getHeaderValue 'access-control-allow-credentials')
if [ "$CREDENTIALS" != 'true' ]; then
  echo '*** The CORS access-control-allow-credentials response header was not set correctly'
  exit
fi

VARY=$(getHeaderValue 'vary')
if [ "$VARY" != 'origin' ]; then
  echo '*** The CORS vary response header was not set correctly'
  exit
fi
echo '4. GET request returned all correct CORS headers for a valid web origin'

#
# Verify that SPA clients can read error responses from the plugin, by sending no credential but the correct origin
#
echo '5. Testing CORS headers for error responses to the SPA ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo '*** Request with no credential did not result in the expected error'
  exit
fi
ORIGIN=$(getHeaderValue 'Access-Control-Allow-Origin')
if [ "$ORIGIN" != "$WEB_ORIGIN" ]; then
  echo '*** CORS headers do not allow the SPA to read the error response'
  exit
fi
echo '5. CORS error responses returned to the SPA have the correct CORS headers'

#
# Verify that access is denied for GET requests without a token or cookie
#
echo '6. Testing POST with no credential ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo '*** POST with no credential did not result in the expected error'
  exit
fi
echo '6. POST with no credential failed with the expected error'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that an access token sent from a mobile client is passed through to the API
#
echo '7. Testing POST from mobile client with an access token ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-H "Authorization: Bearer $ACCESS_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '200' ]; then
  echo "*** POST from mobile client failed, status: $HTTP_STATUS"
  exit
fi
echo '7. POST from mobile client was successfully routed to the API'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that a cookie sent on a GET request is correctly decrypted
#
echo '8. Testing GET with a valid encrypted cookie ...'
HTTP_STATUS=$(curl -i -s -X GET "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '200' ]; then
  echo "*** GET with a valid encrypted cookie failed, status: $HTTP_STATUS"
  exit
fi
echo '8. GET with a valid encrypted cookie was successfully routed to the API'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that data changing commands require a CSRF cookie
#
echo '9. Testing POST with missing CSRF cookie ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo '*** POST with a missing CSRF cookie did not result in the expected error'
  exit
fi
echo '9. POST with a missing CSRF cookie was successfully rejected'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that data changing commands require a CSRF header
#
echo '10. Testing POST with missing CSRF header ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
-H "cookie: example-csrf=$ENCRYPTED_CSRF_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo '*** POST with a missing CSRF header did not result in the expected error'
  exit
fi
echo '10. POST with a missing CSRF header was successfully rejected'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that double submit cookie checks work if the cookie and value do not match
#
echo '11. Testing POST with incorrect CSRF header ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
-H "cookie: example-csrf=$ENCRYPTED_CSRF_TOKEN" \
-H "x-example-csrf: x$CSRF_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo '*** POST with an incorrect CSRF header did not result in the expected error'
  exit
fi
echo '11. POST with an incorrect CSRF header was successfully rejected'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that double submit cookie checks succeed with the correct data
#
echo '12. Testing POST with correct CSRF cookie and header ...'
HTTP_STATUS=$(curl -i -s -X POST "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=$ENCRYPTED_ACCESS_TOKEN" \
-H "cookie: example-csrf=$ENCRYPTED_CSRF_TOKEN" \
-H "x-example-csrf: $CSRF_TOKEN" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '200' ]; then
  echo '*** POST with correct CSRF cookie and header did not succeed'
  exit
fi
echo '12. POST with correct CSRF cookie and header was successfully routed to the API'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq

#
# Verify that malformed cookies are correctly rejected
#
echo '13. Testing GET with malformed access token cookie ...'
HTTP_STATUS=$(curl -i -s -X GET "$API_URL" \
-H "origin: $WEB_ORIGIN" \
-H "cookie: example-at=" \
-o $RESPONSE_FILE -w '%{http_code}')
if [ "$HTTP_STATUS" != '401' ]; then
  echo '*** GET with malformed access token cookie did not result in the expected error'
  exit
fi
echo '13. GET with malformed access token cookie was successfully rejected'
JSON=$(tail -n 1 $RESPONSE_FILE)
echo $JSON | jq
