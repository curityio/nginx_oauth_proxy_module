#
# For CentOS we need to install some dependencies and then install nginx
# CentOS 8 is end of life so we use the latest CentOS Stream instead
# http://nginx.org/en/linux_packages.html#CentOS
#

FROM quay.io/centos/centos:stream8

RUN yum install -y yum-utils
COPY centos7/nginx.repo /etc/yum.repos.d/nginx.repo
RUN yum-config-manager --enable nginx-mainline

# 'yum list available nginx --showduplicates' shows versions available
RUN yum install -y nginx-1:1.21.3-1.el8.ngx.x86_64 valgrind
