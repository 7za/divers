lib:
	$(CC) -DHEAP_LOOKER_TRACEON -fpic -c heap_looker.c -W -Wall -Wextra
	$(LD)  heap_looker.o -soname libheaplooker.so -o libheaplooker.so -ldl -shared -ggdb


test: lib
	$(CC)  -c f1.c main.c
	$(CC) -fpic  f1.o main.o -L. -lheaplooker  -o main -ggdb


clean:
	rm -f *.o *.so core *~ main
