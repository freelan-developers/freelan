Linux packaging how-to
======================

This how-to explains the guidelines to follow to package freelan libraries on the different linux platforms.

All necessary scripts can be found in `scripts` directory.

Debian & Ubuntu
---------------

Debian and Ubuntu distributions share the same packet structure and so require the same tools to get built.

### Packaging a particular version

Run the `get_archive` script to get the archive for a specified library/version.

> ./scripts/get_archive <library> <version> [directory]

Go into this archive extracted directory:

> cd library-version

Then, from within the extracted directory, type:

> dh_make -l -e your.email@address -c gpl3

This will create a `debian` sub-directory which contains all the needed control files.

Delete all examples files using the following command:

> rm -f debian/*.ex debian/*.EX debian/README*

When you are done, generate the source package:

> debuild -S -sa --lintian-opts -i
