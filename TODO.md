BASE64URL
---------
1. Update decryption source file to use non nginx types.\
   Use a typedef for the logger.\
   typedef void (*logger)(void *, const char *, va_list);

2. Update encryption based on interop repo

3. Fix up all tests

4. Calculate encoded size correctly

5. Explain byte patterns

6. Merge the branch

COMPILER
--------
1. Strictest compiler optimizations and warnings in configure.\
   Include -std=c99, O8 or O6, and strictest warnings, then make it fail.\
   They do not seem to stick due to use of automake by NGINX.\
   At least document the results.

2. Statically link with openssl.\
   Current sizes of .so files are 109-137KB - check after static linking.\
   If not possible then at least explain why.

DOC
---
1. Update GitHub wiki with developer resources.\
   I may need to make my module temporarily public to enable the wiki.

2. See if I need to add `--without` comments to the README.\
   Get Travis's viewpoint on this.

HEADERS
-------
1. Remove cookie related headers rather than passing them through.\
   This should really remove only our cookies and leave others in place.\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

CERTIFICATION
-------------
1. Form and post a CSRF question on security channel

2. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.\
   Then upload some initial releases for NGINX 1.21.3.\
   Talk to Travis about supported releases.
