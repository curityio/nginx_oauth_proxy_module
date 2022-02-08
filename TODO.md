FINALIZATION
------------
1. Test the module with the real example SPA on a branch, once other reviews are passed.

2. NGINX+ deployment to verify for different flavors of Linux.\
   I would like to get the certificates and be able to follow the below guide.\
   Also run NGINX+ certification tests.
   https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus \

3. Do a final build and upload a release to the GitHub repo.

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
