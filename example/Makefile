
cpp_files := $(wildcard *.cpp)
out_files := $(cpp_files:%.cpp=%.out)

cxxoptions := -Wall -std=c++11 -fdiagnostics-color=always -I../include

all: $(out_files)

%.out : %.cpp
	c++ -o $@ $(cxxoptions) $<

clean: 
	rm -f *.out

