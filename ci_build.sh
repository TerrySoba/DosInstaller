#!/bin/bash
set -e

DIR=$(dirname $0)
cd $DIR
BASE_DIR=$(pwd)
mkdir -p build
cd $BASE_DIR/build

tar -xJf $BASE_DIR/open_watcom/open_watcom.tar.xz

export WATCOM=$BASE_DIR/build/opt/opwatcom
export PATH=$WATCOM/binl:$PATH
export EDPATH=$WATCOM/eddat
export INCLUDE=$WATCOM/h

cd $BASE_DIR/source

make -j5
