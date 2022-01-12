REVIEW PROCESS
--------------
1. It is probably best to start with a call to discuss questions that any of us have

2. Reviewers should cast their eye over the NGINX code, tests, deployment and docs.\
   Add as many comments as you like to PRs or send me info, on what is missing

3. I will deal with all of the review feedback points and update a number of related PME resources

4. I will also be writing a whitepaper on SPA security, to cover the bigger picture\
   Reviewers might find this interesting as background on the overall web security goal

DEPLOYMENT
----------
1. I have treated OpenSSL as the same type of dependency as pcre and zlib.\
   Do we feel this is the right approach, or should the customer choose an OpenSSL version?\
   I think the right approach is for us to dictate it, and currently we use the latest OpenSSL 1.1.1m.\
   I guess OpenSSL 3.x is best avoided due to interoperability issues?\
   https://github.com/openssl/openssl/tags

2. I have used the same Dockerfile dependencies as the phantom token module.\
   Is this right and do we need pcre and zlib?\
   Everything builds OK but is some of it redundant?\
   I have added some extra options to the Dockerfile, eg these were added to centos7:
   - wget perl tar gzip

   Note that I have updated the pcre download URL since the one in the phantom token module no longer exists:
   - https://ftp.pcre.org/pub/pcre/pcre-8.44.tar.gz
   - https://sourceforge.net/projects/pcre/files/pcre/8.44/pcre-8.44.tar.gz
 
3. I have assumed that most customers use NGINX+ and simply download a dynamic module?\
   Open source NGINX only seems to be supported for Alpine.\
   Are all of our supported distros and versions dictated by NGINX+?
   https://docs.nginx.com/nginx/admin-guide/installing-nginx/installing-nginx-plus/

4. Do some customers build NGINX from source?
   The phantom token module mentions a number of `--without` options that are not compatible.\
   Are these likely to happen and what do I need to look out for?
   
   This module uses this config option, required for SSL to work, and is this likely to be missing in some cases?
   - --with-http_ssl_module

   I use this option when building the module, but do customers need to know?
   - --with-openssl=

5. Do we think there are any library dependency risks?\
   Eg we build against openssl-1.1.1m and a customer NGINX container uses openssl-1.1.1k?\
   Is there anything we do to deal with this for the phantom token module?

6. Certification and release process.
   Is this managed by Jenkins - I had a look but could not see anything?
   Do we ever run this from a development computer, and should I sign up for an account?

IMPLEMENTATION
--------------
1. Cookie encryption uses AES256-GCM to avoid disclosing token details.\
   Politically it is good to be seen to use good encryption, though it does not prevent cookie replay.\
   This is mainstream and easy to explain, as an up to date authenticated symmetric algorithm.\
   The below repo uses it in a few different technologies:\
   https://github.com/curityio/token-handler-encryption-tests

2. Right now all of the token handler work uses hex encoding for encrypted cookies.\
   I plan to update this to base64 (4 characters for every 3 bytes).\
   It is a more efficient wire format, and cookie size can be an issue.

3. I have left secure cookies in the call to the downstream API.\
   Do we think it is cleaner to remove them and only forward the token?

4. Should we return a JSON body in NGINX module error responses?\
   For LUA plugins we have returned a JSON error response with a code and message field.\
   Mostly we return a generic 401 message: 
   - code: unauthorized_request
   - message: The request contained a missing, invalid or expired credential

   I would prefer to do this since it gives us a better mechanism for informing the client.\
   Currently the demo SPA client just reads HTTP status codes though.

5. Phantom token module and SPAs.\
   Pre flight OPTIONS requests should be ignored, since they can never have an Authorization header.\
   Error responses due to expiry have an empty body and a code / message in the www-authenticate response header.\
   The SPA should be able to read these details.\
   Would you prefer to use CORS expose-headers capabilities, or also return a JSON body?\
   My thinking is that a JSON response could return the same details as the www-authenticate header.\
   https://stackoverflow.com/questions/33672689/javascript-jquery-can%C2%B4t-get-www-authenticate-response-header

6. Docs
   I have made the main README focused on using the module and understanding what it does.\
   Meanwhile the resources folder has implementation details, which most users won't care about.\
   There was a lot of head scratching to figure this out, so I've split it into a few markdown files.\
   Let me know if you are not happy with this format.
