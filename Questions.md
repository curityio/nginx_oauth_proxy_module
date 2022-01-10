Answers to these questions will be carried forward to all token handler resources:

1. Should we use base64 as the wire format for encrypted data?
   Currently we use hex for encrypted payloads.

2. Should we return a JSON body in error responses, with an error code?
   Currently we use empty responses in the same manner as phantom token module.
   We have verified that it is sufficient for the SPA to read error status codes.

3. Do we need pcre and zlib dependencies in the Docker build?

4. Do we want to make the OpenSSL version configurable?
   For now I have followed the pcre and zlib deployment approach

5. Is my approach of using OpenSSL in the Dockerfile correct?
   I need to look at the files in each distro and run some of them

6. I updated the download location of the PCRE library from ftp.pcre.org.
   The previously referenced one no longer exists.

7. Shall I update to the latest NGINX release of 1.25?

8. How do I test all built versions, since some of them do not match the release versions?