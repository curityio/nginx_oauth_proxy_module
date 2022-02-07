TASKS
-----
1. Separate C resources and wiki, to consolidate docs

2. Remove headers rather than passing them through, then add GET tests 9 and 10
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

3. Run valgrind with built + deployed images
   Run tests concurrently

4. Fix build error

5. Do CORS updates based on incoming request headers and change the vary header
   Do this for LUA and also for C
   Update READMEs
   Merge code

6. SPA refinements

CODE
----
1. Final review of all memory access code and aim for simplifications

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
