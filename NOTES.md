FINALIZATION
------------
1. Strictest compiler optimizations and warnings in configure.
   Include -std=c99, O8 or O6, and strictest warnings, then make it fail.
   They do not seem to stick due to use of automake by NGINX.
   At least document the results.

2. Statically link with openssl.
   Current sizes of .so files are 109-137KB - check after static linking.
   If not possible then at least explain why.

3. Test the module with the real example SPA on a branch, once other reviews are passed.

4. NGINX+ certification testing to do, maybe via a trial version.
   I would like to get the certificates and be able to follow the below guide.
   See if I can test each .so file I have built in Docker Compose.
   https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus/

5. CentOS 8 reached 'end of life' at end of 2021 so I have commented it out from the Dockerfile for now.
   The recommendation seems to be to update to CentOS stream:
   https://blog.centos.org/2022/01/centos-community-newsletter-january-2022/
   https://techglimpse.com/failed-metadata-repo-appstream-centos-8/
   https://www.linuxcapable.com/how-to-install-latest-nginx-mainline-on-centos-8-stream/

NOTES
-----
1. I used some Apache code for base64url decoding and included it in the NOTICES file.
   There is a similar notice in this Apple repo that also uses the Apache utils:
   https://opensource.apple.com/source/apr/apr-39/apr-util/apr-util

2. Removing cookie headers is complicated and feels inadvisable, so I have omitted this in the C module:
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742
