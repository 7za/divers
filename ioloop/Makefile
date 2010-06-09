CFLAGS = -W -Wall -Wextra 


lib:
	$(CC) -fpic *.c -o libioloop.so -shared $(CFLAGS) -ggdb

test:	lib
	$(CC) test.c -L. -lioloop -o test -ggdb

clean:
	rm -f *.o *.so  *~ test
	ct-ng clean


.PHONY: clean lib
