#clang setup
import subprocess
def SetClang(env):
    env['CXX'] = 'clang++'
    env['CC'] = 'clang'

    path = ['/afs/cern.ch/user/f/fnewson/work/programs/llvm-build/Release+Asserts/bin',
            '/afs/cern.ch/sw/lcg/contrib/gcc/4.8.1/x86_64-slc6-gcc48-opt/bin', 
            '/usr/local/bin', '/bin', '/usr/bin', '/usr/local/sbin', '/usr/sbin' ,'/sbin']

    ld_library_path = ['/afs/cern.ch/sw/lcg/contrib/gcc/4.8.1/x86_64-slc6-gcc48-opt/lib64','/lib64']

    env.Append( ENV = 
            { 'PATH' : ":".join(path), 
                'LD_LIBRARY_PATH' : ':'.join(ld_library_path), 
                'TERM': 'xterm' } )

    env.Append( CXXFLAGS = ['-fcolor-diagnostics'] )

AddMethod( Environment, SetClang )

def SetBhamGCC( env ):
    path = ['/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/bin']
    env['CXX'] = '/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/bin/g++'
    env['CC'] = '/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/bin/gcc'
    ld_library_path = ['/home/fon/work/hnu/code/root/root-5.34.32/root/lib',
            '/home/fon/work/hnu/code/gcc/version/gcc-4.8.2/lib64']
    env.AppendENVPath(  'PATH' , ":".join(path) )
    env.AppendENVPath( 'LD_LIBRARY_PATH' , ':'.join(ld_library_path) )
    env.AppendENVPath( 'TERM', 'xterm'  )

AddMethod( Environment, SetBhamGCC )

def SetBhamBoost( env, boost_home, boost_name ):
    boost_libdir = os.path.join( boost_home , 'stage/lib' )
    boost_incdir = os.path.join( boost_home )
    env.Append( CXXFLAGS = '-isystem ' + boost_incdir )
    env.Append( LIBPATH = boost_libdir )
    env.Append( RPATH = [boost_libdir] )

def SetCERNBoost( env, boost_home, boost_name ):
    boost_libdir = os.path.join( boost_home , 'lib' )
    boost_incdir = os.path.join( boost_home, os.path.join('include', boost_name ) )
    env.Append( CXXFLAGS = '-isystem ' + boost_incdir )
    env.Append( LIBPATH = boost_libdir )
    env.Append( RPATH = [boost_libdir] )

def SetBoost( env, boost_home, boost_name, machine ):
    if machine == 'bham':
        SetBhamBoost( env, boost_home, boost_name )
    else:
        SetCERNBoost( env, boost_home, boost_name )

AddMethod( Environment, SetBoost )

def SetRoot( env, root_home ):
    root_config = os.path.join( root_home, 'bin', 'root-config' )
    root_libdir = subprocess.check_output( [root_config, '--libdir'])
    root_incdir = subprocess.check_output( [root_config, '--incdir'])
    #env.Append( CPPPATH = [root_incdir.rstrip()] )
    env.Append( CXXFLAGS = '-isystem ' + root_incdir.rstrip() + ' ' )
    env.Append( LIBPATH = [root_libdir.rstrip()] )
    env.Append( LIBS = ['Core','Cint','RIO','Net','Hist','Graf','Graf3d','Gpad',
        'Tree','Rint','Postscript','Matrix','Physics','MathCore','Thread','pthread','m','dl'] )

AddMethod( Environment, SetRoot )


def SetYaml( env, yaml_home, machine ):
    env.Append( CPPPATH = [os.path.join( yaml_home, 'include' )] )
    if machine == 'bham':
        env.Append( LIBPATH = os.path.join( yaml_home, 'build' ) )
        env.Append( RPATH = [os.path.join( yaml_home, 'build' )] )
    else:
        env.Append( LIBPATH = os.path.join( yaml_home, 'process_build' ) )
        env.Append( RPATH = [os.path.join( yaml_home, 'process_build' )] )

    env.Append( LIBS = 'yaml-cpp' )

AddMethod( Environment, SetYaml )

def AddLibs( env, libs ):
    env.Append( CPPPATH = [ os.path.join('../', lib, 'inc' ) for lib in libs] )
    env.Append( LIBPATH =  [('../'+lib ) for lib in libs] )
    env.Append( LIBS =  libs )
    env.Append( RPATH =  [ os.path.join( env['variantDir'], lib ) for lib in libs] )

AddMethod( Environment, AddLibs )

def AddEvent( env, remote, local ):
    env.Append( CPPPATH = os.path.join( remote, 'inc' ) )
    #env.Append( LIBPATH =  '../'+local  )
    env.Append( LIBPATH = os.path.join( remote  ) )
    env.Append( LIBS = 'event' )
    #env.Append( RPATH =  os.path.join( env['variantDir'], 'event' ) )
    env.Append( RPATH =  remote )

AddMethod( Environment, AddEvent )
