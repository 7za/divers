all:
	gcc -c -fpic lib.c
	gcc lib.o -o liblib.so -shared
	gcc dsl_elf.c -o dsl_elf -Wall -Wextra -W -ldl 

clean:
	rm -f liblib.so dsl_elf *.o
