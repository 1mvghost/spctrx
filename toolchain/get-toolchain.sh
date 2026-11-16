#!/bin/bash

AUTOCONF_VERSION="2.69"
AUTOCONF_URL="https://ftp.gnu.org/gnu/autoconf"
AUTOCONF_FILE="autoconf-${AUTOCONF_VERSION}"

AUTOMAKE_VERSION="1.15.1"
AUTOMAKE_URL="https://ftp.gnu.org/gnu/automake"
AUTOMAKE_FILE="automake-${AUTOMAKE_VERSION}"

BINUTILS_VERSION="2.47"
BINUTILS_URL="https://ftp.gnu.org/gnu/binutils"
BINUTILS_FILE="binutils-${BINUTILS_VERSION}"

GCC_VERSION="16.2.0"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}"
GCC_FILE="gcc-${GCC_VERSION}"

getTarball() {
	echo "getting $2..."

	local tarball="$2.tar.xz"

	curl "$1/$tarball" -o ./temp/$tarball

	cd temp
	tar -xf ./$tarball
	cd ..
}

rm -rf ./temp
rm -rf $HOME/opt/spctrx-cross

mkdir temp
mkdir -p $HOME/opt/spctrx-cross

export PREFIX="$HOME/opt/spctrx-cross"
export TARGET="x86_64-spctrx"

getTarball \
	${BINUTILS_URL} \
	${BINUTILS_FILE} \

getTarball \
	${GCC_URL} \
	${GCC_FILE} \

getTarball \
	${AUTOMAKE_URL} \
	${AUTOMAKE_FILE} \

getTarball \
	${AUTOCONF_URL} \
	${AUTOCONF_FILE} \

cd temp
	cd ${AUTOCONF_FILE}
		./configure --prefix=${PREFIX}

		make -j 8
		make install -j 8
	cd ..

	cd ${AUTOMAKE_FILE}
		./configure --prefix=${PREFIX}

		make -j 8
		make install -j 8
	cd ..

	cd ${BINUTILS_FILE}
		patch --input ../../patches/binutils.diff --strip 1

		mkdir ../temp-binutils
		cd ../temp-binutils

		../${BINUTILS_FILE}/configure \
			--prefix="${PREFIX}" \
			--target=${TARGET} \
			--disable-nls \
			--disable-werror \
			--enable-default-execstack=no \

		make -j 8
		make install -j 8
	cd ..

	cd ${GCC_FILE}
		patch --input ../../patches/gcc.diff --strip 1
	
		cd libstdc++-v3
			${PREFIX}/bin/autoconf
		cd ..

		mkdir ../temp-gcc
		cd ../temp-gcc
		
		../${GCC_FILE}/configure \
			--prefix="${PREFIX}" \
			--target=${TARGET} \
			--disable-nls \
			--without-headers \
			--enable-languages=c,c++ \
			--enable-shared \
			--disable-hosted-libstdcxx \
		
		make all-gcc -j 8
		make all-target-libgcc -j 8
		make all-target-libstdc++-v3 -j 8

		make install-gcc -j 8
		make install-target-libgcc -j 8
		make install-target-libstdc++-v3 -j 8
	cd ..
cd ..

echo "cleaning up..."
rm -rf ./temp

echo "finished! binaries are located at ${PREFIX}/bin/"