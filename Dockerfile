######
FROM ubuntu:20.04 as ubuntu20-builder

RUN apt-get update && \
    apt-get install -y build-essential wget

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY oauth_proxy.c /tmp
COPY oauth_proxy_decrypt.c /tmp
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN wget https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz && tar xzvf pcre-8.44.tar.gz
RUN wget https://www.zlib.net/zlib-1.2.11.tar.gz && tar xzvf zlib-1.2.11.tar.gz
RUN wget https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1m.tar.gz && tar xzvf OpenSSL_1_1_1m.tar.gz
RUN CONFIG_OPTS="--with-pcre=../pcre-8.44 --with-zlib=../zlib-1.2.11 --with-openssl=../openssl-OpenSSL_1_1_1m" ./configure && make
