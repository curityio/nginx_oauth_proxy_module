TRICKY CODE
-----------
Do the following code in LUA first:

1. Set outgoing CORS headers to set, then update README with new config settings.

2. Remove cookie related headers rather than passing them through.\
   In C this is tricky, and we should not remove non Curity cookies:\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

DOC
---
1. Move developer resources to a Wiki file.

COMPILER
--------
1. Strictest compiler optimizations and warnings in configure.\
   Include -std=c99, O8 or O6, and strictest warnings, then make it fail.\
   They do not seem to stick due to use of automake by NGINX.\
   At least document the results.

2. Statically link with openssl.\
   Current sizes of .so files are 109-137KB - check after static linking.\
   If not possible then at least explain why.

NGINX+
------
1. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.

FINALIZATION
------------
1. See if I need to add `--without` comments to the README.\
   Get Travis's viewpoint on this.

2. Build the initial release and upload .so files for three NGINX versions

3. Consider an encrypt utility for tests, since there are now no links to the interop repo