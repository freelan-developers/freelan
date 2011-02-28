libcryptopen
============

libcryptopen stands a simple, yet complete, C++ wrapper around the OpenSSL cryptographic library.

It provides exception safe classes and functions to deal with symmetric ciphers, public key cryptography, certificates, authentication codes, hash functions and data encoding.

Which parts of the OpenSSL crypto library does it provide ?
-----------------------------------------------------------

In the end: everything.


From now on, here is what is currently implemented:

 - HMAC
 - Error handling
 - Exceptions

And here is what is planned:

 - Hash methods
 - Threads
 - Random
 - ASN1
 - BIO
 - EVP
 - PEM
 - X509
 - Public Key Cryptography

   - DSA
   - RSA
   - Diffie-Hellman

 - Symmetric Ciphers

Note: Support for SSL/TLS/DTLS is **NOT** planned as they do not belong to the OpenSSL crypto library.


Licensing
---------

All code is licensed under the GPLv3. See gpl-3.0.txt.

If you are interested in using this project under a different license, please contact me. I don't bite and I probably won't charge (at least, not much).

Does the project have a website ?
---------------------------------

Yes, it does. And [here](http://www.freelan.org) it is. 
