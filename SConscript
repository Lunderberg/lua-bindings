# Usage:
#   lua_bindings = env.SConscript('lua-binding/SConscript', 'env')
#   env.Append(CPPPATH = ['lua-bindings/include'])
#   env.Append(LIBS = [lua_bindings, 'dl'])


import SCons.Errors

Import('env')

if ('-std=c++11' not in env['CXXFLAGS'] and
    '-std=c++11' not in env['CCFLAGS']):
    raise SCons.Errors.UserError('lua_bindings must be compiled with C++11\n' +
                                 'Please add -std=c++11 to CCFLAGS or CXXFLAGS.')

#Dir() needed to make the include be relative to SConscript
env.Append(CPPPATH=[Dir('include')])
env.Append(CPPPATH=[Dir('lua-5.3.0/src')])

lua_bindings = env.StaticLibrary('lua-5.3.0/lua',[Glob('lua-5.3.0/src/*.cc'), Glob('src/*.cc')],
                                 CPPDEFINES=['LUA_COMPAT_5_2','LUA_USE_LINUX'])

Return('lua_bindings')
