#!/bin/bash
#
# A script that generates a CA certificate and its associated private key.
#
# This file is versioned so you probably should copy it first before modifying
# it if you intend to.

# Get the root directory
export FREELAN_SERVER_CONFIG_ROOT=$(readlink -f `dirname $0`/../server)

export FREELAN_SERVER_OPENSSL_CONFIG_FILE=${FREELAN_SERVER_CONFIG_ROOT}/openssl.cnf
export FREELAN_SERVER_CA_KEY=${FREELAN_SERVER_CONFIG_ROOT}/ca.key
export FREELAN_SERVER_CA_CERTIFICATE_REQUEST=${FREELAN_SERVER_CONFIG_ROOT}/ca.csr
export FREELAN_SERVER_CA_CERTIFICATE=${FREELAN_SERVER_CONFIG_ROOT}/ca.crt

# Generate the private key
openssl genrsa -out ${FREELAN_SERVER_CA_KEY} 2048

# Generate the certificate request
openssl req -config ${FREELAN_SERVER_OPENSSL_CONFIG_FILE} -new -subj '/CN=Certificate Authority/O=freelan/C=FR' -key ${FREELAN_SERVER_CA_KEY} -out ${FREELAN_SERVER_CA_CERTIFICATE_REQUEST}

# Generate the certificate
openssl x509 -req -days 3650 -in ${FREELAN_SERVER_CA_CERTIFICATE_REQUEST} -signkey ${FREELAN_SERVER_CA_KEY} -out ${FREELAN_SERVER_CA_CERTIFICATE}
