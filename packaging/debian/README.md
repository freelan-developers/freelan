Debian packaging
================

This directory contains scripts to proceed to Debian packaging.

`git-buildpackage` is used for this task.

Configuration
-------------

You may file configuration sample files in the `configuration` directory.

Special usage
-------------

While you may find all this information in the man pages, here are some useful options/settings to use when dealing with Debian packages.

### Generating unsigned packages (useful while testing)

Just use the `-uc` and `-us` options, like so:

    git buildpackage -us -uc
