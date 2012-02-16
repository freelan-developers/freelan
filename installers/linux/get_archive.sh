#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Syntax is: $0 <project> <version>" >&2
    exit 2
fi

PROJECT_NAME=$1
PROJECT_VERSION=$2
ROOT=$(git rev-parse --show-toplevel)

echo "Getting ${PROJECT_NAME} version ${PROJECT_VERSION}..."

git archive --remote ${ROOT}/${PROJECT_NAME} --format tar --prefix ${PROJECT_NAME}-${PROJECT_VERSION}/ ${PROJECT_VERSION} | gzip > ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz
tar zxf ${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz
