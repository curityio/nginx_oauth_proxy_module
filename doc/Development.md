# OAuth Proxy Module Development

## Installation

To build this module, run the following commands:

```sh
./configure
make
make install
```

This will download the NGINX source code if it is not already local. If it is, the location may be provided when prompted. By default, version 1.21.3 will be downloaded; a different version can be fetched by setting `NGINX_VERSION` before running the `configure` script. Any [additional parameters](http://nginx.org/en/docs/configure.html) (e.g., `--prefix`) that NGINX's `configure` script supports can also be provided. When this module's `configure` script is run, it will pass along `--with-compat` to NGINX's script. It asks if a dynamic module should be created (thus passing along `--add-dynamic-module`) or if the module should be compiled into the NGINX binary (thus passing `--add-module`); by default, it created a dynamically-linked module. It will also ask if debug flags should be enabled; if so, `--with-debug` and certain GCC flags will be passed on to NGINX's `configure` script to make debugging easier. After the script is run, just execute `make && make install`. These too will delegate to NGINX's `Makefile`. After this, the module will be usable and can be configured as described above.
