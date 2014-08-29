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

env.Decider('MD5-timestamp')

my_libs = [ 
'reco',
'tools',
'corrections',
'apps',
'tracking',
'conditions',
'clusters',
'kinfo',
'km2',
'k2pi',
'muons',
'akl',
]

#Call Sconscript for each library
SConscript( [os.path.join( lib, 'SConscript') 
    for lib in my_libs], exports = 'env' )
