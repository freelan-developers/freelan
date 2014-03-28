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

We also create index files:

    touch index.txt
    touch index.txt.attr

We must now create a configuration file for OpenSSL.

This step is not mandatory since most options can be specified on the OpenSSL command line. However it is really **recommended** that you do not skip it as it simplifies the whole process *a lot*.

Copy the template configuration file from `scripts\resources\ca.cnf`:

    cp <path-to-freelan>/scripts/resources/ca.cnf .

Edit the copied file and replace all occurences of `{CA_PATH}` by the real path you used. In our example, this would be `/home/ca`.

Congratulations ! You just setup your first certificate authority :)

### Creating the authority certificate and its private key

Note: Everything detailed in this section can be done quickly and safely by calling the `generate_ca.sh` script located in the `scripts` directory. The steps here are only provided for completeness.

To create the certification authority certificate and its private key, just type:

    openssl req -new -x509 -extensions v3_ca -keyout key/ca.key -out crt/ca.crt -config ca.cnf

### Creating an intermediate certificate authority

Note: Everything detailed in this section can be done quickly and safely by calling the `generate_intermediate_ca.sh` script located in the `scripts` directory. The steps here are only provided for completeness.

To create an intermediate certification authority, repeat the same steps described in *Setting up the certificate authority* but change the directory. In this example, we consider that we want to create our intermediate certification authority in the `/home/intermediate_ca`.

Create the intermediate certification authority private key file:

    openssl genrsa -des3 -out /key/ca.key 4096 -config $/home/intermediate_ca/ca.cnf

Generate the certificate request:

    openssl req -new -sha1 -key $/home/intermediate_ca/key/ca.key -out $/home/intermediate_ca/crt/ca.csr -config $/home/intermediate_ca/ca.cnf

Sign the certificate request with the root certification authority certificate:

    openssl ca -extensions v3_ca -out $/home/ca/crt/intermediate_ca.crt -in $/home/intermediate_ca/crt/ca.csr -config $/home/ca/ca.cnf

Don't forget to copy the resulting certificate into the intermediate certification authority folder:

    cp $/home/ca/crt/intermediate_ca.crt $/home/intermediate_ca/crt/ca.crt

### Creating a client certificate

Note: Everything detailed in this section can be done quickly and safely by calling the `generate_certificate.sh` script located in the `scripts` directory. The steps here are only provided for completeness.

In this section we will create a certificate signed by the certification authority located at `/home/ca`. You may of course use any certification authority (a root or an intermediate one).

Create the private key file:

    openssl genrsa -des3 -out user.key 4096

Generate the certificate request:

    openssl req -new -sha1 -key user.key -out user.csr

Sign the certificate request with the certification authority certificate:

    openssl ca -out /home/ca/crt/user.crt -in user.csr -config /home/ca/ca.cnf

Don't forget to copy the resulting certificate:

    cp /home/ca/crt/user.crt user.crt
