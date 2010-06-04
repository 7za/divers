#ifndef DBGPRINT_H
#define DBGPRINT_H

#include <stdio.h>
#include <stdarg.h>


extern int dbg_printf(FILE *where, const char *str, ...)
__attribute__((format(printf, 2, 3)));    



#endif

