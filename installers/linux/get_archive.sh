#!/bin/bash

DIR=$(pwd)

if [ $# -gt 0 ]
then
    if [ -d "$1" ]
    then
        DIR=$(cd $1 && pwd)
    else
        echo "\"$1\" does not exist. Using current directory instead." >&2
    fi
fi

BASENAME=$(basename ${DIR})
PROJECT_NAME=$(echo ${BASENAME} | cut -f1 -d_)
PROJECT_VERSION=$(echo ${BASENAME} | cut -f2 -d_)
ROOT=$(git rev-parse --show-toplevel)

echo "Getting ${PROJECT_NAME} version ${PROJECT_VERSION}..."

git archive --remote ${ROOT}/${PROJECT_NAME} --format tar --prefix ${PROJECT_NAME}-${PROJECT_VERSION}/ ${PROJECT_VERSION} | gzip > ${DIR}/${PROJECT_NAME}_${PROJECT_VERSION}.orig.tar.gz
