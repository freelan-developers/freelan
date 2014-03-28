#!/bin/sh

if [ "$#" -ne 2 ]; then
	echo "Syntax: $0 <ca_path> <name>" >&2
	echo >&2
	echo "Generate a certificate for the intermediate CA at the specified path." >&2
	exit 1
fi

CA_PATH=`readlink -f $1`
NAME=$2

if [ ! -d "$CA_PATH" ]; then
	echo "Error: Directory \"$CA_PATH\" does not exists." >&2
	exit 2
fi

echo "Generating private key:"
openssl genrsa -des3 -out $NAME.key 4096

echo "Generating certificate request:"
openssl req -new -sha1 -key $NAME.key -out $NAME.csr

echo "Signing certificate with the CA:"
openssl ca -out $CA_PATH/crt/$NAME.crt -in $NAME.csr -config $CA_PATH/ca.cnf
cp $CA_PATH/crt/$NAME.crt $NAME.crt
