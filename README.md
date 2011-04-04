libcryptoplus
=============

libcryptoplus stands a lightweight, simple, yet complete, C++ wrapper around the OpenSSL cryptographic library.

It provides exception safe classes and functions to deal with symmetric ciphers, public key cryptography, certificates, authentication codes, hash functions and data encoding.

Which parts of the OpenSSL crypto library does it provide ?
-----------------------------------------------------------

In the end: everything.

From now on, here is what is currently implemented:

 - HMAC
 - Error handling
 - Exceptions
 - Hash methods
 - PBKDF2
 - Random
 - Symmetric Ciphers
 - EVP
 - DER
 - BIO
 - BIGNUM
 - ASN1
 - Public Key Cryptography

   - RSA
   - DSA
   - Diffie-Hellman
   - EVP PKEY

And here is what is left to be done:

 - X509

Note: Support for SSL/TLS/DTLS is **NOT** planned as they do not belong to the OpenSSL crypto library.

Why would I use it when I can directly use OpenSSL ?
----------------------------------------------------

Before answering this question, I have to say that I have a mixed relationship with OpenSSL: I love it and hate it at the same time.

I love it, because if offers everything one might need. It was created by very smart people with a strong care for security concerns. It is widely used, efficient and still maintained.

Despite that, I sometimes hate it because even if it does the job, the documentation is almost inexistant or erroneous, the API is inconsistent in both naming and argument typing. What you could do in a couple of minutes takes a week to do, just because of the lack of reliable information.

So, why would you use libcryptoplus ? After all, it is just a wrapper: it doesn't bring any new functionnality in itself. So if you already have an existing and working code base, you probably don't need to use libcryptoplus.

However, it greatly increases code maintainability by providing a object-oriented **RAII** approach of dealing with OpenSSL concepts.

Using libcryptoplus, you will:

 - No longer have to care about your objects lifetime: libcryptoplus heavily relies on **RAII** (Resource Acquisition Is Initialization) and smart pointer mechanisms.
 - Benefit from an exception-oriented environment so can focus on writing efficient code, not on checking return codes for every single line.
 - Use a consistent and **const-correct** API. All the ugly `const_cast<>` are done for you underneath, and you don't need to care anymore about that.
 - Use modern C++ approaches to deal with your objects: you can now *iterate*  trough the extensions of a X509 certificate or the entries of a X509 name using well-known C++ iterators.
 - Have no performance issues: 99% of libcryptoplus source code is *inline* functions: as a result, any modern compiler with optimizations enabled will likely generate the **exact same machine code** that you would have had using the genuine interface.
 - Use a really lightweight library: libcryptoplus does not require any additional library (except boost, but this dependency will go away as soon as the next C++ version goes out, which at the time of this writing should be not that far).
 - Not have to drop your code base: libcryptoplus provides a way to get the RAW pointers of every single OpenSSL object so that if something is missing, you can still use a low-level function to get the job done.

So sure, you can always use OpenSSL directly, that is what I did the past few years as well. But how many time did I wish there was a more C++ alternative ? Now, there is. And it is free ! :)

You claim that almost everything is supported, but [some functionality] is missing !
------------------------------------------------------------------------------------

I tried to implement everything I could but surely, some parts remain unimplemented.

The reason can be either a lack of information (have you ever tried to find the documenation of OpenSSL X509 extensions ? Good luck !), or just something I missed.

In any case, feel free to submit your requests: my intent is that this library becomes more and more complete.

You may also submit patches, features, bug fixes (please use: push requests) and I would be glad to add your name to the contributors list if you want to. The only requirement regarding new features is that they remain of general utility. So, a feature like "Patch to ease certificate generation by automatically setting the subject name domain to mycompany.com" will obviously be rejected, and its author will have to buy me a cake. (Blame me, but I love cake).

What about all the ugly `const_cast<>` ?
----------------------------------------

This is probably what I hate the most when I need to interface my const-correct code with OpenSSL: you must either copy the data to a writable buffer or `const_cast<>` it (with all the risks that comes along) before issuing the call.

Doing a `const_cast<>` is not that bad as long as you have the guarantee that the calling function will not attempt to modify the input buffer. Everytime I had to put a `const_cast<>` somewhere, I double (even triple)-checked both OpenSSL documentation and source code to be sure that I could reasonably assume that the variable was not meant to be modified.

However, if you find any evidence that some `const_cast<>` is not legitimate, please tell-me: I chose the "casting way" over the "copy way" for performance's sake, but not at the cost of leading to *undefined behavior*.

Licensing
---------

All code is licensed under the GPLv3. See gpl-3.0.txt.

If you are interested in using this project under a different license, please [contact me](mailto:julien.kauffmann__AT__freelan.org). I don't bite and I probably won't charge (at least, not much).

Does the project have a website ?
---------------------------------

Yes, it does. And [here](http://www.freelan.org) it is. 

You may also find the [git repository](https://github.com/ereOn/libcryptoplus) on github.
