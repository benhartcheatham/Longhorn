#!/bin/bash

BINUTILS_VER=2.24
GCC_VER=4.9.1
ORIGIN_OS="Arch"
BUILD_DIR=$HOME

if [ $USER = "root" ]
then
    eval BUILD_DIR="~$SUDO_USER/Longhorn-cross"
fi

while getopts ":s :b: :g: :o: :d: :h" opt; do
    case $opt in
        s)
            echo "Versions being used for cross compilation:"
            echo -e "\tbinutils: $BINUTILS_VER \n\tgcc: $GCC_VER \n\tOrigin OS: $ORIGIN_OS \n\tBuild Dir: $BUILD_DIR"
            ;;
        b)
            echo "Setting binutils version to: $OPTARG"
            BINUTILS_VER=$OPTARG
            ;;
        g)
            echo "Setting gcc version to: $OPTARG"
            GCC_VER=$OPTARG
            ;;
        d)
            if [ -n $OPTARG ]
            then
                echo "Setting build directory to: $OPTARG"
                BUILD_DIR=$OPTARG
            fi
            ;;
        o)
            if [ $OPTARG = "Ubuntu" ] 
            then
                echo "OS dependency version set to Ubuntu"
                ORIGIN_OS="Ubuntu"
            fi

            if [ $OPTARG = "Arch" ] 
            then
                echo "OS dependency version set to Arch"
                ORIGIN_OS="Arch"
            fi
            ;;
        h)
            echo -e "Suported origin OSes are Ubuntu and Arch.\nDefault gcc Source Version: 4.9.1\nDefault binutils Source Version: 2.24"
            echo -e "Supported options:\n\t-h: Show this help prompt\n\t-g: Set gcc source version\n\t-b: Set binutils source version"
            echo -e "\t-o: Set origin OS\n\t-d: Sets directory to build in\n\t-s: Show current settings of this script"
            exit 1
            ;;
        \?)
            echo "invalid opts"
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            exit 1
            ;;
    esac
done

if [ $ORIGIN_OS = "Arch" ] 
then
    echo "Installing dependencies for: $ORIGIN_OS"
    echo "sudo pacman -S base-devel gmp libmpc mpfr"
    sudo pacman -S base-devel gmp libmpc mpfr
fi

if [ $ORIGIN_OS = "Ubuntu" ]
then
    echo "Installing dependencies for: $ORIGIN_OS"
    echo "sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo"
    sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo
fi

echo -e "\nStarting Build:\nMaking directory $BUILD_DIR"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo -e "\nGetting binutils source version: $BINUTILS_VER"

curl "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.gz" -o "binutils-$BINUTILS_VER.tar.gz"
ret_code=$?

if [ $ret_code != 0 ]
then
    echo "Getting binutils failed with exit code $ret_code. Exiting..."
    exit $ret_code
fi

echo -e "\nGetting gcc source version: $GCC_VER"
curl "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.gz" -o "gcc-$GCC_VER.tar.gz"
ret_code=$?

if [ $ret_code != 0 ]
then
    echo "Getting gcc failed with exit code $ret_code. Exiting..."
    exit $ret_code
fi

cd $BUILD_DIR
echo -e "\nUnpacking binutils source..."
echo tar -xf "binutils-$BINUTILS_VER.tar.gz"

echo "Unpacking gcc source..."
echo tar -xf "gcc-$GCC_VER.tar.gz"

mkdir -p $BUILD_DIR
export PREFIX="$BUILD_DIR/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo -e "\nStarting Compilation..."
echo "Compiling binutils..."
cd $BUILD_DIR    # just in case
mkdir -p build-binutils
cd build-binutils
../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd $BUILD_DIR
# need to check to make sure $PREFIX/bin is in path here
mkdir -p build-gcc
cd build-gcc
../gcc-$GCC_VER/configure --target=$TARGET --prefix"$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

export PATH="$BUILD_DIR/cross/bin:$PATH"
echo -e "\nCompilation complete."
$TARGET-gcc --version
echo -e "\nTo use this cross compiler outside of this shell session add"
echo "export PATH=\"\$$BUILD_DIR/cross/bin:\$PATH\" to your .profile or .bashrc"

exit 0
