#!/bin/bash

./install/bin/freelan -f -d --server.enabled=yes --server.authentication_script=scripts/authenticate.sh $@
