######
FROM ubuntu:18.04 as ubuntu18-builder

RUN apt-get update && \
    apt-get install -y build-essential libxslt1-dev libssl-dev

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make

######
FROM ubuntu:20.04 as ubuntu20-builder

RUN apt-get update && \
    apt-get install -y build-essential wget libssl-dev

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN wget https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz && tar xzvf pcre-8.44.tar.gz
RUN wget https://www.zlib.net/zlib-1.2.11.tar.gz && tar xzvf zlib-1.2.11.tar.gz
RUN CONFIG_OPTS="--with-pcre=../pcre-8.44 --with-zlib=../zlib-1.2.11" ./configure && make

######
FROM centos:7 as centos7-builder

RUN yum install -y \
     gcc pcre-devel zlib-devel make openssl-devel

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make

######
FROM centos:8 as centos8-builder

RUN yum install -y \
     gcc pcre-devel zlib-devel make openssl-devel

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make

######
FROM debian:stretch as debian9-builder

RUN apt update && apt install -y \
    wget build-essential git tree software-properties-common dirmngr apt-transport-https ufw libssl-dev

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN wget https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz && tar xzvf pcre-8.44.tar.gz
RUN wget https://www.zlib.net/zlib-1.2.11.tar.gz && tar xzvf zlib-1.2.11.tar.gz
RUN CONFIG_OPTS="--with-pcre=../pcre-8.44 --with-zlib=../zlib-1.2.11" ./configure && make

######
FROM debian:buster as debian10-builder

RUN apt update && apt install -y \
    wget build-essential git tree software-properties-common dirmngr apt-transport-https ufw libssl-dev

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN wget https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz && tar xzvf pcre-8.44.tar.gz
RUN wget https://www.zlib.net/zlib-1.2.11.tar.gz && tar xzvf zlib-1.2.11.tar.gz
RUN CONFIG_OPTS="--with-pcre=../pcre-8.44 --with-zlib=../zlib-1.2.11" ./configure && make

######
FROM amazonlinux:1 as amzn-builder

RUN yum install -y \
 gcc pcre-devel zlib-devel make openssl-devel

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make

######
FROM amazonlinux:2 as amzn2-builder

RUN yum install -y \
 gcc pcre-devel zlib-devel make openssl-devel

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make

######
FROM alpine as alpine-builder

RUN apk add --no-cache --virtual .build-deps \
    gcc libc-dev make libressl-dev pcre-dev zlib-dev linux-headers libxslt-dev \
    gd-dev geoip-dev perl-dev libedit-dev mercurial bash alpine-sdk findutils bash

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make

######
FROM alpine

ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
COPY --from=ubuntu18-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/ubuntu.18.04.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=ubuntu20-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/ubuntu.20.04.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=centos7-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/centos.7.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=centos8-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/centos.8.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=debian9-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/debian.stretch.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=debian10-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/debian.buster.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=amzn-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/amzn.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=amzn2-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/amzn2.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so
COPY --from=alpine-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/alpine.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so

ENTRYPOINT ["sleep"]
CMD ["300"]
