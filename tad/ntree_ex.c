#include "n_tree.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

struct package_t {
	struct ntree_node link;
	char name[128];
	char desc[128];
};

int cmp(struct package_t *t1, struct package_t *t2)
{
	return strcmp(t1->name, t2->name);
}

bool cond(struct package_t * t1, char *p)
{
	if (t1)
		return strcmp(t1->name, p) == 0;
	return false;
}

int main()
{
	struct package_t *walker;
	struct package_t package[] = {
		{.link = NTREE_ELT_INIT,.name = "devel",.desc =
		 "developpement group"},
		{.link = NTREE_ELT_INIT,.name = "anjuta",.desc = "IDE"},
		{.link = NTREE_ELT_INIT,.name = "jeux",.desc = "game group"},
		{.link = NTREE_ELT_INIT,.name = "arcade",.desc =
		 "arcade group"},
		{.link = NTREE_ELT_INIT,.name = "space",.desc =
		 "space invader"},
		{.link = NTREE_ELT_INIT,.name = "sport",.desc = "sport group"},
		{.link = NTREE_ELT_INIT,.name = "fifa",.desc = "football"},
		{.link = NTREE_ELT_INIT,.name = "txtor",.desc = "fun game"},
		{.link = NTREE_ELT_INIT,.name = "sprint",.desc = "athle"},
		{.link = NTREE_ELT_INIT,.name = "ouet",.desc = "ouch"},
		{.link = NTREE_ELT_INIT,.name = "comprx",.desc = "compressor"},
		{.link = NTREE_ELT_INIT,.name = "list",.desc = "all"}
	};

	ntree_prepend_child(&package[0], &package[1]);
	ntree_prepend_child(&package[2], &package[3]);
	ntree_prepend_child(&package[3], &package[4]);
	ntree_insert_child(&package[3], &package[5], cmp);
	ntree_insert_child(&package[3], &package[8], cmp);
	ntree_insert_child(&package[3], &package[9], cmp);
	ntree_prepend_child(&package[5], &package[6]);
	ntree_prepend_child(&package[11], &package[0]);
	ntree_prepend_child(&package[11], &package[2]);

	struct package_t *search = NULL;

	/*
	    ntree_search_node(&package[11], cond, "fifa");

	*/
	ntree_foreach_child_cond(search, &package[0], !strcmp(search->name, "fifa")) {
		printf("name = %s\n", search->name);
	}

	printf("%s %x\n", search->name, &package[3]);


	printf("child of [%s, %s]\n",package[3].name, package[3].desc);
  ntree_foreach_child(walker, &package[3])
	{
		struct package_t *walker2;
    printf("\t%s %s\n", walker->name, walker->desc);
    ntree_foreach_child(walker2, walker)
		{
      printf("\t\t%s %s\n", walker2->name, walker2->desc);
		}
	}

	puts("##########################################");
	ntree_remove_child( &package[3], &package[5]);
  ntree_foreach_child(walker, &package[3])
	{
		struct package_t *walker2;
    printf("\t%s %s\n", walker->name, walker->desc);
    ntree_foreach_child(walker2, walker)
		{
      printf("\t\t%s %s\n", walker2->name, walker2->desc);
		}
	}
	puts("##########################################");
	ntree_remove_child( &package[3], &package[8]);
  ntree_foreach_child(walker, &package[3])
	{
		struct package_t *walker2;
    printf("\t%s %s\n", walker->name, walker->desc);
    ntree_foreach_child(walker2, walker)
		{
      printf("\t\t%s %s\n", walker2->name, walker2->desc);
		}
	}
	puts("##########################################");
	ntree_remove_child( &package[3], &package[9]);
  ntree_foreach_child(walker, &package[3])
	{
		struct package_t *walker2;
    printf("\t%s %s\n", walker->name, walker->desc);
    ntree_foreach_child(walker2, walker)
		{
      printf("\t\t%s %s\n", walker2->name, walker2->desc);
		}
	}
	puts("##########################################");
	ntree_remove_child( &package[3], &package[4]);
  ntree_foreach_child(walker, &package[3])
	{
		struct package_t *walker2;
    printf("\t%s %s\n", walker->name, walker->desc);
    ntree_foreach_child(walker2, walker)
		{
      printf("\t\t%s %s\n", walker2->name, walker2->desc);
		}
	}
	puts("##########################################");

  

	return 0;
}
