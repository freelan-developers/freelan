#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
MODE=debug
USERNAME=${1:-client}

shift

${SCRIPTPATH}/../build/${MODE}/bin/freelan -f -d --client.enabled=yes --client.disable_peer_verification=yes --client.disable_host_verification=yes --fscp.listen_on=0.0.0.0:12001 --client.username=${USERNAME} --client.password="ok" --client.public_endpoint=0.0.0.0 --client.public_endpoint=:: --client.public_endpoint=localhost:12001 $@
