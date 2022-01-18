NIKOS
-----
1. NGINX+ testing to discuss with Nikos.\
   I would like to test each .so file I have built, but this requires certs and an account setup.

2. Discuss build system and process for releases.\
   Do any work that is needed here.

3. Ask Nikos if we thinks all of the same dependencies look fine for my plugin.\
   I have updated the pcre download URL since the one in the phantom token module no longer exists:
   - https://ftp.pcre.org/pub/pcre/pcre-8.44.tar.gz
   - https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz

TASKS
-----
1. Use an attributes file so that the module shows as C in GitHub

2. Add a JSON body to error responses and update tests

3. Set compiler flags, including C99 and release O8 / O6, and aim to statically link with openssl.\
   If not possible then explain why, and also update docs.

4. Post questions on the security channel.\
   Cover both CSRF and encryption.

5. Update GitHub wiki with developer resources.\
   I may need to make my module temporarily public to enable the wiki.

6. valgrind for phantom token in a minor PR
   Fix any invalid location build issues also, or invalid locations
 
7. Do I need to add `--without` options to the README.\
   Get Travis's viewpoint on this.
