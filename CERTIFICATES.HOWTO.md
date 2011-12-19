Certificates generation
=======================

FreeLAN provides security trough the use of X509 certificates.

You may use any certificate and/or certificate authority(ies) you want, But if you never worked with X509 certificates, this how-to will help you getting started.

This documentation assumes the use of OpenSSL for certificate generation. Ensure that you have it installed and that `openssl` is available from the command-line.

Terminology
-----------

Here are the definitions of some common terms used in this documentation:

* certificate: Some data that identifies a host, computer, person. Certificates contains a public key and are *signed* asymmetrically by their **private key**.
* private key: Some randomly generated data used to *sign* a certificate or to *cipher* some data. A private key is, as its name indicates, **private** and should **always** be kept secret in a secure location. If your private key is compromised, all the certificates, data, that it signed or ciphered **must** be considered unsafe.
* certificate authority: A root certificate that signs other certificates and acts as a trust entity.
* intermediate certificate authority: A certificate that is signed by a root or an intermediate certificate and used to sign other certificates. An intermediate certificate can be used to separate different groups of child certificates.
* common name: The name indicated in a certificate that identifies the host, computer, person. Common names are case-sensitive.

Let's go
--------

### Setting up the certificate authority

Note: Everything detailed in this section can be done quickly and safely by calling the `create_ca.sh` script located in the `scripts` directory. The steps here are only provided for completeness.

If you don't already have a certificate authority, follow these steps to create one.

Choose a directory into which you will setup the certificate authority. This can be any directory: you just have to ensure that it will be readable only by you.

In the next step, we assume you chose `/home/ca`:

    cd /home/ca
    mkdir crt key crl

Next, we create a serial number file. This file just contains an ASN1 number indicating the serial number to use for the next generated certificate. This file acts exactly like a counter:

    echo '01' > serial

Do the same for the `crlnumber` file:

    echo '01' > crlnumber

We also create an index file:

    touch index.txt

We must now create a configuration file for OpenSSL.

This step is not mandatory since most options can be specified on the OpenSSL command line. However it is really **recommended** that you do not skip it as it simplifies the whole process *a lot*.

Copy the template configuration file from `scripts\resources\ca.cnf`:

    cp <path-to-freelan>/scripts/resources/ca.cnf .

Edit the copied file and replace all occurences of `{CA_PATH}` by the real path you used. In our example, this would be `/home/ca`.

Create the authority certificate and its private key:

    openssl req -new -x509 -extensions v3_ca -keyout key/ca.key -out crt/ca.crt -config ca.cnf

Congratulations ! You just setup your first certificate authority :)

### Creating an intermediate certificate authority
