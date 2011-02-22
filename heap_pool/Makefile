heap_pool: clean
	gcc -fpic  heap_pool.c -o libheap_pool.so -lrt -Wall -W -Wextra -shared -ggdb
	objcopy --only-keep-debug libheap_pool.so libheap_pool.so.debug

test: heap_pool
	gcc main.c -L. -lheap_pool -o main -ggdb
clean:
	rm -f libheap_pool.so* main
