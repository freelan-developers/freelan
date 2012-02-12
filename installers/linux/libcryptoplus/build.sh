#!/bin/sh

PROJECT_NAME=libcryptoplus
PROJECT_VERSION=master
GIT_TAG=${PROJECT_VERSION}

git archive --remote ../../../${PROJECT_NAME} --format tar --prefix ${PROJECT_NAME}/ ${GIT_TAG} | gzip > ${PROJECT_NAME}.tar.gz
