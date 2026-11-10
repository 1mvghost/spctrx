#!/bin/sh

export TARGET="x86_64-spctrx"
export PREFIX="$HOME/opt/spctrx-tools"

mkdir -p $PREFIX

#
# clean
#

echo "cleaning..."
rm -rf ./binutils*
rm -rf ./gcc*
rm -rf $PREFIX/*

#
# clone the repositories
#

echo "getting repositories..."
git clone https://sourceware.org/git/binutils-gdb.git
git clone https://gcc.gnu.org/git/gcc.git

#
# patch the tools
#

echo "patching..."
cd binutils-gdb
git checkout .
git apply ../patches/binutils-patch.diff
cd ..

cd gcc
git checkout .
git apply ../patches/gcc-patch.diff
cd ..

#
# compile cross binutils
#

echo "installing binutils..."

mkdir binutils-build
cd binutils-build

../binutils-gdb/configure --target=$TARGET --prefix="$PREFIX" -disable-werror

make all -j 8
make install -j 8

cd ..

#
# compile cross gcc
#

echo "installing gcc..."

mkdir gcc-build
cd gcc-build

#
# todo: libc
#

../gcc/configure --target=$TARGET --prefix="$PREFIX" --without-headers --enable-languages=c,c++ --enable-shared --disable-nls

make all-gcc -j 8
make all-target-libgcc -j 8
make install-gcc -j 8
make install-target-libgcc -j 8

cd ..

#
# remove temporary files
#

echo "removing temporary files..."
rm -rf ./binutils*
rm -rf ./gcc*

echo "toolchain building done. binaries are located at $PREFIX/bin/"