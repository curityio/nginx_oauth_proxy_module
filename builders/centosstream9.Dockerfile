
FROM quay.io/centos/centos:stream9

RUN yum install -y \
     gcc pcre2-devel zlib-devel make openssl-devel

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make