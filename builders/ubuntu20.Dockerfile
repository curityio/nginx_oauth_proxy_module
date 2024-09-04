FROM ubuntu:20.04

RUN apt-get update && \
    apt-get install -y build-essential libssl-dev libpcre2-dev zlib1g-dev

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make
