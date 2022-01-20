TASKS
-----
1. Update cookie encryption wire format and run valgrind
   Fix up all tests

2. Finalize compiler flags, eg in the configure script.\
   Include -std=c99, O8 or O6, and strictest warnings.\
   Either statically link with openssl or explain why not possible.\
   Current sizes of .so files are 109-137KB - check after static linking.\
   CFLAGS=-std=c99

3. Form a CSRF question and get my arguments lined up.\
   Then post it on the security channel.

4. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.\
   Then upload some initial releases for NGINX 1.21.3.\
   Talk to Travis about supported releases.

5. Update GitHub wiki with developer resources.\
   I may need to make my module temporarily public to enable the wiki.

6. See if I need to add `--without` comments to the README.\
   Get Travis's viewpoint on this.

7. Consider removing cookie related headers rather than passing them through.\
   This should really remove only our cookies and leave others in place.\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742