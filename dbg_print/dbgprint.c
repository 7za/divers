#include "dbgprint.h"

#ifdef DBG_PRINT_SHOW_TRACE
# include <stdio.h>
# include <stdarg.h>


__attribute__((format(printf, 2, 3)))
int dbg_printf(FILE *where, const char *str, ...)
{
    FILE *fp = where? where: stdout;
    int ret;
    va_list arg;
    va_start(arg, str);
    ret = vfprintf(fp, str, arg);
    va_end(arg);

    return ret;
}


#else

__attribute__((format(printf, 2, 3)))
int dbg_printf(FILE *where, const char *str, ...)
{
    return 0;
}

#endif
