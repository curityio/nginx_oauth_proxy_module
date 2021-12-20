-include .build.info

# There is no `all` target in the NGINX Makefile, but it's a common default, so we add it. When this is used though,
# we always pass on `default` since `all` is unknown to the NGINX Makefile
default all: .build.info make_openssl install_openssl make_nginx

# Build OpenSSL before the NGINX make, so that headers are available to compile against
make_openssl:
	@echo "make_openssl"
	@if [ ! -f '/usr/local/bin/openssl' ]; then \
	  echo "NOT EXISTS" && cd $(OPENSSL_SRC_DIR) && $(MAKE) && cd ..; \
	fi

# Install OpenSSL before the NGINX make, so that libs are available to link with
install_openssl:
	@echo "install_openssl"
	@if [ ! -f '/usr/local/bin/openssl' ]; then \
	  echo "NOT EXISTS2" && cd $(OPENSSL_SRC_DIR) && $(MAKE) install_sw && cd ..; \
	fi

make_nginx:
	cd $(NGINX_SRC_DIR) && $(MAKE) -e default

module modules: .build.info $(NGINX_SRC_DIR)/Makefile
ifneq (, $(filter y yes Y YES Yes, $(DYNAMIC_MODULE)))
	cd $(NGINX_SRC_DIR) && $(MAKE) -f Makefile modules
else
	$(error Rerun the configure script and indicate that a dynamic module should be built)	
endif	

build install upgrade: .build.info $(NGINX_SRC_DIR)/Makefile
	cd $(NGINX_SRC_DIR) && $(MAKE) -e $@

clean:
	test -d "$(NGINX_SRC_DIR)" && $(MAKE) -C $(NGINX_SRC_DIR) $@ || true
	rm -rf .build.info nginx-$(NGINX_VERSION) nginx-$(NGINX_VERSION).tar.gz* openssl-${OPENSSL_VERSION} t/servroot

test: all
	docker-compose up -d
	@echo "Waiting for the Curity Identity Server to start..."
	@bash -c 'c=0; while [[ $$c -lt 25 && "$$(curl -fs -w ''%{http_code}'' localhost:8443)" != "404" ]]; do ((c++)); echo -n "."; sleep 1; done'
	PATH=$(NGINX_SRC_DIR)/objs:$$PATH prove -v -f t/
	docker-compose down

.build.info $(NGINX_SRC_DIR)/Makefile:
	$(error You need to run the configure script in the root of this directory before building the source)
