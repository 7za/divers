plugin:
	gcc -fPIC -c plugin.c
	ld plugin.o -o libplugintest.so -soname libplugintest.so -shared

test: plugin
	gcc main.c -o main -L.. -ldsoplugin -ldl
