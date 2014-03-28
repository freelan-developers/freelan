In this directory, you will find:

- The source code for the Win32/64 TAP adapter.
- The source code for a tool that ease TAP adapter installation/removal.

You may compile the driver but unless you have a trusted certificate, you won't be able to sign it (and so to install it). 

We currently don't have such a certificate and even if we did, we could not publish since it would compromise it's security. Such a certificate costs about $500/year.

As a free open-source project, we don't have any money to spend for a certificate, and until we do, we'll use the OpenVPN signed driver which relies on a similar source code. If you feel like you want to contribute financially to freelan so we get our own certificate, write to julien.kauffmann@freelan.org.

For now on, we use the same driver as OpenVPN which, apart from the name, has the exact same code.

The tool can setup any driver (either OpenVPN's one or our own) so feel free to use it and redistribute it (under the terms of his license).
