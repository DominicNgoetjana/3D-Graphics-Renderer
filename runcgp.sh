#!/bin/sh

if [ $1 = "compile" ]
then
cd build;
make -j8;
cd ..;
fi
build/tesselate/tessviewer
