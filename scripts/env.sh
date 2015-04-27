#!/bin/bash
#echo "Loading ROOT 5.34.14 x86_64-sl6-gcc46-opt ..."
#source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.14/x86_64-slc6-gcc46-opt/root/bin/thisroot.sh
echo "Loading ROOT 5.34.30 x86_64-sl6-gcc48-opt ..."
source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.30/x86_64-slc6-gcc48-opt/root/bin/thisroot.sh
echo "Loading python py27 ... "
source /afs/cern.ch/user/f/fnewson/.virtualenvs/py27/bin/activate
echo "Loading clang ... "
source ~/envfiles/clang.env.sh
