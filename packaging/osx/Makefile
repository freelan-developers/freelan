PRODUCT_NAME=freelan
IDENTIFIER=org.freelan.freelan
VERSION=2.0
DAEMON_PACKAGE=${IDENTIFIER}.pkg
FINAL_PACKAGE=${PRODUCT_NAME}_${VERSION}.pkg

default: package

clean:
	rm -rf ${IDENTIFIER}
	rm -f ${DAEMON_PACKAGE}
	rm -f ${FINAL_PACKAGE}

package: clean
	mkdir -p ${IDENTIFIER}
	mkdir -p ${IDENTIFIER}/usr/local
	mkdir -p ${IDENTIFIER}/usr/local/share/freelan
	mkdir -p ${IDENTIFIER}/Library/LaunchDaemons
	cp -r ../../install/* ${IDENTIFIER}/usr/local/
	cp org.freelan.freelan.plist ${IDENTIFIER}/Library/LaunchDaemons/
	cp uninstall.sh ${IDENTIFIER}/usr/local/share/freelan/
	pkgbuild --root ${IDENTIFIER} --identifier ${IDENTIFIER} --version ${VERSION} --ownership recommended --scripts scripts ${DAEMON_PACKAGE}
	productbuild --distribution distribution.xml --resources resources --package-path . --version ${VERSION} ${FINAL_PACKAGE}
