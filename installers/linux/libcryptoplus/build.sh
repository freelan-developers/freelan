#!/bin/sh

PROJECT_NAME=libcryptoplus
PROJECT_VERSION=1.0
#GIT_TAG=${PROJECT_VERSION}
GIT_TAG=master
MAINTAINER_EMAIL=julien.kauffmann@freelan.org

### DO NOT CHANGE ANYTHING BELOW THIS LINE ! ###

# Extract the git archive
git archive --remote ../../../${PROJECT_NAME} --format tar --prefix ${PROJECT_NAME}-${PROJECT_VERSION}/ ${GIT_TAG} | gzip > ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz

# We extract the source
tar zxf ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz

# We rename it as a orig file
mv ${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz

# We run dh_make
cd ${PROJECT_NAME}-${PROJECT_VERSION}
dh_make -l -e ${MAINTAINER_EMAIL} -c gpl3
