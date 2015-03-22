#!/bin/sh

set -e

sudo apt-get update
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install build-essential g++-4.9 clang-3.5 llvm-3.5-dev libedit-dev libboost-all-dev git valgrind 
sudo ln -s /usr/bin/clang++-3.5 /usr/local/bin/clang++ || true
sudo ln -s /usr/bin/llvm-config-3.5 /usr/local/bin/llvm-config || true

git clone https://github.com/mbmaier/mblib.git
cd mblib
git pull
sudo make install
cd ..

git clone https://github.com/mbmaier/asm-lisp.git
cd asm-lisp
git pull
make build-dirs
make full-test
make debug
sudo make install
