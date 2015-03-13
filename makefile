CPP=clang++
LLVM_CPP_FLAGS=-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -I$(shell llvm-config --includedir)
COMMON_CPPFLAGS=-std=c++14 $(LLVM_CPP_FLAGS)
DEBUG_CPPFLAGS=$(COMMON_CPPFLAGS) -Wall -g -fcolor-diagnostics
RELEASE_CPPFLAGS=$(COMMON_CPPFLAGS) -O3 -DNDEBUG

LLVM_LD_FLAGS=-rdynamic $(shell llvm-config --ldflags)
LLVM_LIBS=-L$(shell llvm-config --libdir) $(shell llvm-config --libs core native jit bitwriter) $(shell llvm-config --system-libs)

COMMON_LDFLAGS=$(LLVM_LD_FLAGS)
COMMON_LIBS=-lboost_system -lboost_filesystem $(LLVM_LIBS)
DEBUG_LDFLAGS=-g $(COMMON_LDFLAGS)
DEBUG_LIBS=-lboost_unit_test_framework $(COMMON_LIBS)
RELEASE_LDFLAGS=-O3 $(COMMON_LDFLAGS)
RELEASE_LIBS=$(COMMON_LIBS)

ALL_SRCS=$(wildcard src/*.cpp)
ALL_TESTS=$(wildcard test/*.cpp)




# build library, create build directories and build executable
full-build:
	make build-dirs
	echo "building executable..."
	make -j build/release/bin
	echo "building executable done"
.SILENT: full-build

clean:
	rm -r build test-build || true
	make build-dirs

# all required directories for builds
build-dirs:
	mkdir -p build
	mkdir -p build/dep
	mkdir -p build/debug
	mkdir -p build/debug/obj
	mkdir -p build/release
	mkdir -p build/release/obj
	mkdir -p test-build
	mkdir -p test-build/obj
	mkdir -p test-build/dep
	mkdir -p test-build/bin
	mkdir -p test-build/output

install:
	cp build/debug/bin /usr/local/bin/asm-lisp
	chmod +x /usr/local/bin/asm-lisp

# convenience targets
debug: build/debug/bin
release: build/release/bin
debugger: build/debug/bin
	gdb build/debug/bin

# include generated dependency files
include $(wildcard build/dep/*.dep)
include $(wildcard test-build/dep/*.dep)

# dependency files
.SECONDARY: $(patsubst src/%.cpp,build/dep/%.dep,$(ALL_SRCS))
build/dep/%.dep: src/%.cpp
	$(CPP) $(COMMON_CPPFLAGS) -MM -MT $@ $< -MF $@

# debug build
DEBUG_OBJS=$(patsubst src/%.cpp,build/debug/obj/%.o,$(ALL_SRCS))
.SECONDARY: $(DEBUG_OBJS) 
build/debug/obj/%.o: build/dep/%.dep
	$(CPP) $(DEBUG_CPPFLAGS) -c $(patsubst build/debug/obj/%.o,src/%.cpp,$@) -o $@

build/debug/bin: $(patsubst src/%.cpp,build/debug/obj/%.o,$(ALL_SRCS))
	$(CPP) $(DEBUG_LDFLAGS) -o $@ $^ $(DEBUG_LIBS)

# release build
RELEASE_OBJS=$(patsubst src/%.cpp,build/release/obj/%.o,$(ALL_SRCS))
.SECONDARY: $(RELEASE_OBJS)
build/release/obj/%.o: build/dep/%.dep
	$(CPP) $(RELEASE_CPPFLAGS) -c $(patsubst build/release/obj/%.o,src/%.cpp,$@) -o $@

build/release/bin: $(patsubst src/%.cpp,build/release/obj/%.o,$(ALL_SRCS))
	$(CPP) $(RELEASE_LDFLAGS) -o $@ $^ $(RELEASE_LIBS)

# tests
# dependency files for tests
.SECONDARY: $(patsubst test/%.cpp,test-build/dep/%.dep,$(ALL_TESTS))
test-build/dep/%.dep: test/%.cpp
	$(CPP) $(COMMON_CPPFLAGS) -Isrc -MM -MT $@ $< -MF $@

# obj files for tests
.SECONDARY: $(patsubst test/%.cpp,test-build/obj/%.o,$(ALL_TESTS))
test-build/obj/%.o: test-build/dep/%.dep
	$(CPP) $(DEBUG_CPPFLAGS) -Isrc -c $(patsubst test-build/obj/%.o,test/%.cpp,$@) -o $@

# binaries for tests
.SECONDARY: $(patsubst test/%.cpp,test-build/bin/%,$(ALL_TESTS))
test-build/bin/%: $(filter-out %main.o,$(DEBUG_OBJS)) test-build/obj/%.o
	$(CPP) $(DEBUG_LDFLAGS) -o $@ $^ $(DEBUG_LIBS)

# output of tests
.SECONDARY: $(patsubst test/%.cpp,test-build/output/%,$(ALL_TESTS))
test-build/output/%: test-build/bin/%
	valgrind -q $^ > $@ 2>&1 || true

test-%: test-build/output/%
	cat $^

.SILENT: full-test
full-test: $(patsubst test/%.cpp,test-build/output/%, $(ALL_TESTS))
	for output in $^; \
	do \
	echo "cat $${output}"; \
		cat $$output; \
		echo ""; \
		echo ""; \
	done
	
