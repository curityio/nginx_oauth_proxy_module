REVIEW DISCUSSION POINTS
------------------------
1. Do NGINX+ certification testing when possible, though licensing is tricky to manage.\
   I may be able to get a trial license or we may descope this for an initial release.

2. I used some Apache code for base64url decoding and included it in the NOTICES file.\
   There is a similar notice in this Apple repo that also uses the Apache utils:\
   https://opensource.apple.com/source/apr/apr-39/apr-util/apr-util

3. Removing request headers in modules is not supported or recommended, so I have not implemented this:\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

4. The PCRE (Perl regex) dependency is used to build the shared module in Docker images.\
   The ftp.pcre.org site is no longer available so we now need to use a sourceforge URL:\
   https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz

5. CentOS 8 reached 'end of life' at the end of 2021 but I can't find NGINX Plus info on this.\
   Perhaps the future direction is CentOS Stream (either version 8 or 9).\
   For now I've used the hack from here to ensure that the build works:
   https://techglimpse.com/failed-metadata-repo-appstream-centos-8/

6. Some work will be done on the phantom token module also.\
   This might include porting some work I have done here, if we consider it useful.