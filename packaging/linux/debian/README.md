Debian packaging
================

This folder contains a `Makefile` to help you build the debian packages.

Just type:

    make sync

To import the git repositories.

To clean everything after you built packages, use:

    make clean

Why not git submodules ?
------------------------

Normally, we would have created a link to the repositories using `git
submodule`. But unfortunately, for now, the `pristine-tar` doesn't seem to
handle those very well.

We may switch to `git submodule` approach as soon as `pristine-tar` gets fixed.
