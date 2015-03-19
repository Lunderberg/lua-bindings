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
env.Append(CCFLAGS=['-O3'])

env.Append(CPPPATH=['lua-5.3.0/src'])
lua = env.StaticLibrary('lua-5.3.0/lua',Glob('lua-5.3.0/src/*.c'),
                        CPPDEFINES=['LUA_COMPAT_5_2','LUA_USE_LINUX'])
env.Append(LIBS=['dl'])

env.Append(CPPPATH=['include'])
env.Program('main',['main.cc',Glob('src/*.cc'), lua])

env.SConscript('tests/SConscript',
               exports=['env','lua'])
