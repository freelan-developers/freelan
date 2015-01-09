#!/bin/bash

echo "Connection from $FREELAN_REMOTE_HOST:$FREELAN_REMOTE_PORT as '$FREELAN_USERNAME' with password '$FREELAN_PASSWORD'."

if [ "$FREELAN_USERNAME" == "" ]; then
	echo "No username was specified" >&2
	exit 1
fi

if [ "$FREELAN_PASSWORD" != "ok" ]; then
	echo "Invalid password" >&2
	exit 2
fi
