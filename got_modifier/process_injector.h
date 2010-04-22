#ifndef PROCESS_INJECTOR_H
#define PROCESS_INJECTOR_H

#include <libelf.h>
#include <sys/types/h>
#include <unistd.h>

unsigned long
process_injector_inject(pid_t pid, Elf32_Addr text, char *sh, size_t shlen);

#endif
