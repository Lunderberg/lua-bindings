env = Environment()

env.Append(CXXFLAGS=['-std=c++11'])
env.Append(CCFLAGS=['-Wall','-Wextra','-g'])

env.Append(CPPPATH=['lua-5.3.0/src'])
lua = env.StaticLibrary('lua-5.3.0/lua',Glob('lua-5.3.0/src/*.c'),
                        CPPDEFINES=['LUA_COMPAT_5_2','LUA_USE_LINUX'])
env.Append(LIBS=['dl'])


env.Append(CPPPATH=['include'])
env.Program('main',['main.cc',Glob('src/*.cc'), lua])
