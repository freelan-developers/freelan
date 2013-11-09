Mac OS X Installer
-------------------

To build the Mac OS X package, you will need to do the following:
- Compile freelan statically;
- Create some directories, /tmp/freelan_pkg, /tmp/freelan_pkg/bin and /tmp/freelan_pkg/etc;
- Copy freelan binary to /tmp/freelan_pkg/bin;
- Copy freelan.cfg configuration file to /tmp/freelan_pkg/etc/
- Copy the gpl-3.0.txt (from the web or freelan git repository) to /tmp/freelan_pkg/
- Open the Freelan_installer.pmdoc with Package Maker and build.

