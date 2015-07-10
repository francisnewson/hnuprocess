#!/bin/bash

echo "Loading /disk/homedisk/home/fon/work/hnu/code/scripts/env"
source /disk/homedisk/home/fon/work/hnu/code/scripts/env

echo "Loading ROOT 5.34.30 x86_64-sl6-gcc48-opt ..."
source /home/fon/work/hnu/code/root/root-5.34.32/root/bin/thisroot.sh
echo "Loading python py27 ... "
source /home/fon/work/hnu/code/python/venv/hnu/bin/activate
echo "Loading gcc ... "
export PATH=/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/bin:$PATH
export LD_LIBRARY_PATH=/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/lib64:/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/lib:$LD_LIBRARY_PATH
export CC=/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/bin/gcc
export CXX=/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/bin/g++

export LD_LIBRARY_PATH=/home/fon/work/hnu/code/python/version/Python-2.7.9/lib:$LD_LIBRARY_PATH
export PATH=/home/fon/work/hnu/code/python/version/Python-2.7.9/bin/:$PATH

echo "Setting environment variables for SCons"
export SC_ROOT_DIR=/home/fon/work/hnu/code/root/root-5.34.32/root
export SC_BOOST_DIR=/home/fon/work/hnu/code/boost/boost_1_55_0
export SC_BOOST_NAME=boost-1_55_0
export SC_YAML_DIR=/home/fon/work/hnu/code/yaml/yaml-cpp
export SC_EVENT_DIR=/home/fon/work/hnu/code/fneevent/build_bham

export MACHINE=bham
