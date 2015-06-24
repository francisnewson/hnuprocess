#!/bin/bash
#echo "Loading ROOT 5.34.14 x86_64-sl6-gcc46-opt ..."
#source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.14/x86_64-slc6-gcc46-opt/root/bin/thisroot.sh
echo "Loading ROOT 5.34.30 x86_64-sl6-gcc48-opt ..."
source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.30/x86_64-slc6-gcc48-opt/root/bin/thisroot.sh
echo "Loading python py27 ... "
source /afs/cern.ch/user/f/fnewson/.virtualenvs/py27/bin/activate
echo "Loading clang ... "
source ~/envfiles/clang.env.sh

echo "Setting environment variables for SCons"
export SC_ROOT_DIR=/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.30/x86_64-slc6-gcc48-opt/root
export SC_BOOST_DIR=/afs/cern.ch/sw/lcg/external/Boost/1.55.0_python2.7/x86_64-slc6-gcc47-opt
export SC_BOOST_NAME=boost-1_55
export SC_YAML_DIR=/afs/cern.ch/user/f/fnewson/work/programs/yaml-cpp/yaml-cpp-0.5.1
export SC_EVENT_DIR=/afs/cern.ch/user/f/fnewson/work/hnu/gopher/code/fneevent/build_sprocess/
