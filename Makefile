PRODUCT_NAME:=freelan
PRODUCT_VERSION:=$(shell cat VERSION | tr -d '\r\n')
PRODUCT_PREFIX=/usr

default: build

build:
	FREELAN_NO_GIT=1 FREELAN_NO_GIT_VERSION=${PRODUCT_VERSION} scons --mode=release apps prefix=${PRODUCT_PREFIX}

install:
	# Install the files to $(DESTDIR) (defaults to /)
	FREELAN_NO_GIT=1 FREELAN_NO_GIT_VERSION=${PRODUCT_VERSION} DESTDIR=$(DESTDIR) scons --mode=release install prefix=${PRODUCT_PREFIX}

package:
	git archive HEAD --prefix=${PRODUCT_NAME}-${PRODUCT_VERSION}/ | gzip > ${PRODUCT_NAME}-${PRODUCT_VERSION}.tar.gz
