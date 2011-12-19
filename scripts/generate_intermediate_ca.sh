#!/bin/sh

if [ "$#" -ne 3 ]; then
	echo "Syntax: $0 <ca_path> <root_ca_path> <name>" >&2
	echo >&2
	echo "Generate a certificate for the intermediate CA at the specified path." >&2
	exit 1
fi

CA_PATH=`readlink -f $1`
ROOT_CA_PATH=`readlink -f $2`
NAME=$3

if [ ! -d "$CA_PATH" ]; then
	echo "Error: Directory \"$CA_PATH\" does not exists." >&2
	exit 2
fi

if [ ! -d "$ROOT_CA_PATH" ]; then
	echo "Error: Directory \"$ROOT_CA_PATH\" does not exists." >&2
	exit 3
fi

echo "Generating private key:"
openssl genrsa -des3 -out $CA_PATH/key/ca.key 4096 -config $CA_PATH/ca.cnf

echo "Generating CA certificate request:"
openssl req -new -sha1 -key $CA_PATH/key/ca.key -out $CA_PATH/crt/ca.csr -config $CA_PATH/ca.cnf

echo "Signing certificate with root CA:"
openssl ca -extensions v3_ca -out $ROOT_CA_PATH/crt/$NAME.crt -in $CA_PATH/crt/ca.csr -config $ROOT_CA_PATH/ca.cnf
cp $ROOT_CA_PATH/crt/$NAME.crt $CA_PATH/crt/ca.crt
