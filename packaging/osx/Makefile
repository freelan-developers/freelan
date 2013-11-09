PRODUCT_NAME=freelan
IDENTIFIER=org.freelan.freelan
VERSION=1.1
DAEMON_PACKAGE=${IDENTIFIER}.pkg
FINAL_PACKAGE=${PRODUCT_NAME}_${VERSION}.pkg

default: package

clean:
	rm -rf ${IDENTIFIER}
	rm -f ${DAEMON_PACKAGE}
	rm -f ${FINAL_PACKAGE}

package: clean
	mkdir -p ${IDENTIFIER}
	mkdir -p ${IDENTIFIER}/usr/local/sbin
	mkdir -p ${IDENTIFIER}/usr/local/etc/freelan
	mkdir -p ${IDENTIFIER}/usr/local/share/freelan
	mkdir -p ${IDENTIFIER}/Library/LaunchDaemons
	cp ../../install/bin/freelan ${IDENTIFIER}/usr/local/sbin/
	cp ../../freelan/config/freelan.cfg ${IDENTIFIER}/usr/local/etc/freelan/
	cp org.freelan.freelan.plist ${IDENTIFIER}/Library/LaunchDaemons/
	cp uninstall.sh ${IDENTIFIER}/usr/local/share/freelan/
	pkgbuild --root ${IDENTIFIER} --identifier ${IDENTIFIER} --version ${VERSION} --ownership recommended --scripts scripts ${DAEMON_PACKAGE}
	productbuild --distribution distribution.xml --resources resources --package-path . --version ${VERSION} ${FINAL_PACKAGE}
