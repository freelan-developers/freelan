%{!?name:%define name freelan}
%{!?version:%define version 2.1}
%{!?release:%define release 1}

Summary: Peer-to-peer virtual private network daemon
Name:    freelan
Version: %{version}
Release: %{release}
License: GPLv3+
Group:   Networking/Other
URL:     http://www.freelan.org
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: openssl-devel
BuildRequires: boost-devel
BuildRequires: libcurl-devel
Requires: openssl
Requires: boost
Requires: libcurl

%description
Freelan is an application to create secure ethernet tunnels over a
single UDP port. It can be used to create virtual LANs ("Local
Area Network"), hence the name: "freelan".

Freelan may create peer-to-peer tunnel connections or rely on a
more classic client/server layout. The virtual network can be
shaped to fit exactly the bandwidth or topology constraints,
providing an optimal virtual private network.

Freelan is particularly useful for remote sites interconnection and
gaming.

%prep
%setup -q

%build

%install
rm -rf $RPM_BUILD_ROOT
make install PRODUCT_PREFIX=$RPM_BUILD_ROOT/ PRODUCT_BIN_PREFIX=$RPM_BUILD_ROOT/usr
# manpage
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man1
cp packaging/rpm/freelan.1 $RPM_BUILD_ROOT/usr/share/man/man1/freelan.1
# default script
mkdir -p $RPM_BUILD_ROOT/etc/default
cp packaging/rpm/freelan.default $RPM_BUILD_ROOT/etc/default/freelan
# init script
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
cp -p packaging/rpm/freelan.initd $RPM_BUILD_ROOT/etc/rc.d/init.d/freelan

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_initddir}/freelan
%{_bindir}/freelan
%{_mandir}/man1/*
%{_sysconfdir}/default/freelan
%{_sysconfdir}/freelan/freelan.cfg

%preun
# $1 is the number of instances of this package present _after_ the action.
if [ $1 = 0 ]; then
    /sbin/service freelan stop || :
else
    /sbin/service freelan condrestart || :
fi

%changelog
* Fri Aug 11 2017  <sebastien.vincent@freelan.org> - 2.1-1
- First RPM version.

