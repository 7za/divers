#ifndef SYM_RESOLVER_H
#define SYM_RESOLVER_H

#include <elf.h>
#include <sys/types.h>

struct sym_resolv_desc
{
    int      sr_fd;
    void    *sr_mem;
    size_t   sr_memlen;
    pid_t    sr_pid;

    Elf32_Ehdr  *sr_ehdr;
    Elf32_Shdr  *sr_shdr;
    Elf32_Shdr  *sr_shdrstrsectab;
    Elf32_Shdr  *sr_shdrsymtab;
    Elf32_Shdr  *sr_shdrstrsymtab;

    char        *sr_sectionstr;
};



struct sym_resolv
{
    void    *sr_symaddr;
    char     sr_symname[512];
};


void
sym_resolv_close(struct sym_resolv_desc *desc);


int 
sym_resolv_open(struct sym_resolv_desc *desc);


int
sym_resolv_get_funcname_by_addr(struct sym_resolv_desc *desc,    
                                void *addr[],
                                struct sym_resolv res[], 
                                size_t tablen);


#endif
