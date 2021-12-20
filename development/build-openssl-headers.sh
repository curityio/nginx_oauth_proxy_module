#!/bin/bash

###############################################################
# Make OpenSSL header files available on a development computer
###############################################################

cd "$(dirname "${BASH_SOURCE[0]}")"
set -e

#
# Make OpenSSL header files available for macOS, such as the following file, which is missing otherwise
# openssl-1.1.1m/include/openssl/opensslconf.h
#
cd ../openssl-1.1.1m
./Configure darwin64-x86_64-cc
make
