#include <elf.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdbool.h>

#define __USE_GNU
#include <dlfcn.h>

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "sym_resolver.h"

static char *__sym_resolv_sectionhdr_to_section(struct sym_resolv_desc *const
						desc, Elf32_Shdr * shdr)
{
	if (!desc || !shdr) {
		return NULL;
	}
	return (char *)(desc->sr_mem) + shdr->sh_offset;
}

static Elf32_Shdr *__sym_resolv_get_sectionhdr_by_name(struct sym_resolv_desc
						       *const desc,
						       char *const secname)
{
	Elf32_Shdr *ret = NULL;
	uint16_t i = 0;
	uint16_t nbsec = 0;

	if (!desc || !desc->sr_ehdr) {
		return NULL;
	}

	nbsec = desc->sr_ehdr->e_shnum;

	for (i = 0; ret == NULL && i < nbsec; ++i) {
		Elf32_Shdr *ptr = &desc->sr_shdr[i];
		char *currstr = desc->sr_sectionstr + ptr->sh_name;
		if (!strcmp(secname, currstr)) {
			ret = ptr;
		}
	}
	return ret;
}

static int __sym_resolv_open(pid_t pid, struct sym_resolv_desc *desc)
{
	struct stat stat;
	char buff[PATH_MAX], buff2[PATH_MAX];
	int ret;
	size_t len;

	snprintf(buff, sizeof(buff), "/proc/%d/exe", pid);
	readlink(buff, buff2, sizeof(buff2));

	desc->sr_fd = open(buff, O_RDONLY, 0644);
	if (desc->sr_fd <= 0) {
		return -1;
	}

	ret = fstat(desc->sr_fd, &stat);
	if (ret < 0) {
		goto sr_open_staterr;
	}

	len = (size_t) stat.st_size;

	desc->sr_mem = mmap(NULL, len, PROT_READ, MAP_PRIVATE, desc->sr_fd, 0);
	if (desc->sr_mem == MAP_FAILED) {
		goto sr_open_maperr;
	}

	desc->sr_memlen = len;
	desc->sr_pid = pid;
	desc->sr_ehdr = (Elf32_Ehdr *) desc->sr_mem;
	desc->sr_shdr =
	    (Elf32_Shdr *) (((char *)(desc->sr_mem)) + desc->sr_ehdr->e_shoff);

	if (desc->sr_ehdr->e_shstrndx == SHN_UNDEF) {
		goto sr_no_stndxerr;
	}

	desc->sr_shdrstrsectab = &desc->sr_shdr[desc->sr_ehdr->e_shstrndx];
	desc->sr_sectionstr = __sym_resolv_sectionhdr_to_section(desc,
								 desc->sr_shdrstrsectab);
	desc->sr_shdrsymtab =
	    __sym_resolv_get_sectionhdr_by_name(desc, ".symtab");
	if (!desc->sr_shdrsymtab) {
		goto sr_no_symtab;
	}

	desc->sr_shdrstrsymtab = __sym_resolv_get_sectionhdr_by_name(desc,
								     ".strtab");
	if (!desc->sr_shdrstrsymtab) {
		goto sr_no_symname;
	}

	return 0;

 sr_no_symname:
 sr_no_symtab:
 sr_no_stndxerr:
	munmap(desc->sr_mem, desc->sr_memlen);
 sr_open_maperr:
 sr_open_staterr:
	close(desc->sr_fd);

	return -1;
}

void sym_resolv_close(struct sym_resolv_desc *desc)
{
	munmap(desc->sr_mem, desc->sr_memlen);
	close(desc->sr_fd);
}

int sym_resolv_open(struct sym_resolv_desc *desc)
{
	return __sym_resolv_open(getpid(), desc);
}

static bool
__sym_resolv_get_funcname_by_addr(struct sym_resolv_desc *desc,
				  void *addr, struct sym_resolv *res)
{
	Elf32_Sym *walker;
	char *symnamelist;
	uint32_t i, nb_sym;
	bool find = false;
	if (!desc || !desc->sr_shdrsymtab || !desc->sr_shdrstrsymtab) {
		return false;
	}

	walker = (Elf32_Sym *) __sym_resolv_sectionhdr_to_section(desc,
								  desc->sr_shdrsymtab);

	symnamelist = __sym_resolv_sectionhdr_to_section(desc,
							 desc->
							 sr_shdrstrsymtab);

	if (!walker || !symnamelist) {
		return false;
	}

	nb_sym = desc->sr_shdrsymtab->sh_size / sizeof(*walker);

	for (i = 0; !find && i < nb_sym; ++walker, ++i) {
		void *start = (void *)walker->st_value;
		void *end = (void *)((unsigned long)walker->st_value +
				     (unsigned long)walker->st_size);
		if (start <= addr && end > addr) {
			strncpy(res->sr_symname,
				symnamelist + walker->st_name,
				sizeof(res[i].sr_symname));
			res->sr_symaddr = start;
			find = true;
		}
	}

	return find;
}

Elf32_Sym *sym_resolv_symbol(struct sym_resolv_desc * desc, char name[])
{
	Elf32_Sym *walker = NULL;
	char *symnamelist;
	size_t nb_sym, i;

	walker = (Elf32_Sym *) __sym_resolv_sectionhdr_to_section(desc,
								  desc->sr_shdrsymtab);

	symnamelist = __sym_resolv_sectionhdr_to_section(desc,
							 desc->
							 sr_shdrstrsymtab);

	if (!walker || !symnamelist) {
		return false;
	}

	nb_sym = desc->sr_shdrsymtab->sh_size / sizeof(*walker);

	for (i = 0; i < nb_sym; ++walker, ++i) {
		if (strcmp(name, symnamelist + walker->st_name) == 0) {
			return walker;
		}
	}
	return NULL;
}

int
sym_resolv_addr(struct sym_resolv_desc *desc,
		void *addr[], struct sym_resolv res[], size_t tablen)
{
	Dl_info info = { 0, 0, 0, 0 };
	size_t i = 0;
	for (i = 0; i < tablen; ++i) {
		if (__sym_resolv_get_funcname_by_addr(desc, addr[i], &res[i]) ==
		    false) {
			if (dladdr(addr[i], &info) <= 0 || !info.dli_sname) {
				res[i].sr_symname[0] = '\0';
			} else {
				strncpy(res[i].sr_symname, info.dli_sname,
					sizeof(res[i].sr_symname));
				res[i].sr_symaddr = info.dli_saddr;
			}
		}
	}

	return 0;
}
