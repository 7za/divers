lib:
	$(CC) -fPIC -W -Wall -Wextra -c dso_plugin.c
	$(CC) dso_plugin.o -o libdsoplugin.so  -shared

all: lib

clean:
	rm -f *.o *.so *~
