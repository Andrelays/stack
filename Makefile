#------------------------------------------------------------------------------
#
# Makefile, approach 3
# * implicit rule for first dependency not for all
# * dependency gather filtered
# * pattern substitution
#
#------------------------------------------------------------------------------
#
# This file is licensed after LGPL v3
# Look at: https://www.gnu.org/licenses/lgpl-3.0.en.html for details
#
#------------------------------------------------------------------------------

# make OUT_O_DIR=debug CXX=clang CXXFLAGS="-g -O0" -f makefile.v3.mak
# make -f makefile.v3.mak
# touch include/cache.h
# make -f makefile.v3.mak
# make OUT_O_DIR=debug -f makefile.v3.mak
# make -f makefile.v3.mak testrun
# make -f makefile.v3.mak testrun -j4
# make OUT_O_DIR=debug -f makefile.v3.mak testrun -j4
# make OUT_O_DIR=debug -f makefile.v3.mak clean
# make -f makefile.v3.mak clean

ifeq ($(origin CXX),default)
  CXX = g++
endif

CXXFLAGS ?= -D DEBUG_OUTPUT_TREE_DUMP -Wno-pointer-arith -D DEBUG -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
OUT_O_DIR ?= build
COMMONINC = -I./include -I./libraries/utilities
SRC = ./source
LDFLAGS = ./libraries/utilities/libfile.a
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

override CXXFLAGS += $(COMMONINC)

CSRC = source/main.cpp source/stack.cpp

# reproducing source tree in object tree
COBJ := $(addprefix $(OUT_O_DIR)/,$(CSRC:.cpp=.o))
DEPS = $(COBJ:.o=.d)

.PHONY: all
all: $(OUT_O_DIR)/release

$(OUT_O_DIR)/release: $(COBJ) $(LDFLAGS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $^ -o $@

# static pattern rule to not redefine generic one
$(COBJ) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CXX) -E $(CXXFLAGS) $< -MM -MT $(@:.d=.o) > $@

.PHONY: clean
clean:
	rm -rf $(COBJ) $(DEPS) $(OUT_O_DIR)/release $(OUT_O_DIR)/*.log

# targets which we have no need to recollect deps
NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(DEPS)
endif
