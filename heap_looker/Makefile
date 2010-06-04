CFLAGS = -W -Wall -Wextra  -Wextra  -Wall  -Waddress  -Waggregate-return  -Warray-bounds -Wno-attributes -Wno-builtin-macro-redefined  -Wcast-align  -Wcast-qual -Wconversion  -Wempty-body  -Wenum-compare   -Wignored-qualifiers -Wimplicit  -Winit-self  -Wno-int-to-pointer-cast  -Wmissing-noreturn  -Wnonnull  -Wno-overflow -Woverlength-strings   -Wpointer-arith -Wno-pointer-to-int-cast  -Wreturn-type  -Wsequence-point  -Wshadow -Wsign-compare  -Wsign-conversion   -Wstrict-aliasing  -Wstrict-overflow  -Wuninitialized  -Wno-pragmas   -Wunused -Wvariadic-macros  -Wwrite-strings

INCLUDE_PATH= -I. -I..
LIBRARY_PATH= -L. -L../dbg_print -ldbgprint -ldl


lib:
	$(CC) -DHEAP_LOOKER_TRACEON $(INCLUDE_PATH) -fpic -c heap_looker.c  $(CFLAGS)
	$(CC)  heap_looker.o $(LIBRARY_PATH) -o libheaplooker.so  -shared


test: lib
	$(CC)  -c f1.c main.c
	$(CC) -fpic  $(LIBRARY_PATH) f1.o main.o -L. -lheaplooker  -o main 


clean:
	rm -f *.o *.so core *~ main
