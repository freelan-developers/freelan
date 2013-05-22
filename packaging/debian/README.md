Debian packaging
================

This directory contains scripts to proceed to Debian packaging.

The tools used for this task are:

* `git-buildpackage` and `pristine-tar` to generate and build Debian source and binary packages.

Configuration
-------------

You may file configuration sample files in the `configuration` directory.

Special usage
-------------

While you may find all this information in the man pages, here are some useful options/settings to use when dealing with Debian packages.

### Generating unsigned packages (useful while testing)

Just use the `-uc` and `-us` options, like so:

    git buildpackage -us -uc

To generate only source packages (recommended if you intend to use cowbuilder/pbuilder):

    git buildpackage -S

Using GPG
---------

Ensure your GPG keys were imported, and that you specified your key in your `.bashrc` file (replace the key id as appropriate):

    export GPGKEY=6198217D

Also you probably should set those environment variables in your `.bashrc` as well (replace the values as appropriate):

    export DEBEMAIL="julien.kauffmann@freelan.org"
    export DEBFULLNAME="Julien Kauffmann"
    export DEBSIGN_KEYID=${GPGKEY}
