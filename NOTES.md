FINALIZATION
------------
1. Test the module with the real example SPA on an spa-deployments branch

2. Document static v dynamic linking below, and sizes of the dynamic module in each case
   Collect ldd settings from running containers I have so far

3. Get 1.19.5 and 1.19.10 building

4. Complete NGINX deployment for all distributions

5. Do NGINX+ certification testing if possible.\
   https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus \

6. Catch up with Travis to discuss proposed release.
   
7. Test builds and deployment on 1.19.10 and 1.19.5

8. Record valgrind results for all permutations   

9. Do a final build for all 3 NGINX versions and upload a release to the GitHub repo.

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

CURRENT SIZES
-------------
-rwxr-xr-x  1 gary.archer  staff   193K 10 Feb 14:25 alpine.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   175K 10 Feb 14:21 amzn.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   230K 10 Feb 14:23 amzn2.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   174K 10 Feb 14:16 centos.7.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   251K 10 Feb 14:20 debian.buster.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   227K 10 Feb 14:18 debian.stretch.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   234K 10 Feb 14:13 ubuntu.18.04.ngx_curity_http_oauth_proxy_module_1.21.3.so*
-rwxr-xr-x  1 gary.archer  staff   253K 10 Feb 14:15 ubuntu.20.04.ngx_curity_http_oauth_proxy_module_1.21.3.so*