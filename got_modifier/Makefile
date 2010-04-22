parasite:
	gcc -fPIC -c libtest.c -nostdlib
	ld -shared -o libtest.so.1.0 libtest.o

lop_lib:
	gcc -fPIC -c linux_obj_parser.c
	ld linux_obj_parser.o -o liblop.so -soname liblop.so -shared

injector_lib: 
	gcc -fPIC -c process_injector.c
	ld process_injector.o -o libprocess_injector.so -soname libprocess_injector.so -shared 

lib: clean lop_lib injector_lib

hijack: clean lib
	gcc map_parser.c -L. -llop -lprocess_injector -o map_parser -ggdb 

clean:
	rm -f *.o *.so
