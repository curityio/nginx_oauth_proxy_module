#
# For CentOS we need to install some dependencies and then install nginx
# http://nginx.org/en/linux_packages.html#CentOS
#

FROM centos:8

# https://techglimpse.com/failed-metadata-repo-appstream-centos-8
WORKDIR /etc/yum.repos.d
RUN sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-*
RUN sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*
RUN yum update -y

RUN yum install -y yum-utils
COPY centos7/nginx.repo /etc/yum.repos.d/nginx.repo
RUN yum-config-manager --enable nginx-mainline

# 'yum list available nginx --showduplicates' shows versions available
RUN yum install -y nginx-1:1.21.3-1.el8.ngx.x86_64 valgrind
