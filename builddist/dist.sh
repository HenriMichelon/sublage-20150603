#!/bin/sh
cd ..
rev=`cat VERSION`
context_h="sublage/include/sublage/context.h"
echo "building dist $rev..."

if [ -z "$TMP" ]
then
    TMP="/tmp"
fi
ctxhtmp="$TMP/`basename context_h`_$$"
cp $context_h $ctxhtmp
cat $ctxhtmp | sed "s/\(#define SUBLAGE_REVISION \)[0-9]*/\1$rev/" > $context_h
rm -f $ctxhtmp

os=`uname -s | tr [:upper:] [:lower:]`
case $os in
    "freebsd")
        make=gmake
        ;;
    *)
        make=make
        ;;
esac

export MODE='RELEASE'
cd sublage
$make clean && $make
cd ../stdlib
ant clean
cd src
$make clean && $make
cd ..
export LD_LIBRARY_PATH=../sublage:$LD_LIBRARY_PATH
ant compile
cd ..
unset MODE

distdir="$TMP/sublage-r$rev"
if [ -e $distdir ]
then
    echo "$distdir already exists"
    exit
fi
subdir="$distdir/lib/sublage"
for d in $distdir/bin $subdir/stdlib $subdir/bin
do
    mkdir -p $d
done
for ext in library native
do
    cp stdlib/binaries/*.$ext $subdir/stdlib
done
for f in sublage sublagec
do
    cp sublage/$f $subdir/bin
done
cp LICENCE $distdir
cp builddist/sublage.sh $distdir/bin/sublage
cp builddist/sublagec.sh $distdir/bin/sublagec

dist="`pwd`/dist"
mkdir -p $dist
arch=`uname -m`
case $os in
    "darwin")
        rt=libsublagert.dylib
        ;;
    "linux")
        rt=libsublagert.so
        ;;
    "freebsd")
        rt=libsublagert.so
        ;;
esac

cat builddist/install.sh | sed "s/SUBLAGERT/$rt/" > $distdir/install.sh
chmod +x $distdir/install.sh
cp sublage/$rt $distdir/lib
prev=`pwd`
cd $TMP
zip -r $dist/sublage-r$rev-$os-$arch.zip `basename $distdir`
tar zcf $dist/sublage-r$rev-$os-$arch.tar.gz `basename $distdir`
cd $prev

case $os in
    "darwin")
        /usr/local/bin/packagesutil --file builddist/Sublage.pkgproj set project name "Sublage-r$rev"
        /usr/local/bin/packagesbuild -v builddist/Sublage.pkgproj set package version $rev
        ;;
    "linux")
        rm -f $distdir/install.sh
        mkdir $distdir/usr
        mv $distdir/* $distdir/usr
        mkdir $distdir/DEBIAN
        cp builddist/control $distdir/DEBIAN
        mv $distdir/usr/LICENCE $distdir/DEBIAN/copyright
        dpkg -b $distdir $dist/sublage-r${rev}_amd64.deb
        ;;
esac
rm -rf $distdir
