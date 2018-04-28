PRODUCT_NAME:=freelan
PRODUCT_VERSION:=$(shell cat VERSION | tr -d '\r\n')
PRODUCT_BIN_PREFIX?=/usr/local
PRODUCT_PREFIX?=/usr/local
USE_UPNP?=yes
USE_MONGOOSE?=no
MAKE_JOBS?=1

default: build

.PHONY: build install

build:
	FREELAN_NO_GIT=1 FREELAN_NO_GIT_VERSION=${PRODUCT_VERSION} scons -j${MAKE_JOBS} --mode=release apps prefix=${PRODUCT_PREFIX} bin_prefix=${PRODUCT_BIN_PREFIX} --upnp=${USE_UPNP} --mongoose=${USE_MONGOOSE}

install:
	# Install the files to $(DESTDIR) (defaults to /)
	FREELAN_NO_GIT=1 FREELAN_NO_GIT_VERSION=${PRODUCT_VERSION} DESTDIR=$(DESTDIR) scons -j${MAKE_JOBS} --mode=release install prefix=${PRODUCT_PREFIX} bin_prefix=${PRODUCT_BIN_PREFIX} --upnp=${USE_UPNP} --mongoose=${USE_MONGOOSE}

package:
	git archive HEAD --prefix=${PRODUCT_NAME}-${PRODUCT_VERSION}/ -o ${PRODUCT_NAME}-${PRODUCT_VERSION}.tar.gz

clean:
	FREELAN_NO_GIT=1 FREELAN_NO_GIT_VERSION=${PRODUCT_VERSION} scons -c --mode=release apps prefix=${PRODUCT_PREFIX} bin_prefix=${PRODUCT_BIN_PREFIX} --upnp=${USE_UPNP} --mongoose=${USE_MONGOOSE}
