COMBOBS=comb_separation.o comb.o util.o graph.o
CC= gcc
CFLAGS=  -O3 -g -Wall -pedantic -Wno-long-long

comb: $(COMBOBS)
	  $(CC) $(CFLAGS) -Wl,-no_compact_unwind -o comb_separation $(COMBOBS) -lm -lpthread

clean:
	rm -f *.o comb

comb_separation.o: comb_separation.c util.h
comb.o: comb.c comb.h
util.o: util.c util.h
graph.o: graph.c graph.h
