#!/bin/sh

#  libcopy.sh
#  kitsas
#
#  Created by Petri Aarnio on 29/06/2018.
#  

APPNAME=Kitsas
APPCONTENTS=$1/$APPNAME.app/Contents
APPDIR=$APPCONTENTS/MacOS
LIBDIR=$APPCONTENTS/Libraries

mkdir -p $LIBDIR/

#app
#install_name_tool -change /usr/local/opt/libzip/lib/libzip.5.dylib @rpath/libzip.dylib $APPDIR/$APPNAME

#libzip
#cp /usr/local/lib/libzip.dylib $LIBDIR
#chmod +w $LIBDIR/libzip.dylib
#install_name_tool -id @rpath/libzip.dylib $LIBDIR/libzip.dylib
