FINALIZATION
------------
1. Small CORS change to complete and update tests

2. Complete NGINX deployment for all distributions on 1.21.3
   Collect ldd settings also
   Record valgrind results for all permutations

3. Get 1.19.5 and 1.19.10 building and produce a full set of local .so files

4. Do NGINX+ certification testing if possible.\
   https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus \

5. Post on OSS channel

ISSUES
------
1. I used some Apache code for base64url decoding and included it in the NOTICES file.\
   There is a similar notice in this Apple repo that also uses the Apache utils:\
   https://opensource.apple.com/source/apr/apr-39/apr-util/apr-util

2. Removing cookie headers is complicated and feels inadvisable, so I have omitted this in the C module:\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

3. The PCRE (Perl regex) dependency is used to build the shared module in Docker images.\
   The download URL has changed for this version so I have used the new URL:\
   https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz

4. CentOS 8 reached 'end of life' at end of 2021 so I have commented it out from the Dockerfile for now.\
   The recommendation seems to be to update to CentOS stream.\
   Return to this once I'm more familiar with the NGINX+ setup\.
   https://blog.centos.org/2022/01/centos-community-newsletter-january-2022/
   https://techglimpse.com/failed-metadata-repo-appstream-centos-8/
   https://www.linuxcapable.com/how-to-install-latest-nginx-mainline-on-centos-8-stream/

DYNAMICALLY LINKED SIZES
------------------------
193K 10 Feb 14:25 alpine.ngx_curity_http_oauth_proxy_module_1.21.3.so*
175K 10 Feb 14:21 amzn.ngx_curity_http_oauth_proxy_module_1.21.3.so*
230K 10 Feb 14:23 amzn2.ngx_curity_http_oauth_proxy_module_1.21.3.so*
174K 10 Feb 14:16 centos.7.ngx_curity_http_oauth_proxy_module_1.21.3.so*
251K 10 Feb 14:20 debian.buster.ngx_curity_http_oauth_proxy_module_1.21.3.so*
227K 10 Feb 14:18 debian.stretch.ngx_curity_http_oauth_proxy_module_1.21.3.so*
234K 10 Feb 14:13 ubuntu.18.04.ngx_curity_http_oauth_proxy_module_1.21.3.so*
253K 10 Feb 14:15 ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so*

STATICALLY LINKED SIZES
-----------------------