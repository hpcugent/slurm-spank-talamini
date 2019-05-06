#!/bin/bash

PACKAGE=slurm-spank-talamini
GITTAG=$(git log --format=%ct.%h -1)
VERSION=$(grep "Version:.*[0-9]" ${PACKAGE}.spec | tr -s " " |  awk '{print $2;}')
RELEASE=$(grep "%global rel.*[-1-9]" ${PACKAGE}.spec | tr -s " " | awk '{print $3}')

SUFFIX=${VERSION}-${RELEASE}


mkdir -p BUILD SOURCES SPECS RPMS BUILDROOT
git archive --format=tar.gz -o "SOURCES/${PACKAGE}-${SUFFIX}.tar.gz" --prefix="${PACKAGE}-${VERSION}/" HEAD
cp ${PACKAGE}.spec "SPECS"
rpmbuild --define "gittag ${GITTAG}" --define "_topdir $PWD" -ba SPECS/${PACKAGE}.spec
rm -rf BUILD SOURCES BUILDROOT SPECS

