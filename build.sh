#!/bin/bash

pushd server
mvn package
popd

if [[ ! -d client_build ]]; then 
  mkdir client_build
fi

pushd client_build
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ../client
make

popd
