import os

Import( 'env' )

env.SetClang()
env.Append( CPPFLAGS = ['-std=c++0x',
'-Werror',
'-Wall',
'-Wno-error=unused-private-field',
'-Wno-error=unused-variable',
'-Wno-error=deprecated-declarations'
] )

my_libs = [
'tools', 
'subscriber', 
#'event',
'analyses',
'factory',
'reco', 
'apps', 
'parse',
'shuffle',
'testapps', 
'unit', 
'tmva'
]

#Call Sconscript for each library
SConscript( [os.path.join( lib, 'SConscript') 
    for lib in my_libs], exports = 'env' )
