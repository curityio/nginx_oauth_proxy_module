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
COPY --from=alpine-builder /tmp/nginx-$NGINX_VERSION/objs/ngx_curity_http_oauth_proxy_module.so /build/alpine.ngx_curity_http_oauth_proxy_module_$NGINX_VERSION.so

ENTRYPOINT ["sleep"]
CMD ["300"]
