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

Open or create the configuration file:

    vim ca.cnf

And write the following content:

    #
    # OpenSSL example configuration file.
    # This is mostly being used for generation of certificate requests.
    #
    
    # This definition stops the following lines choking if HOME isn't
    # defined.
    HOME = .
    RANDFILE = $ENV::HOME/.rnd
    
    # Extra OBJECT IDENTIFIER info:
    #oid_file = $ENV::HOME/.oid
    oid_section = new_oids
    
    # To use this configuration file with the "-extfile" option of the
    # "openssl x509" utility, name here the section containing the
    # X.509v3 extensions to use:
    # extensions = 
    # (Alternatively, use a configuration file that has only
    # X.509v3 extensions in its main [= default] section.)
    
    [ new_oids ]
    
    # We can add new OIDs in here for use by 'ca', 'req' and 'ts'.
    # Add a simple OID like this:
    # testoid1=1.2.3.4
    # Or use config file substitution like this:
    # testoid2=${testoid1}.5.6

    [ ca ]
    default_ca = CA_freelan  # The default ca section
    
    [ CA_freelan ]
    
    dir = /home/ca  # Where everything is kept
    certs = $dir/crt  # Where the issued certs are kept
    crl_dir = $dir/crl  # Where the issued crl are kept
    database = $dir/index.txt # database index file.
    #unique_subject = no   # Set to 'no' to allow creation of several ctificates with same subject.
    new_certs_dir = $dir/crt  # default place for new certs.
    
    certificate = $dir/crt/ca.crt  # The CA certificate
    serial = $dir/serial   # The current serial number
    crlnumber = $dir/crlnumber # the current crl number. Must be commented out to leave a V1 CRL.
    crl = $dir/crl/ca.crl   # The current CRL
    private_key = $dir/key/ca.key# The private key
    RANDFILE = $dir/.rand # private random number file
    
    x509_extensions = usr_cert  # The extentions to add to the cert
    
    # Comment out the following two lines for the "traditional"
    # (and highly broken) format.
    name_opt = ca_default  # Subject Name options
    cert_opt = ca_default  # Certificate field options
    
    # Extension copying option: use with caution.
    # copy_extensions = copy
    
    # Extensions to add to a CRL. Note: Netscape communicator chokes on V2 CRLs
    # so this is commented out by default to leave a V1 CRL.
    # crlnumber must also be commented out to leave a V1 CRL.
    # crl_extensions = crl_ext
    
    default_days = 365   # how long to certify for
    default_crl_days= 30   # how long before next CRL
    default_md = default  # use public key default MD
    preserve = no   # keep passed DN ordering
    
    # A few difference way of specifying how similar the request should look
    # For type CA, the listed attributes must be the same, and the optional
    # and supplied fields are just that :-)
    policy = policy_match
    
    # For the CA policy
    [ policy_match ]
    countryName = match
    stateOrProvinceName = match
    organizationName = match
    organizationalUnitName = optional
    commonName = supplied
    emailAddress = optional
    
    # For the 'anything' policy
    # At this point in time, you must list all acceptable 'object'
    # types.
    [ policy_anything ]
    countryName = optional
    stateOrProvinceName = optional
    localityName = optional
    organizationName = optional
    organizationalUnitName = optional
    commonName = supplied
    emailAddress = optional
    
    [ req ]
    default_bits = 1024
    default_keyfile = privkey.pem
    distinguished_name = req_distinguished_name
    attributes = req_attributes
    x509_extensions = v3_ca # The extentions to add to the self signed cert
    
    # Passwords for private keys if not present they will be prompted for
    # input_password = secret
    # output_password = secret
    
    # This sets a mask for permitted string types. There are several options. 
    # default: PrintableString, T61String, BMPString.
    # pkix  : PrintableString, BMPString (PKIX recommendation before 2004)
    # utf8only: only UTF8Strings (PKIX recommendation after 2004).
    # nombstr : PrintableString, T61String (no BMPStrings or UTF8Strings).
    # MASK:XXXX a literal mask value.
    # WARNING: ancient versions of Netscape crash on BMPStrings or UTF8Strings.
    string_mask = utf8only
    
    # req_extensions = v3_req # The extensions to add to a certificate request
    
    [ req_distinguished_name ]
    countryName = Country Name (2 letter code)
    countryName_default = AU
    countryName_min = 2
    countryName_max = 2
    
    stateOrProvinceName = State or Province Name (full name)
    stateOrProvinceName_default = Some-State
    
    localityName = Locality Name (eg, city)
    
    0.organizationName = Organization Name (eg, company)
    0.organizationName_default = Internet Widgits Pty Ltd
    
    # we can do this but it is not needed normally :-)
    #1.organizationName = Second Organization Name (eg, company)
    #1.organizationName_default = World Wide Web Pty Ltd
    
    organizationalUnitName = Organizational Unit Name (eg, section)
    #organizationalUnitName_default =
    
    commonName = Common Name (eg, YOUR name)
    commonName_max = 64
    
    emailAddress = Email Address
    emailAddress_max = 64
    
    # SET-ex3 = SET extension number 3
    
    [ req_attributes ]
    challengePassword = A challenge password
    challengePassword_min = 4
    challengePassword_max = 20
    
    unstructuredName = An optional company name
    
    [ usr_cert ]
    # These extensions are added when 'ca' signs a request.
    
    # This goes against PKIX guidelines but some CAs do it and some software
    # requires this to avoid interpreting an end user certificate as a CA.
    
    basicConstraints=CA:FALSE
    
    # Here are some examples of the usage of nsCertType. If it is omitted
    # the certificate can be used for anything *except* object signing.
    
    # This is OK for an SSL server.
    # nsCertType = server
    
    # For an object signing certificate this would be used.
    # nsCertType = objsign
    
    # For normal client use this is typical
    # nsCertType = client, email
    
    # and for everything including object signing:
    # nsCertType = client, email, objsign
    
    # This is typical in keyUsage for a client certificate.
    # keyUsage = nonRepudiation, digitalSignature, keyEncipherment
    
    # This will be displayed in Netscape's comment listbox.
    nsComment = "OpenSSL Generated Certificate"
    
    # PKIX recommendations harmless if included in all certificates.
    subjectKeyIdentifier=hash
    authorityKeyIdentifier=keyid,issuer
    
    # This stuff is for subjectAltName and issuerAltname.
    # Import the email address.
    # subjectAltName=email:copy
    # An alternative to produce certificates that aren't
    # deprecated according to PKIX.
    # subjectAltName=email:move
    
    # Copy subject details
    # issuerAltName=issuer:copy
    
    #nsCaRevocationUrl = http://www.domain.dom/ca-crl.pem
    #nsBaseUrl
    #nsRevocationUrl
    #nsRenewalUrl
    #nsCaPolicyUrl
    #nsSslServerName
    
    # This is required for TSA certificates.
    # extendedKeyUsage = critical,timeStamping
    
    [ v3_req ]
    # Extensions to add to a certificate request
    
    basicConstraints = CA:FALSE
    keyUsage = nonRepudiation, digitalSignature, keyEncipherment
    
    [ v3_ca ]
    # Extensions for a typical CA
    
    # PKIX recommendation.
    subjectKeyIdentifier=hash
    authorityKeyIdentifier=keyid:always,issuer
    
    # This is what PKIX recommends but some broken software chokes on critical
    # extensions.
    #basicConstraints = critical,CA:true
    # So we do this instead.
    basicConstraints = CA:true
    
    # Key usage: this is typical for a CA certificate. However since it will
    # prevent it being used as an test self-signed certificate it is best
    # left out by default.
    # keyUsage = cRLSign, keyCertSign
    
    # Some might want this also
    # nsCertType = sslCA, emailCA
    
    # Include email address in subject alt name: another PKIX recommendation
    # subjectAltName=email:copy
    # Copy issuer details
    # issuerAltName=issuer:copy
    
    # DER hex encoding of an extension: beware experts only!
    # obj=DER:02:03
    # Where 'obj' is a standard or added object
    # You can even override a supported extension:
    # basicConstraints= critical, DER:30:03:01:01:FF
    
    [ crl_ext ]
    # CRL extensions.
    # Only issuerAltName and authorityKeyIdentifier make any sense in a CRL.
    
    # issuerAltName=issuer:copy
    authorityKeyIdentifier=keyid:always
    
    [ proxy_cert_ext ]
    # These extensions should be added when creating a proxy certificate
    
    # This goes against PKIX guidelines but some CAs do it and some software
    # requires this to avoid interpreting an end user certificate as a CA.
    
    basicConstraints=CA:FALSE
    
    # Here are some examples of the usage of nsCertType. If it is omitted
    # the certificate can be used for anything *except* object signing.
    
    # This is OK for an SSL server.
    # nsCertType = server
    
    # For an object signing certificate this would be used.
    # nsCertType = objsign
    
    # For normal client use this is typical
    # nsCertType = client, email
    
    # and for everything including object signing:
    # nsCertType = client, email, objsign
    
    # This is typical in keyUsage for a client certificate.
    # keyUsage = nonRepudiation, digitalSignature, keyEncipherment
    
    # This will be displayed in Netscape's comment listbox.
    nsComment = "OpenSSL Generated Certificate"
    
    # PKIX recommendations harmless if included in all certificates.
    subjectKeyIdentifier=hash
    authorityKeyIdentifier=keyid,issuer
    
    # This stuff is for subjectAltName and issuerAltname.
    # Import the email address.
    # subjectAltName=email:copy
    # An alternative to produce certificates that aren't
    # deprecated according to PKIX.
    # subjectAltName=email:move
    
    # Copy subject details
    # issuerAltName=issuer:copy
    
    #nsCaRevocationUrl = http://www.domain.dom/ca-crl.pem
    #nsBaseUrl
    #nsRevocationUrl
    #nsRenewalUrl
    #nsCaPolicyUrl
    #nsSslServerName
    
    # This really needs to be in place for it to be a proxy certificate.
    proxyCertInfo=critical,language:id-ppl-anyLanguage,pathlen:3,policy:foo

You may of course change some properties so they match your wishes. Just ensure to have consistent values for anything that is related to the filesystem paths.

### Creation of the certification authority certificate

Create the authority certificate and its private key:

    openssl req -new -x509 -extensions v3_ca -keyout key/ca.key -out crt/ca.crt -config ca.cnf


