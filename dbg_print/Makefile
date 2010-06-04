CFLAGS = -W -Wall -Wextra  -Wextra  -Wall  -Waddress  -Waggregate-return  -Warray-bounds -Wno-attributes -Wno-builtin-macro-redefined  -Wcast-align  -Wcast-qual -Wconversion  -Wempty-body  -Wenum-compare   -Wignored-qualifiers -Wimplicit  -Winit-self  -Wno-int-to-pointer-cast  -Wmissing-noreturn  -Wnonnull  -Wno-overflow -Woverlength-strings   -Wpointer-arith -Wno-pointer-to-int-cast  -Wreturn-type  -Wsequence-point  -Wshadow -Wsign-compare  -Wsign-conversion   -Wstrict-aliasing  -Wstrict-overflow  -Wuninitialized  -Wno-pragmas   -Wunused -Wvariadic-macros  -Wwrite-strings

lib:
	$(CC) -DDBG_PRINT_SHOW_TRACE -c dbgprint.c $(CFLAGS)
	$(LD) *.o -o libdbgprint.so -soname=libdbgprint.so -shared

test: lib
	$(CC) test.c -L. -ldbgprint -o test


clean:
	rm -f *.o *.so ./test *~


.PHONY:	clean lib test
