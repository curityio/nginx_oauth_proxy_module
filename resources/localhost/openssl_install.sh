#!/bin/bash

######################################################################################################
# Install OpenSSL on a development computer, for the best developer experience and use of intellisense
# This also enables local programming against OpenSSL, such as in the encryption tests repo:
# - https://github.com/curityio/token-handler-encryption-tests
######################################################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../..

#
# Get the code
#
curl -O -L https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1m.tar.gz
if [ $? -ne 0 ]; then
  echo 'Problem encountered downloading OpenSSL source code'
  exit 1
fi

#
# Unzip it
#
rm -rf openssl-OpenSSL_1_1_1m
tar xzvf OpenSSL_1_1_1m.tar.gz
if [ $? -ne 0 ]; then
  echo 'Problem encountered unzipping OpenSSL archive'
  exit 1
fi

#
# Configure it
#
cd openssl-OpenSSL_1_1_1m
./config
if [ $? -ne 0 ]; then
  echo 'Problem encountered configuring OpenSSL'
  exit 1
fi

#
# Build it
#
make
if [ $? -ne 0 ]; then
  echo 'Problem encountered compiling OpenSSL'
  exit 1
fi

#
# Deploy header and libraries to standard places so that it is programmable against
#
sudo make install
if [ $? -ne 0 ]; then
  echo 'Problem encountered deploying OpenSSL'
  exit 1
fi