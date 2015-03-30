# Usage:
#   lua_bindings = env.SConscript('lua-binding/SConscript', 'env')
#   env.Append(CPPPATH = ['lua-bindings/include', 'lua-bindings/lua-5.3.0/src'])
#   env.Append(LIBS = [lua_bindings])


import SCons.Errors

Import('env')

if ('-std=c++11' not in env['CXXFLAGS'] and
    '-std=c++11' not in env['CCFLAGS'] and
    '-std=c++0x' not in env['CXXFLAGS'] and
    '-std=c++0x' not in env['CCFLAGS']):
    raise SCons.Errors.UserError('lua_bindings must be compiled with C++11\n' +
                                 'Please add -std=c++11 to CCFLAGS or CXXFLAGS.')

#Dir() needed to make the include be relative to SConscript
env.Append(CPPPATH=[Dir('.').abspath + '/include'])
env.Append(CPPPATH=[Dir('.').abspath + '/lua-5.3.0/src'])

lua_bindings = env.StaticLibrary('lua-5.3.0/lua',[Glob('lua-5.3.0/src/*.cc'), Glob('src/*.cc')])

Return('lua_bindings')
