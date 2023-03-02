#!/bin/bash

mkdir compressor_build
cd compressor_build
cmake ../tools/compressor
make -j8
