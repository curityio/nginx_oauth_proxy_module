NGINX+
------
1. See if I can get Docker images and understand state

DOC
---
1. Update GitHub wiki with developer resources.\
   I may need to make my module temporarily public to enable the wiki.

COMPILER
--------
1. Strictest compiler optimizations and warnings in configure.\
   Include -std=c99, O8 or O6, and strictest warnings, then make it fail.\
   They do not seem to stick due to use of automake by NGINX.\
   At least document the results.

2. Statically link with openssl.\
   Current sizes of .so files are 109-137KB - check after static linking.\
   If not possible then at least explain why.

CERTIFICATION
-------------
1. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.

TRICKY CODE
-----------
1. Outgoing CORS headers to set, if I get this working in LUA.\
   Then update README with new config settings.

2. Remove cookie related headers rather than passing them through.\
   This should really remove only our cookies and leave others in place.\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

FINALIZATION
------------
1. See if I need to add `--without` comments to the README.\
   Get Travis's viewpoint on this.

2. Create an initial release and upload .so files for three NGINX versions

3. Consider an encrypt utility for tests, since there are now no links to the interop repo