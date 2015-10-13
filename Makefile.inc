# This file can be placed inside of a library directory to customize
# the behavior of that library. Each option, if left commented, will
# assumed its default value.

# The name of the library.
# Defaults to LIBNAME, where libLIBNAME is the directory.
LIBRARY_NAME = LuaBindings

# The flag that will be passed to the include the library in
# executables.
LIBRARY_FLAG = -l$(LIBRARY_NAME)

# The directories containing source files for the library.
LIBRARY_SRC_DIRS = src lua-5.3.0/src

# The directories containing include files for the library. These
# directories will be added to the include path for all files in the
# project.
LIBRARY_INCLUDE_DIRS = include lua-5.3.0/src

# The directories containing include files for the library.  These
# directories will be added to the include path only for files within
# this library
LIBRARY_PRIVATE_INCLUDE_DIRS =

# Compiler flag overrides for src files within this library.
ifeq ($(SYSTEM),linux)
  $(LIBRARY): ALL_CPPFLAGS += -DLUA_USE_LINUX
  $(LIBRARY): SHARED_LDLIBS += -Wl,-E -ldl -lreadline
endif
