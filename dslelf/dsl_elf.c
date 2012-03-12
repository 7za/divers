#ifndef DLS_ELF_H
#define DLS_ELF_H
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/uio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#ifdef __x86_64__
typedef Elf64_Ehdr dls_ehdr;
typedef Elf64_Shdr dls_shdr;
typedef Elf64_Phdr dls_phdr;
typedef Elf64_Sym  dls_sym;
typedef Elf64_Rel  dls_rel; 
typedef Elf64_Rela dls_rela; 
typedef Elf64_Addr dls_addr;
#else
typedef Elf32_Ehdr dls_ehdr;
typedef Elf32_Shdr dls_shdr;
typedef Elf32_Phdr dls_phdr;
typedef Elf32_Sym  dls_sym;
typedef Elf32_Rel  dls_rel; 
typedef Elf32_Rela dls_rela; 
typedef Elf32_Addr dls_addr;
#endif

struct dls_file {
    int     df_fd;
    char   *df_map;
    size_t  df_size;
};

struct dls_external_sym {
    dls_rela *de_rel;
    char     *de_name;
    dls_sym  *de_sym;  
};

struct dls_external_symlist {
    int des_len;
    struct dls_external_sym *des_array;
};

int dls_file_open(char name[], struct dls_file *f)
{
    struct stat st;
    char *map;
    int fd = open(name, O_RDONLY);
    if(fd < 0)
        return -errno;
    if(fstat(fd, &st)) {
        close(fd);
        return -errno;
    }
    map = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(MAP_FAILED == map) {
        close(fd);
        return -errno;
    }
    f->df_fd   = fd;
    f->df_map  = map;
    f->df_size = st.st_size;
    return 0;
}

void dls_file_close(struct dls_file *f)
{
    munmap(f->df_map, f->df_size);
    close(f->df_fd);
}

char* dls_file_get_section_strings(char *f)
{
    dls_ehdr *ehdr = (dls_ehdr*)f;
    int index = ehdr->e_shstrndx;
    dls_shdr *sec = (dls_shdr*) (f + ehdr->e_shoff);
    return f + sec[index].sh_offset;
}


int dls_search_dso(char name[], struct dls_file *f, struct iovec *vector)
{
    FILE *fp;
    void *addrstart, *addrend;
    int offset, find = 0;
    char filename[sizeof("/proc/XXXXX/maps")], line[512], perm[5], lib[255],
         garbage[255], *ptr;

    snprintf(filename, sizeof(filename), "/proc/%u/maps", getpid());

    fp = fopen(filename, "r");
    if(NULL == fp)
        return -1;

    while(!find && fgets(line, sizeof(line), fp)) {
        sscanf(line, "%p-%p %[^ ] %x %[^/]%s", &addrstart, &addrend, perm, &offset, garbage, lib);
        ptr = basename(lib);
        if(!offset && strcmp(ptr, name) == 0) {
            dls_file_open(lib, f);
            vector->iov_base = addrstart; 
            vector->iov_len  = (char*)addrend - (char*)addrstart;
            find = 1;
        }
    }
    fclose(fp);
    return find;
}

dls_shdr* dls_get_shdr_by_name(char *base, char *name)
{
    dls_ehdr *ehdr = (dls_ehdr*)base;
    dls_shdr *shdr = (dls_shdr*)(base + ehdr->e_shoff);
    char *section_name = dls_file_get_section_strings(base);
    int nr_section = ehdr->e_shnum;
    int i;
    for(i = 0; i < nr_section; ++i, ++shdr) {
        char *this_name = &section_name[shdr->sh_name];
        if(strcmp(this_name, name) == 0) {
            return shdr;
        }
    }
    return NULL;
}

void funky(void)
{
   printf("clope\n"); 
}


static char* dls_get_rel_sym(char *base, int section_ind, int sym_ind)
{
    int num_section = ((dls_ehdr*)base)->e_shnum;
    int i;
    dls_shdr *walker =(dls_shdr *) (base + ((dls_ehdr*)base)->e_shoff);
    for(i = 0; i < num_section; ++i, ++walker) {
        if(walker->sh_type == SHT_RELA && section_ind == walker->sh_link) {
            dls_rela *curr;
            unsigned int numrel = walker->sh_size / sizeof(dls_rel);
            int j;
            curr = (dls_rela *)(base + walker->sh_offset);
            for(j = 0; j < numrel; ++j, ++curr) {
                if(ELF64_R_SYM(curr->r_info) == sym_ind) {
                    return (char*)curr;
                }
            }
        }
    }
    return NULL;
}

static int dls_get_external_syms(char *base, struct dls_external_symlist *out)
{
    dls_shdr *s4 = dls_get_shdr_by_name(base, ".dynsym");
    dls_shdr *s5 = dls_get_shdr_by_name(base, ".dynstr");
    int nr_sym = s4->sh_size / sizeof(dls_sym);
    int i, count;
    dls_shdr *first_section = (dls_shdr *)(base + ((dls_ehdr*)base)->e_shoff);

    char *u = base + s5->sh_offset;
    dls_sym *sym = (dls_sym *)(base + s4->sh_offset);
    out->des_array = malloc(sizeof(struct dls_external_sym) * nr_sym);
    out->des_len = 0;
    for(i = 0; i < nr_sym; ++i, ++sym) {
        if(ELF64_ST_BIND(sym->st_info) == 1 &&
           ELF64_ST_TYPE(sym->st_info) == 0) {
            out->des_array[out->des_len].de_sym  = sym;
            out->des_array[out->des_len].de_name = u + sym->st_name;
            out->des_array[out->des_len].de_rel  =
                    (void*)dls_get_rel_sym(base, s4 - first_section, i);
            out->des_len++; 
        }
    }
    out->des_array = realloc(out->des_array,
                             out->des_len * sizeof(struct dls_external_sym));
    return out->des_len;
}


int main(int argc, char *argv[])
{
    void *handle;
    struct iovec vec2;
    dls_shdr *walker = NULL;
    const char *section_name;
    struct dls_file f;

    handle = dlopen("./liblib.so", RTLD_LAZY);

    if(!handle) {
        puts(dlerror());
        return 0;
    }

    dls_search_dso("liblib.so", &f, &vec2); 
    struct dls_external_symlist out;
    dls_get_external_syms(f.df_map, &out); 
    int i;
    for(i = 0; i < out.des_len; ++i) {
        if(!strcmp(out.des_array[i].de_name, "funky")) {
            printf("setting %p\n", funky);
            *(uint64_t*)(vec2.iov_base + out.des_array[i].de_rel->r_offset) =
                funky;
        }
    }
    void (*ptr)(void) = dlsym(handle, "hello_world");
    ptr();

    dlclose(handle);
    dls_file_close(&f);
}

#endif
