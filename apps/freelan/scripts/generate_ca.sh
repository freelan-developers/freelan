#!/bin/sh

SCRIPT_PATH=`readlink -f $0 | xargs dirname`

if [ "$#" -ne 1 ]; then
	echo "Syntax: $0 <ca_path>" >&2
	echo >&2
	echo "Generate a certificate for the CA at the specified path." >&2
	exit 1
fi

CA_PATH=`readlink -f $1`

if [ ! -d "$CA_PATH" ]; then
	echo "Error: Directory \"$CA_PATH\" does not exists." >&2
	exit 2
fi

echo "Generating CA certificate and private key:"
openssl req -new -x509 -extensions v3_ca -keyout $CA_PATH/key/ca.key -out $CA_PATH/crt/ca.crt -config $CA_PATH/ca.cnf
