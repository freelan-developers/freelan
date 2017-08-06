#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
MODE=debug

export LD_LIBRARY_PATH="${SCRIPTPATH}/build/${MODE}/lib"
export DYLD_LIBRARY_PATH="${SCRIPTPATH}/build/${MODE}/lib"

${SCRIPTPATH}/build/${MODE}/bin/freelan $@
#valgrind --leak-check=full --show-reachable=yes --suppressions=${SCRIPTPATH}/.valgrind-suppressions ${SCRIPTPATH}/build/${MODE}/bin/freelan $@
#gdb --args ${SCRIPTPATH}/build/${MODE}/bin/freelan $@
#lldb -- ${SCRIPTPATH}/build/${MODE}/bin/freelan $@
