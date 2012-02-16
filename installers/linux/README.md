Linux packaging how-to
======================

This how-to explains the guidelines to follow to package freelan libraries on the different linux platforms.

Debian & Ubuntu
---------------

Debian and Ubuntu distributions share the same packet structure and so require the same tools to get built.

### Packaging a particular version

Run the `get_archive.sh` script to get the archive for a specified library/version.

> get_archive.sh library version

Go into this archive extracted directory:

> cd library-version

Then, from within the extracted directory, type:

> dh_make -l -e your.email@address -c gpl3

This will create a `debian` sub-directory which contains all the needed control files.

When you are done, generate the source package:

> debuild -S -sa --lintian-opts -i
