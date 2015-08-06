
cpp_files := $(wildcard *.cpp)
obj_files := $(cpp_files:%.cpp=%.o)

cxxoptions := -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused -fdiagnostics-color=always -I../include -I../mgbase/include

all: $(obj_files)

%.o : %.cpp
	c++ -O3 -S $(cxxoptions) $<

clean: 
	rm -f *.o

