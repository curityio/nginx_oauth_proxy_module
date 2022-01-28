TASKS
-----
1. CORS headers not quite outputting correctly.
   Short circuit execution when cors_enabled and return a 200 without visiting the API

2. Separate C resources and wiki, to consolidate docs

3. Update to a minimal REST API, without CORS headers

4. Update GET, CORS and CONFIG tests in line with LUA

5. Run valgrind and build all images
   Merge code

CODE
----
1. Remove cookie related headers rather than passing them through.\
   In C this is tricky, and we should not remove non Curity cookies:\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

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

2. Do an initial release for all NGINX versions

3. Discuss `--without` comments in  README with Travis
