lib:
	$(CC) -fpic *.c -o libsym_resolver.so -shared -ldl -Wextra -Wall -W

test: lib
	$(CC) test.c -I.  -L. -lsym_resolver  -o test -ggdb

clean:
	rm -f *.o *.so test *~
