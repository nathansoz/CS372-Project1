#!/bin/bash

export CC=clang
export CXX=clang++

pushd server
mvn package
popd

if [[ ! -d client_build ]]; then 
  mkdir client_build
fi

pushd client_build
cmake ../client
make

popd
