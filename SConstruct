#Global environment
import os
import subprocess

#LIBRARY LOCATIONS
root_dir = os.environ['SC_ROOT_DIR']

boost_dir =  os.environ['SC_BOOST_DIR']

boost_name = os.environ['SC_BOOST_NAME']

yaml_dir = os.environ['SC_YAML_DIR']

event_dir = os.environ['SC_EVENT_DIR']

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
