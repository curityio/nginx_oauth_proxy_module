#!/bin/bash

######################################################################################################
# Install OpenSSL on a development computer, for the best developer experience and use of intellisense
######################################################################################################

cd "$(dirname "${BASH_SOURCE[0]}")"
cd ../..
OPENSSL_VERSION='openssl-3.1.2'

#
# Get the code
#
curl -O -L https://github.com/openssl/openssl/releases/download/$OPENSSL_VERSION/$OPENSSL_VERSION.tar.gz
if [ $? -ne 0 ]; then
  >&2 echo 'Problem encountered downloading OpenSSL source code'
  exit 1
fi

#
# Unzip it
#
rm -rf $OPENSSL_VERSION 2>/dev/null
tar xzvf $OPENSSL_VERSION.tar.gz
if [ $? -ne 0 ]; then
  >&2 echo 'Problem encountered unzipping OpenSSL archive'
  exit 1
fi

#
# Configure it
#
cd $OPENSSL_VERSION
./config
if [ $? -ne 0 ]; then
  >&2 echo 'Problem encountered configuring OpenSSL'
  exit 1
fi

#
# Build it
#
make
if [ $? -ne 0 ]; then
  >&2 echo 'Problem encountered compiling OpenSSL'
  exit 1
fi

#
# Deploy header and libraries to standard places so that it is programmable against
#
sudo make install
if [ $? -ne 0 ]; then
  >&2 echo 'Problem encountered deploying OpenSSL'
  exit 1
fi
