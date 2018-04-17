%define _use_internal_dependency_generator 0
%define __find_requires %{_builddir}/find-requires
Summary: Slurm SPANK plugins developed by HPCUGent
Name: slurm-spank-talamini
Version: 0.0.1
%global rel	1
Release: %{rel}%{gittag}%{?dist}.ug
License: GPL
Group: System Environment/Base
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: slurm-devel
Requires: slurm

%description
Various Slurm SPANK plugins.

%prep
%setup -q
# Dummy file used to get a RPM dependency on libslurm.so
echo 'int main(){}' > %{_builddir}/libslurm_dummy.c
cat <<EOF > %{_builddir}/find-requires
#!/bin/sh
# Add dummy to list of files sent to the regular find-requires
{ echo %{_builddir}/libslurm_dummy; cat; } | \
    /usr/lib/rpm/redhat/find-requires
EOF
chmod +x %{_builddir}/find-requires

%build
make all
gcc -lslurm -o %{_builddir}/libslurm_dummy %{_builddir}/libslurm_dummy.c

%install
install -d %{buildroot}%{_libdir}/slurm
install -d %{buildroot}%{_sysconfdir}/slurm/plugstack.conf.d
install -m 755 env-test.so %{buildroot}%{_libdir}/slurm/

%clean
rm -rf %{buildroot}

%files
%doc README LICENSE
%defattr(-,root,root,-)
%{_libdir}/slurm/env-test.so

%changelog
* Mon Apr 15 2018 Andy Georges <andy.georges@ugent.be> - 0.0.1-1.ug
- Initial version for UGent
