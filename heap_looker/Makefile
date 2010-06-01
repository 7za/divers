lib:
	gcc -DHEAP_LOOKER_TRACE_ON -fpic -c heap_looker.c
	ld heap_looker.o -soname libheaplooker.so -o libheaplooker.so -shared -ggdb


test: lib
	gcc -c f1.c main.c
	gcc f1.o main.o -L. -lheaplooker -o main -ldl -ggdb 

clean:
	rm -f *.o *.so main *~
