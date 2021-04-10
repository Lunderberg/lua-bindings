Import('env')

usage = env.SConscript('SConscript_lib_only', 'env')
env.Append(**usage)

env.UnitTestDir('tests')
