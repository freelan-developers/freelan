PROJECT_NAME=libcryptoplus
PROJECT_VERSION=1.3
GIT_TAG=${PROJECT_VERSION}
MAINTAINER_EMAIL=julien.kauffmann@freelan.org
LICENCE=gpl3

### DO NOT CHANGE ANYTHING BELOW THIS LINE ! ###

default: create_package

${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz/:
	git archive --remote ../../../${PROJECT_NAME} --format tar --prefix ${PROJECT_NAME}-${PROJECT_VERSION}/ ${GIT_TAG} | gzip > ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz

${PROJECT_NAME}-${PROJECT_VERSION}/: ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz
	tar zxf ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz

${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz: ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz
	cp ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz

copy_debian: ${PROJECT_NAME}-${PROJECT_VERSION}/
	cp -r package/* ${PROJECT_NAME}-${PROJECT_VERSION}/

create_package: copy_debian ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz
	cd ${PROJECT_NAME}-${PROJECT_VERSION} && \
		dpkg-buildpackage

dh_make: ${PROJECT_NAME}-${PROJECT_VERSION}/ ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz
	cd ${PROJECT_NAME}-${PROJECT_VERSION} && \
		dh_make -l -e ${MAINTAINER_EMAIL} -c ${LICENCE} -p ${PROJECT_NAME}_${PROJECT_VERSION}
	cp -r debian ../package/

clean:
	rm -rf ${PROJECT_NAME}-${PROJECT_VERSION}/
	rm -f ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz
	rm -f ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz
