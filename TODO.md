NIKOS
-----
1. NGINX+ certification testing to discuss with Nikos.\
   I would also like to test each .so file I have built in Docker Compose.\
   This requires certs and an account setup though.

2. Discuss build system and process for releases.\
   Do any work that is needed here.

3. Ask Nikos if we thinks all of the same dependencies look fine for my plugin.\
   I have updated the pcre download URL since the one in the phantom token module no longer exists:
   - https://ftp.pcre.org/pub/pcre/pcre-8.44.tar.gz
   - https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz

TASKS
-----
1. Set compiler flags, including C99 and release O8 / O6, and aim to statically link with openssl.\
   If not possible then explain why, and also update docs.

2. Post questions on the security channel.\
   Cover both CSRF and encryption.

3. Update GitHub wiki with developer resources.\
   I may need to make my module temporarily public to enable the wiki.

4. valgrind for phantom token in a minor PR
   Fix any invalid location build issues also, or invalid locations
 
5. Do I need to add `--without` options to the README.\
   Get Travis's viewpoint on this.
