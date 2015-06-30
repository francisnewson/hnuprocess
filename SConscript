import os

Import( 'env' )

env.Append( CPPFLAGS = ['-std=c++0x',
'-Wall',
'-Wno-error=unused-variable',
'-Wno-error=deprecated-declarations',
'-Wno-error=sign-compare',
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
'k3pi',
'muons',
'dch',
'akl',
'shuffle'
]

#Call Sconscript for each library
SConscript( [os.path.join( lib, 'SConscript') 
    for lib in my_libs], exports = 'env' )
