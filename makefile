CPP=clang++
COMMON_CPPFLAGS=-std=c++1y
DEBUG_CPPFLAGS=$(COMMON_CPPFLAGS) -Wall -g 
RELEASE_CPPFLAGS=$(COMMON_CPPFLAGS) -O3

COMMON_LDFLAGS=
DEBUG_LDFLAGS=$(COMMON_LDFLAGS)
RELEASE_LDFLAGS=-O3 $(COMMON_LDFLAGS)


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

# convenience targets
debug: build/debug/bin
release: build/release/bin


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
	$(CPP) $(DEBUG_LDFLAGS) -o $@ $^

# release build
RELEASE_OBJS=$(patsubst src/%.cpp,build/release/obj/%.o,$(ALL_SRCS))
.SECONDARY: $(RELEASE_OBJS)
build/release/obj/%.o: build/dep/%.dep
	$(CPP) $(RELEASE_CPPFLAGS) -c $(patsubst build/release/obj/%.o,src/%.cpp,$@) -o $@

build/release/bin: $(patsubst src/%.cpp,build/release/obj/%.o,$(ALL_SRCS))
	$(CPP) $(RELEASE_LDFLAGS) -o $@ $^

# tests
# dependency files for tests
.SECONDARY: $(patsubst test/%.cpp,test-build/dep/%.dep,$(ALL_TESTS))
test-build/dep/%.dep: test/%.cpp
	$(CPP) $(COMMON_CPPFLAGS) -Isrc -MM -MT $@ $< -MF $@

# obj files for tests
.SECONDARY: $(patsubst test/%.cpp,test-build/obj/%.o,$(ALL_TESTS))
test-build/obj/%.o: test-build/dep/%.dep
	$(CPP) $(DEBUG_CPPFLAGS) -Isrc -c $(patsubst test-build/obj/%.o,test/%.cpp,$@) -o $@

# binaries fo tests
.SECONARY: $(patsubst test/%.cpp,test-build/%,$(ALL_TESTS))
test-build/%: $(filter-out %main.o,$(DEBUG_OBJS)) test-build/obj/%.o
	$(CPP) $(DEBUG_LDFLAGS) -o $@ $^

test-%: test-build/%
	./$^

full-test: $(patsubst test/%.cpp,test-%,$(ALL_TESTS))
