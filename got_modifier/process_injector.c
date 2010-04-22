#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>


static void
process_injector_read_at(pid_t pid, char buff[], size_t l, Elf32_Addr base_addr)
{
  long *addr = (long*)base_addr;
  long *ptr  = (long*)buff;
  size_t i;
  uint8_t *debug;
  
  for( i = 0; i < l; i = i + sizeof(long), ++ptr, ++addr){
    *ptr = (long)ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
  }
}

static void
process_injector_write_at(pid_t pid,
                          char buffer[],
                          size_t l,
                          Elf32_Addr base_addr)
{
  long *ptr  = (long*)buffer;
  long *addr = (long*)base_addr;
  
  size_t i;

  for( i = 0; i < l; i = i + sizeof(long), ++ptr, ++addr){
    ptrace(PTRACE_POKETEXT, pid, addr, *ptr);
  }
}


static void displaysh(char *sh, size_t l)
{
  size_t i;
  printf("shellcode : ...\n");
  for( i = 0; i < l; ++i)
	  printf("0x%2x  ", *(uint8_t*)(sh + i));
  puts("\n----------");
}

long 
process_injector_load_dso(pid_t pid, Elf32_Addr text, char *sh, size_t shlen)
{
  long ret = 0;
  struct user_regs_struct reg, bckreg;
  char buffer[shlen + 1], buffer2[shlen + 1];
  ptrace (PTRACE_GETREGS, pid, NULL, &reg);
  bckreg = reg;

  process_injector_read_at(pid, buffer, shlen, text);

  process_injector_write_at(pid, sh, shlen, text);

  reg.eip = text + 2; /* kernel substract by 2 addr after being interrupt by ptrace*/
  ptrace (PTRACE_SETREGS, pid, NULL, &reg);
  ptrace (PTRACE_CONT,    pid, NULL, NULL);
  wait (NULL);

  ptrace (PTRACE_GETREGS, pid, NULL, &reg);
  ret = reg.eax;

  process_injector_write_at(pid, buffer, shlen, text);

  reg = bckreg;
  ptrace (PTRACE_SETREGS, pid, NULL, &reg);
  return ret;
}


long
process_injector_inject(pid_t pid, Elf32_Addr text, char *sh, size_t shlen)
{
  long retval = 0;
  int ret = ptrace (PTRACE_ATTACH, pid, NULL, NULL);
  if(ret){
    perror("ptrace");
    exit(1);
  }
  retval = process_injector_load_dso(pid, text, sh, shlen);
  ptrace(PTRACE_DETACH, pid, NULL, NULL);
  return retval;
}

bool
process_injector_check_for_dso(pid_t pid, char *dsoname)
{/*{{{*/
  FILE *fp;
  bool ret = false;
  char line[255];
  char procfilename[strlen("/proc/XXXXX/maps") + 1];
  snprintf(procfilename, sizeof(procfilename), "/proc/%d/maps", pid);
  fp = fopen(procfilename, "r+");
  if(fp == NULL){
    perror("fopen : ");
    return false;
  }
  while(!feof(fp)){
    fgets(line, 255, fp);
    if(strstr(line, dsoname)){
      ret = true;
      break;/*}}}*/
    }
  }
  fclose(fp);
  return ret;
}


