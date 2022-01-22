BASE64URL
---------
ENC KEY FOR TESTS
4e4636356d65563e4c73233847503e3b21436e6f7629724950526f4b5e2e4e50

AT_COOKIE
='AcYBf995tTBVsLtQLvOuLUZXHm2c-XqP8t7SKmhBiQtzy5CAw4h_RF6rXyg6kHrvhb8x4WaLQC6h3mw6a3O3Q9A'

CSRF token for tests:
AfctuC2zuBeZoQHfbopmpQyOADYU6Tp9raMEA-2EhWp4I3HtoiAtoP-H2U_PIrF7O0ZQ0nwE7VmWcl3BAY6bGlv4_EGqToyh4lOqynkSlBByxixJY-kA3bIFufJl

2. Get rid of // comments

2. Fix up all tests

3. Merge the branch

DOC
---
1. Update GitHub wiki with developer resources.\
   I may need to make my module temporarily public to enable the wiki.

2. See if I need to add `--without` comments to the README.\
   Get Travis's viewpoint on this.

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
1. Form and post a CSRF question on security channel

2. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.\
   Then upload some initial releases for NGINX 1.21.3.\
   Talk to Travis about supported releases.

HEADERS
-------
1. Remove cookie related headers rather than passing them through.\
   This should really remove only our cookies and leave others in place.\
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

