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

# I would like to exclude openssl libraries but doing so results in an 'undefined symbol: EVP_DecryptFinal_ex' error:
RUN yum install -y gcc pcre-devel zlib-devel make wget valgrind

# TODO: delete
# openssl-devel
# openssl makes the libssl.so and libcrypto.so libs available at /usr/lib64

WORKDIR /tmp/nginx
RUN wget http://nginx.org/download/nginx-1.21.3.tar.gz && tar xzvf nginx-1.21.3.tar.gz
WORKDIR /tmp/nginx/nginx-1.21.3
RUN ./configure && make && make install

WORKDIR /
RUN rm -rf /tmp/nginx
