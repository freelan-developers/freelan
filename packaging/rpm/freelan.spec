%{!?name:%define name freelan}
%{!?version:%define version 2.3}
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
BuildRequires: miniupnpc-devel
BuildRequires: help2man
Requires: openssl
Requires: boost
Requires: libcurl
Requires: miniupnpc

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
rm -rf $RPM_BUILD_ROOT
make

%install
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
cp -a build/release/bin/freelan $RPM_BUILD_ROOT/%{_bindir}/
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/freelan
cp -a apps/freelan/config/freelan.cfg $RPM_BUILD_ROOT/%{_sysconfdir}/freelan/ 
# manpage
mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man1
cp build/release/man/freelan.1 $RPM_BUILD_ROOT/%{_mandir}/man1/freelan.1
# systemd unit
mkdir -p $RPM_BUILD_ROOT/%{_unitdir}
cp -p packaging/rpm/freelan@.service $RPM_BUILD_ROOT/%{_unitdir}/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_unitdir}/freelan@.service
%{_bindir}/freelan
%{_mandir}/man1/*
%{_sysconfdir}/freelan/freelan.cfg

%preun
# $1 is the number of instances of this package present _after_ the action.
if [ $1 = 0 ]; then
    systemctl stop freelan || :
else
    systemctl condrestart freelan || :
fi

%changelog
* Tue May 7 2019  <sebastien.vincent@freelan.org> - 2.2-1
- Version 2.2.

* Fri Aug 11 2017  <sebastien.vincent@freelan.org> - 2.1-1
- First RPM version.

