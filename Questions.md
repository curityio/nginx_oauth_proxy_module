REVIEW PROCESS
--------------
1. It is probably best to start with a call to discuss questions that any of us have

2. Reviewers should cast their eye over the NGINX code, tests, deployment and docs\
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
 
3. What are the most common customer deployment models?\
   I would expect most of them to download an image such as 1.21.5-alpine, then copy in the module .so file.\
   Do some of them build NGINX from source?\
   I'm assuming that NGINX Plus is the main option used, due to better admin options etc?
 
4. Right now I am using nginx 1.21.3 only, and 1.21.5 is the latest version.\
   How do we decide which specific versions to support?\
   My preference is just to start with the current latest for the initial release.\
   Are there official NGINX download links for each of the distros we support?

5. What NGINX prerequisite flags are there in total that I need to document?\
   The module adds this config option, required for SSL to work:
   - --with-http_ssl_module

   It uses this option when building the module, to point to openssl source code:
   - --with-openssl=

   The phantom token module mentions a number of `--without` options that are not compatible.\
   Should I mention all of the same options in this module, or are some of them not relevant?

6. Do we think there are any library dependency risks?\
   Eg we build against openssl-1.1.1m and a customer NGINX container uses openssl-1.1.1k?\
   Is there anything we do to deal with this for other dependencies?

7. Jenkins and NGINX online deployment\
   I guess we will configure official builds in Jenkins once other details have been dealt with?\
   Worth discussing any special requirements here?

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

5. Phantom token module enhancements.\
   Pre flight OPTIONS requests should be ignored, since they can never have an Authorization header.\
   Error responses due to expiry have an empty body and a code / message in the www-authenticate response header.\
   The SPA should be able to read these details.\
   Would you prefer to use CORS expose-headers capabilities, or also return a JSON body?\
   My thinking is that a JSON response could return the same details as the www-authenticate header.\
   https://stackoverflow.com/questions/33672689/javascript-jquery-can%C2%B4t-get-www-authenticate-response-header

6. Docs
   I have made the main README focused on using the module and understanding what it does.\
   I am assuming that most readers will just want to grab a dynamic module and plug it in.\
   Meanwhile the resources folder has implementation details, which most users won't care about.\
   There was a lot of head scratching to figure this out, so I've split it into a few markdown files.\
   Let me know if you are not happy with this format.
