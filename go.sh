#!/bin/bash

set -e

make

sudo make install

sudo cp nginx.conf /usr/local/nginx/conf/

sudo /usr/local/nginx/sbin/nginx