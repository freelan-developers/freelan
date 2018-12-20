#!/bin/bash

# Usage: start-client.sh [server] [username]

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
MODE=debug
SERVER=${1:-localhost}
USERNAME=${2:-client}

shift

${SCRIPTPATH}/../build/${MODE}/bin/freelan -f -d --client.enabled=yes --client.disable_peer_verification=yes --client.disable_host_verification=yes --fscp.listen_on=0.0.0.0:12001 --client.username=${USERNAME} --client.password="ok" --client.public_endpoint=0.0.0.0 --client.public_endpoint=:: --client.server_endpoint=${SERVER} $@
