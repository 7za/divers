INCLUDE_PATH = -I.. -I.
#LIBRARY_PATH = -L../trace -ldbgprint -ldl
LIBRARY_PATH= -L../elf -lsym_resolver
CFLAGS = -W -Wall -Wextra


test: sig_look.c 
	gcc  *.c $(LIBRARY_PATH) $(INCLUDE_PATH) $(CFLAGS) -o test


clean:
	rm -f *.o *.so test *~
