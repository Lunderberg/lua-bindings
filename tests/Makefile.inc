EXE_SRC_FILES = run_tests.cc
EXE_INCLUDE_DIRS = gtest-1.7.0/include gtest-1.7.0
EXTRA_SRC_FILES = $(patsubst $(CURDIR)/%,%,$(filter-out $(EXE_SRC_FILES),$(wildcard $(CURDIR)/*.cc))) gtest-1.7.0/src/gtest-all.cc

ALL_CPPFLAGS += -pthread
ALL_LDLIBS += -pthread
