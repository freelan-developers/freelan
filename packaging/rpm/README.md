# RPM packaging

## Prerequisites

The rpm-build package must be installed:

`yum install rpm-build`

Be sure to have the FreeLAN dependencies:

`yum install scons boost-devel libcurl-devel openssl-devel miniupnpc-devel help2man`

## Build

Create a tar archive and upload it to the build directory:

```
make package
mv freelan-$(cat VERSION).tar.gz ~/rpmbuild/SOURCES/
```

Run the following command to build the package:

`rpmbuild -ba ./packaging/rpm/freelan.spec`

## Package

The generated RPM package will be in ~/rpmbuild/RPMS/.

