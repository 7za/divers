INCLUDE_PATH = -I.. -I.
LIBRARY_PATH= -L../sym_resolv -lsym_resolver -L../dbg_print -ldbgprint
CFLAGS = -W -Wall -Wextra  -Wextra  -Wall  -Waddress  -Waggregate-return  -Warray-bounds -Wno-attributes -Wno-builtin-macro-redefined -Wc++-compat  -Wcast-align  -Wcast-qual -Wchar-subscripts -Wclobbered  -Wcomment -Wconversion -Wcoverage-mismatch  -Wno-deprecated -Wno-deprecated-declarations -Wdisabled-optimization -Wno-div-by-zero -Wempty-body  -Wenum-compare -Wno-endif-labels  -Wfatal-errors  -Wfloat-equal  -Wformat  -Wformat=2 -Wno-format-contains-nul -Wno-format-extra-args -Wformat-nonliteral -Wformat-security  -Wformat-y2k  -Wjump-misses-init -Wignored-qualifiers -Wimplicit -Wimplicit-function-declaration  -Wimplicit-int -Winit-self  -Winline -Wno-int-to-pointer-cast  -Winvalid-pch   -Wunsafe-loop-optimizations -Wlogical-op  -Wmain  -Wmissing-braces  -Wmissing-field-initializers -Wmissing-format-attribute  -Wmissing-include-dirs -Wmissing-noreturn  -Wno-mudflap -Wno-multichar  -Wnonnull  -Wno-overflow -Woverlength-strings  -Wpacked  -Wpacked-bitfield-compat  -Wpadded -Wparentheses   -Wpointer-arith -Wno-pointer-to-int-cast  -Wreturn-type  -Wsequence-point  -Wshadow -Wsign-compare  -Wsign-conversion  -Wstack-protector -Wstrict-aliasing  -Wstrict-overflow  -Wswitch  -Wswitch-default  -Wswitch-enum -Wsync-nand -Wsystem-headers  -Wtrigraphs  -Wtype-limits  -Wundef  -Wuninitialized -Wunknown-pragmas  -Wno-pragmas -Wunsuffixed-float-constants  -Wunused -Wunused-function -Wunused-label  -Wunused-parameter -Wno-unused-result -Wunused-value  -Wunused-variable -Wvariadic-macros -Wvla -Wvolatile-register-var  -Wwrite-strings


test: sig_look.c 
	gcc  *.c $(LIBRARY_PATH) $(INCLUDE_PATH) $(CFLAGS) -o test


clean:
	rm -f *.o *.so test *~


.PHONY: clean test
