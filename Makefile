CXX=clang++
CXXFLAGS=-I. -std=c++2b -g

DEPS = context.hpp interpreter.hpp symbol.hpp dictionary.hpp operators.hpp types.hpp enable_shared_from_base.hpp parser.hpp value.hpp

OBJ = context.o symbol.o value.o test.o parser.o interpreter.o operators.o

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

test: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm $(OBJ) test
