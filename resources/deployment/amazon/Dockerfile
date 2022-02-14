#
# For Amazon Linux 1 the open source install is not supported:
# http://nginx.org/en/linux_packages.html#Amazon-Linux
#
# I think Amazon 1 yum installs can only work on NGINX Plus, via this repo:
# https://cs.nginx.com/static/files/nginx-plus-amazon.repo
#
# Therefore we build a default NGINX from source as in this post:
# https://stackoverflow.com/questions/37082406/how-to-install-nginx-1-9-15-on-amazon-linux-disto
#

FROM amazonlinux:1

RUN yum install -y gcc pcre-devel zlib-devel make wget valgrind

WORKDIR /tmp/nginx
RUN wget http://nginx.org/download/nginx-1.21.3.tar.gz && tar xzvf nginx-1.21.3.tar.gz
WORKDIR /tmp/nginx/nginx-1.21.3

# If customers ever build nginx from source they must use the --with-compat flag.
# When not set, dynamic modules must be built with the full compile time options that were used for the nginx binary.
# This results in a 'module is not binary compatible' error.
RUN ./configure --with-compat && make && make install

WORKDIR /
RUN rm -rf /tmp/nginx
