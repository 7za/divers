CFLAGS = -W -Wall -Wextra 


lib:
	$(CC) -fpic *.c -o libioloop.so -shared $(CFLAGS) -ggdb

test:	lib
	$(CC) test.c -L. -lioloop -o test -lpthread -ggdb

clean:
	rm -f *.o *.so  *~ test


.PHONY: clean lib
