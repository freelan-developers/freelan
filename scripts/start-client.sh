#!/bin/bash

./install/bin/freelan2 -f -d --client.enabled=yes --client.disable_peer_verification=yes --client.disable_host_verification=yes --fscp.listen_on=0.0.0.0:12001 --client.username=$1 --client.password="ok"
