#!/bin/sh

SCRIPT_PATH=`readlink -f $0 | xargs dirname`

if [ "$#" -ne 1 ]; then
	echo "Syntax: $0 <ca_path>" >&2
	echo >&2
	echo "Create a certification authority at the specified path." >&2
	exit 1
fi

CA_PATH=`readlink -f $1`

if [ -d "$CA_PATH" ]; then
	echo "Error: Directory \"$CA_PATH\" already exists." >&2
	exit 2
fi

mkdir -p $CA_PATH
cd $CA_PATH
mkdir crt key crl
echo '01' > serial
echo '01' > crlnumber
touch index.txt
touch index.txt.attr

cp $SCRIPT_PATH/resources/ca.cnf .
sed -i "s;{CA_PATH};${CA_PATH};g" ca.cnf

echo "Default configuration copied. You may now edit it to your needs... (Press enter)"
read

if ! $EDITOR ca.cnf; then
	echo "The configuration edition was aborted."
	rm -rf "$CA_PATH"
	exit 3
fi

echo "Done."
