#!/bin/bash

for ITEM in "libcryptoplus_1.3"
do
    PROJECT_NAME=$(echo ${ITEM} | cut -f1 -d_)
    PROJECT_VERSION=$(echo ${ITEM} | cut -f2 -d_)

    ../../get_archive.sh ${PROJECT_NAME} ${PROJECT_VERSION}
done
