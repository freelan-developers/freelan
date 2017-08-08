#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
MODE=debug

${SCRIPTPATH}/../build/${MODE}/bin/freelan -f -d --server.enabled=yes --server.authentication_script=${SCRIPTPATH}/scripts/authenticate.sh $@
