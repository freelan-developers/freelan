@echo off

rem A script that generates a CA certificate and its associated private key.
rem 
rem This file is versioned so you probably should copy it first before modifying
rem it if you intend to.

rem Get the root directory
set FREELAN_SERVER_CONFIG_ROOT=%~dp0..\server

set FREELAN_SERVER_OPENSSL_CONFIG_FILE=%FREELAN_SERVER_CONFIG_ROOT%\openssl.cnf
set FREELAN_SERVER_CA_KEY=%FREELAN_SERVER_CONFIG_ROOT%\ca.key
set FREELAN_SERVER_CA_CERTIFICATE_REQUEST=%FREELAN_SERVER_CONFIG_ROOT%\ca.csr
set FREELAN_SERVER_CA_CERTIFICATE=%FREELAN_SERVER_CONFIG_ROOT%\ca.crt

rem Generate the private key
openssl genrsa -out %FREELAN_SERVER_CA_KEY% 2048

rem Generate the certificate request
openssl req -config %FREELAN_SERVER_OPENSSL_CONFIG_FILE% -new -subj "/CN=Certificate Authority/O=freelan/C=FR" -key %FREELAN_SERVER_CA_KEY% -out %FREELAN_SERVER_CA_CERTIFICATE_REQUEST%

rem Generate the certificate
openssl x509 -req -days 3650 -in %FREELAN_SERVER_CA_CERTIFICATE_REQUEST% -signkey %FREELAN_SERVER_CA_KEY% -out %FREELAN_SERVER_CA_CERTIFICATE%
