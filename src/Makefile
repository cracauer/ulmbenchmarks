ALL = ulmbenchmarks-base

all: $(ALL)

exception-stacks.cc: make-exception-stacks
	./make-exception-stacks > $@.tmp
	mv $@.tmp $@

exception-stacks.o: exception-stacks.cc stuff.h
	$(CXX) -fno-inline $(CFLAGS) -O3 -c $<

foo.o: stuff.cc stuff.h
	$(CXX) -fno-inline $(CFLAGS) -O3 -c $<

ulmbenchmarks-base: ulmbenchmarks-base.cc exception-stacks.o stuff.h stuff.o
	c++ -Wall -O3 -o $@ exception-stacks.o stuff.o $< -lpthread -lm

clean:
	-rm ulmbenchmarks-base exception-stacks.cc exception-stacks.o stuff.o
