#Global environment
import os
import subprocess

#LIBRARY LOCATIONS
root_dir = os.environ['SC_ROOT_DIR']

boost_dir =  os.environ['SC_BOOST_DIR']

boost_name = os.environ['SC_BOOST_NAME']

yaml_dir = os.environ['SC_YAML_DIR']

event_dir = os.environ['SC_EVENT_DIR']

machine = os.environ.get('MACHINE')

Export ( 'root_dir boost_dir boost_name yaml_dir event_dir machine' )

env_default = Environment()


if machine == 'bham':
    env = env_default.Clone()
    env['variantDir'] = 'build_bham'
    env.Append( CPPFLAGS = ['-O3'] )

    env.SetBhamGCC()

    VariantDir(env['variantDir'], '.' )
    SConscript( os.path.join( env['variantDir'], 'SConscript' ),
            exports = 'env')
    Clean( ',', env['variantDir'] )

    print( "Defining build_bham")
    print ( 'path is: {0}'.format( env['ENV']['PATH'] ) )
    print ( 'ld_library_path is : {0}'.format( env['ENV']['LD_LIBRARY_PATH'] ) )
else:
    env = env_default.Clone()
    env['variantDir'] = 'build_O3'
    env.Append( CPPFLAGS = ['-O3', '-ggdb'] )
    env.Append( CPPFLAGS = ['-std=c++0x', '-Wno-error=unused-private-field', ] )
    env.SetClang()
    VariantDir(env['variantDir'], '.' )
    SConscript( os.path.join( env['variantDir'], 'SConscript' ),
            exports = 'env')
    Clean( ',', env['variantDir'] )

    env = env_default.Clone()
    env['variantDir'] = 'build_O0'
    env.SetClang()
    env.Append( CPPFLAGS = ['-O0', '-ggdb'] )
    env.Append( CPPFLAGS = ['-std=c++0x', '-Wno-error=unused-private-field', ] )
    VariantDir(env['variantDir'], '.' )
    SConscript( os.path.join( env['variantDir'], 'SConscript' ),
            exports = 'env')
    Clean( ',', env['variantDir'] )
