CXX=clang++
CFLAGS=-I. -std=c++2b -g
DEPS = context.hpp symbol.hpp dictionary.hpp value.hpp enable_shared_from_base.hpp types.hpp
OBJ = context.o symbol.o value.o test.o

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	$(CXX) -o $@ $^ $(CFLAGS)

clean:
	rm $(OBJ) test
