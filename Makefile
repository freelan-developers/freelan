PRODUCT_NAME:=freelan
PRODUCT_VERSION:=$(shell git describe)

default: install

install:
	# Install the files to ${DESTDIR} (defaults to /)
	scons install prefix=/

build:
	scons all samples

package:
	git archive HEAD --prefix=${PRODUCT_NAME}-${PRODUCT_VERSION}/ | gzip > ${PRODUCT_NAME}-${PRODUCT_VERSION}.tar.gz
