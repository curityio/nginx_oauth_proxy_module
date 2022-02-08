TASKS
-----
1. Test all OPTIONS permutations
   Try again via tests

2. Fix memory issue if possible:
memcheck GC: 1000 nodes, 994 survivors (99.4%)
--7-- memcheck GC: 1414 new table size (stepup)
--7-- memcheck GC: 1414 nodes, 1414 survivors (100.0%)
--7-- memcheck GC: 1999 new table size (stepup)
--7-- memcheck GC: 1999 nodes, 1999 survivors (100.0%)
--7-- memcheck GC: 2827 new table size (stepup)

FINALIZATION
------------
1. Final review of all memory access code and aim for simplifications
   Valgrind results indicate some GC that needs to be better understood
   Read up more on ngx_pcalloc etc

2. Strictest compiler optimizations and warnings in configure.\
   Include -std=c99, O8 or O6, and strictest warnings, then make it fail.\
   They do not seem to stick due to use of automake by NGINX.\
   At least document the results.

3. Statically link with openssl.\
   Current sizes of .so files are 109-137KB - check after static linking.\
   If not possible then at least explain why.

4. NGINX+ certification testing to do, maybe via a trial version.\
   See if I can test each .so file I have built in Docker Compose.

NOTES
-----
1. Removing cookie headers is complicated and feels inadvisable, so I have omitted this in the C module:
   https://www.ruby-forum.com/t/removing-a-request-header-in-an-access-phase-handler/245742

2. CentOS 8 is no longer supported so I have commented it out for now.
   Return to this as part of NGINX certification testing:
   https://techglimpse.com/failed-metadata-repo-appstream-centos-8/