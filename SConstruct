import os

env = Environment()

# More readable output
if not ARGUMENTS.get('VERBOSE'):
    env['CXXCOMSTR'] = 'Compiling C++ object $TARGETS'
    env['CCCOMSTR'] = 'Compiling C object $TARGETS'
    env['ARCOMSTR'] = 'Packing static library $TARGETS'
    env['RANLIBCOMSTR'] = 'Indexing static library $TARGETS'
    env['SHCCCOMSTR'] = 'Compiling shared C object $TARGETS'
    env['SHCXXCOMSTR'] = 'Compiling shared C++ object $TARGETS'
    env['LINKCOMSTR'] = 'Linking $TARGETS'
    env['SHLINKCOMSTR'] = 'Linking shared $TARGETS'

# Command-line adjustments
optimize = ARGUMENTS.get('OPTIMIZE')
if not optimize:
    optimize = 3
if optimize!='0':
    env.Append(CPPFLAGS=['-O{}'.format(optimize)])

if ARGUMENTS.get('RELEASE'):
    env.Append(CPPDEFINES=['NDEBUG'])
    env.Append(CPPFLAGS=['-s'])
else:
    env.Append(CPPFLAGS=['-g'])

if ARGUMENTS.get('PROFILE'):
    env.Append(CPPFLAGS=['-pg','-g'])
    env.Append(LINKFLAGS=['-pg'])

env.Append(CXXFLAGS=['-std=c++11'])
env.Append(CCFLAGS=['-Wall','-Wextra','-g'])

env.Append(CPPPATH=[Dir('include')])
lua_bindings = env.SConscript('SConscript','env')
env.Append(LIBS=[lua_bindings, 'dl'])

env.Program('main','main.cc')

env.SConscript('tests/SConscript',
               exports=['env','lua_bindings'])
