FROM debian:bookworm

RUN apt update && apt install -y \
    wget build-essential git tree software-properties-common dirmngr apt-transport-https ufw libssl-dev libpcre2-dev zlib1g-dev

COPY configure /tmp
COPY config /tmp
COPY Makefile /tmp
COPY src/* /tmp/src/
ARG NGINX_VERSION
ENV NGINX_VERSION=$NGINX_VERSION
ADD nginx-$NGINX_VERSION.tar.gz /tmp/

WORKDIR /tmp
RUN ./configure && make