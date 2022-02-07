TASKS
-----
1. Remove headers rather than passing them through, then add GET tests 9 and 10
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

2. Fix build error
   Merge code

3. Do CORS updates based on incoming request headers and change the vary header
   Do this for LUA and also for C

4. SPA refinements to cope with changed encryption keys or redeployments

FINALIZATION
------------
1. Final review of all memory access code and aim for simplifications
   Valgrind results indicate some GC that needs to be better understood

2. Strictest compiler optimizations and warnings in configure.\
   Include -std=c99, O8 or O6, and strictest warnings, then make it fail.\
   They do not seem to stick due to use of automake by NGINX.\
   At least document the results.

3. Statically link with openssl.\
   Current sizes of .so files are 109-137KB - check after static linking.\
   If not possible then at least explain why.

4. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.

5. Do an initial release for all NGINX versions
