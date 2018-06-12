.PHONY=all tests clean obj update
CXX=g++
CC=gcc

GMATCH=$(findstring g++,$(CXX))

CLHASH_CHECKOUT = "&& git checkout master"
WARNINGS=-Wall -Wextra -Wno-char-subscripts \
		 -Wpointer-arith -Wwrite-strings -Wdisabled-optimization \
		 -Wformat -Wcast-align -Wno-unused-function -Wno-unused-parameter \
		 -pedantic -DUSE_PDQSORT -Wunused-variable \
		-Wduplicated-branches -Wdangling-else  -Wno-class-memaccess# -Wconversion
ifndef EXTRA
	EXTRA:=
endif
ifndef INCPLUS
	INCPLUS:=
endif
ifndef EXTRA_LD
	EXTRA_LD:=
endif
DBG:=
OS:=$(shell uname)
FLAGS=

OPT_MINUS_OPENMP= -O3 -funroll-loops\
	  -pipe -fno-strict-aliasing -march=native -mpclmul $(FLAGS) $(EXTRA)
OPT=$(OPT_MINUS_OPENMP) -fopenmp
XXFLAGS=-fno-rtti
CXXFLAGS=$(OPT) $(XXFLAGS) -std=c++17 $(WARNINGS)
CXXFLAGS_MINUS_OPENMP=$(OPT_MINUS_OPENMP) $(XXFLAGS) -std=c++1z $(WARNINGS) -Wno-cast-align -Wno-gnu-zero-variadic-macro-arguments
CCFLAGS=$(OPT) $(CFLAGS) -std=c11 $(WARNINGS)
LIB=-lz
LD=-L. $(EXTRA_LD)

ifneq (,$(findstring g++,$(CXX)))
	ifeq ($(shell uname),Darwin)
		ifeq (,$(findstring clang,$(CXX)))
			POPCNT_CXX:=clang
		else
			POPCNT_CXX:=$(CXX)
		endif
	else
		POPCNT_CXX:=$(CXX)
	endif
endif

OBJS= $(patsubst %.cpp,%.o,$(wildcard src/*.cpp)) clhash/clhash.o

ZSTD_INCLUDE_DIRS=bonsai/zstd/zlibWrapper bonsai/zstd/lib/common bonsai/zstd/lib
ZSTD_INCLUDE=$(patsubst %,-I%,$(ZSTD_INCLUDE_DIRS))
ZFLAGS=-DZWRAP_USE_ZSTD=1
ZCOMPILE_FLAGS= $(ZFLAGS) -lzstd
ZW_OBJS=$(patsubst %.c,%.o,$(wildcard bonsai/zstd/zlibWrapper/*.c)) libzstd.a
ALL_ZOBJS=$(ZOBJS) $(ZW_OBJS) bonsai/bonsai/clhash.o bonsai/klib/kthread.o
INCLUDE=-Ibonsai/clhash/include -I.  -Ibonsai/libpopcnt -Iinclude -Ibonsai/circularqueue $(ZSTD_INCLUDE) $(INCPLUS) -Ibonsai/hll -Ibonsai/hll/vec -Ibonsai/pdqsort -Ibonsai -Ibonsai/bonsai/include/ -Ihll/

EX=$(patsubst src/%.cpp,%,$(wildcard src/*.cpp))
Z_EX=$(patsubst src/%.cpp,%_z,$(wildcard src/*.cpp))
D_EX=$(patsubst src/%.cpp,%_d,$(wildcard src/*.cpp))


all: $(EX)

z: $(Z_EX)
d: $(D_EX)

update:
	git submodule update --init --remote --recursive . && cd bonsai && git checkout master && git pull && make update && cd .. && cd distmat && git checkout master && git pull && cd ..

libzstd.a:
	+cd bonsai/bonsai && make libzstd.a && cp libzstd.a ../../

klib/kstring.o:
	cd klib && make kstring.o && cd ..

clhash/clhash.o:
	cd bonsai/bonsai && make clhash.o && cd ../..

DEPS=bonsai/hll/cbf.h bonsai/hll/bf.h bonsai/hll/hll.h

test/%.o: test/%.cpp
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) -c $< -o $@ $(LIB)

test/%.zo: test/%.cpp
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) -c $< -o $@ $(ZCOMPILE_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) -DNDEBUG -c $< -o $@ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LD) -DNDEBUG -c $< -o $@ $(LIB)

%.do: %.cpp
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) -c $< -o $@ $(LIB)

%: %.cpp clhash/clhash.o
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(OBJ) clhash/clhash.o -DNDEBUG $< -o $@ $(LIB)

%_d: %.cpp clhash/clhash.o
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(OBJ) clhash/clhash.o $< -o $@ $(LIB)

%_s: $(OBJ) src/%.cpp libzstd.a $(DEPS)
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(OBJ) -static-libstdc++ -static-libgcc -DNDEBUG $< -o $@ $(LIB)

zobj: $(ALL_ZOBJS)

%_z: src/%.cpp $(ALL_ZOBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(ALL_ZOBJS) -DNDEBUG $< -o $@ $(ZCOMPILE_FLAGS) $(LIB)
%_d: src/%.cpp $(ALL_ZOBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(ALL_ZOBJS) -g $< -o $@ $(ZCOMPILE_FLAGS) $(LIB)
%_di: src/%.cpp $(ALL_ZOBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(ALL_ZOBJS) -g -fno-inline -O1 $< -o $@ $(ZCOMPILE_FLAGS) $(LIB)
#
#
#%_sz: src/%.cpp $(ALL_ZOBJS)
#	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(ALL_ZOBJS) -static-libstdc++ -static-libgcc -DNDEBUG $< -o $@ $(ZCOMPILE_FLAGS) $(LIB)
#
#%_d: src/%.cpp $(DOBJS)
#	$(CXX) $(CXXFLAGS) $(DBG) $(INCLUDE) $(LD) $(DOBJS) $< -o $@ $(LIB)

clean:
	rm -f $(EX) $(Z_EX) $(D_EX) libzstd.a $(OBJ)
mostlyclean: clean
