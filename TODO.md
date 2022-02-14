FINALIZATION
------------
1. Get CentOS 8 working

2. Post on OSS channel on Monday
   Point to integrated with SPA configuration
   Say I'd like to build a release and make the repo public fairly soon

3. Do NGINX+ certification testing when possible.\
   https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus \

NOTES
-----
1. I used some Apache code for base64url decoding and included it in the NOTICES file.\
   There is a similar notice in this Apple repo that also uses the Apache utils:\
   https://opensource.apple.com/source/apr/apr-39/apr-util/apr-util

2. Removing request headers in modules is not supported or recommended, so I have not implemented this:\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

3. The PCRE (Perl regex) dependency is used to build the shared module in Docker images.\
   The ftp.pcre.org site is no longer available so we now need to use a sourceforge URL:\
   https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz

4. CentOS 8 reached 'end of life' at the end of 2021 so I have commented it out from the Dockerfile for now.\
   The recommendation seems to be to update to CentOS stream.\
   Return to this once I'm more familiar with the NGINX+ setup\.
   https://blog.centos.org/2022/01/centos-community-newsletter-january-2022/
   https://techglimpse.com/failed-metadata-repo-appstream-centos-8/
   https://www.linuxcapable.com/how-to-install-latest-nginx-mainline-on-centos-8-stream/
