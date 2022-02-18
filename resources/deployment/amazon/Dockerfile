#
# For Amazon Linux 1 the open source install is not supported:
# http://nginx.org/en/linux_packages.html#Amazon-Linux
#
# Amazon 1 yum installs require NGINX Plus, via this repo:
# https://cs.nginx.com/static/files/nginx-plus-amazon.repo
#
# Therefore we build a default NGINX from source as in this post:
# https://stackoverflow.com/questions/37082406/how-to-install-nginx-1-9-15-on-amazon-linux-disto
#

FROM amazonlinux:1

RUN yum install -y gcc pcre-devel zlib-devel make wget valgrind

# To build NGINX from source and use OpenSSL, we also need to point it to OpenSSL source
WORKDIR /tmp
RUN wget https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1m.tar.gz && tar xzvf OpenSSL_1_1_1m.tar.gz
RUN wget http://nginx.org/download/nginx-1.21.3.tar.gz                              && tar xzvf nginx-1.21.3.tar.gz
WORKDIR /tmp/nginx-1.21.3

# with-compat ensures that the dynamic module does not need to use the same compile time options as NGINX.
RUN ./configure --with-compat --with-http_ssl_module --with-openssl=../openssl-OpenSSL_1_1_1m && make && make install

WORKDIR /
RUN rm -rf /tmp/openssl*
RUN rm -rf /tmp/OpenSSL*
RUN rm -rf /tmp/nginx*
