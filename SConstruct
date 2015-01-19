#Global environment
import os
import subprocess

#LIBRARY LOCATIONS
root_dir = ( '/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.14/'
+'x86_64-slc6-gcc46-opt/root' )

boost_dir =  ( '/afs/cern.ch/sw/lcg/external/Boost/'
        '1.55.0_python2.7/x86_64-slc6-gcc47-opt'
        )

boost_name = 'boost-1_55'

yaml_dir = ( '/afs/cern.ch/user/f/fnewson/work/programs/'
        +'yaml-cpp/yaml-cpp-0.5.1' )

event_dir = '/afs/cern.ch/user/f/fnewson/work/hnu/gopher/code/fneevent/build_sprocess/'

Export ( 'root_dir boost_dir boost_name yaml_dir event_dir' )

env_default = Environment()

env = env_default.Clone()
env['variantDir'] = 'build_O3'
env.Append( CPPFLAGS = ['-O3', '-ggdb'] )
VariantDir(env['variantDir'], '.' )
SConscript( os.path.join( env['variantDir'], 'SConscript' ),
        exports = 'env')
Clean( ',', env['variantDir'] )

env = env_default.Clone()
env['variantDir'] = 'build_O0'
env.Append( CPPFLAGS = ['-O0', '-ggdb'] )
VariantDir(env['variantDir'], '.' )
SConscript( os.path.join( env['variantDir'], 'SConscript' ),
        exports = 'env')
Clean( ',', env['variantDir'] )
