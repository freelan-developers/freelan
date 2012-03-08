# Packaging makefile
#
# To get all the packaging repositories, just type "make sync"

default:
	@echo "Please specify a target."

sync: freelan-buildtools.debian libcryptoplus.debian libasiotap.debian libfscp.debian libfreelan.debian freelan.debian

update: freelan-buildtools.debian libcryptoplus.debian libasiotap.debian libfscp.debian libfreelan.debian freelan.debian
	cd freelan-buildtools.debian && git pull --all
	cd libcryptoplus.debian && git pull --all
	cd libasiotap.debian && git pull --all
	cd libfscp.debian && git pull --all
	cd libfreelan.debian && git pull --all
	cd freelan.debian && git pull --all

freelan-buildtools.debian:
	git clone git@github.com:ereOn/freelan-buildtools.debian.git
	cd freelan-buildtools.debian && git checkout -b upstream origin/upstream && git checkout -b pristine-tar origin/pristine-tar && git checkout master

libcryptoplus.debian:
	git clone git@github.com:ereOn/libcryptoplus.debian.git
	cd libcryptoplus.debian && git checkout -b upstream origin/upstream && git checkout -b pristine-tar origin/pristine-tar && git checkout master

libasiotap.debian:
	git clone git@github.com:ereOn/libasiotap.debian.git
	cd libasiotap.debian && git checkout -b upstream origin/upstream && git checkout -b pristine-tar origin/pristine-tar && git checkout master

libfscp.debian:
	git clone git@github.com:ereOn/libfscp.debian.git
	cd libfscp.debian && git checkout -b upstream origin/upstream && git checkout -b pristine-tar origin/pristine-tar && git checkout master

libfreelan.debian:
	git clone git@github.com:ereOn/libfreelan.debian.git
	cd libfreelan.debian && git checkout -b upstream origin/upstream && git checkout -b pristine-tar origin/pristine-tar && git checkout master

freelan.debian:
	git clone git@github.com:ereOn/freelan.debian.git
	cd freelan.debian && git checkout -b upstream origin/upstream && git checkout -b pristine-tar origin/pristine-tar && git checkout master

clean:
	rm -f *.dsc
	rm -f *.deb
	rm -f *.build
	rm -f *.changes
	rm -f *.tar.gz
