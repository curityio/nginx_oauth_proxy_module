#
# For Amazon Linux we need to install some dependencies and then install nginx
# http://nginx.org/en/linux_packages.html#Amazon-Linux
#

FROM amazonlinux:1

RUN yum install -y yum-utils

# Amazon 1 is based on CentOS 7 and priority 10 has been added to the nginx.repo file
# https://stackoverflow.com/questions/37082406/how-to-install-nginx-1-9-15-on-amazon-linux-disto
COPY amazon/nginx.repo /etc/yum.repos.d/nginx.repo

RUN yum-config-manager --enable nginx-mainline

# 'yum list available nginx --showduplicates' shows versions available
# But this command results in a cryptic error due to a missing libpcre.so.1 dependency
RUN yum install -y nginx-1:1.21.3-1.el7.ngx.x86_64 valgrind

# I think Amazon 1 can only work on NGINX Plus, via this repo:
# https://cs.nginx.com/static/files/nginx-plus-amazon.repo
