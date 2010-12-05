#ifndef __S__LIST_H
#define __S__LIST_H

#include <stdbool.h>


struct slist_elt_t;

struct slist_elt_t
{
  struct slist_elt_t *next;
};

#define SLIST_ELT_INIT  { (void*)0 }

#define slist_elt_has_next(elt)  (((struct slist_elt_t*)(elt))->next  != (void*)0)
#define to_slist(elt)	    ((struct slist_elt_t*)(elt))
#define slist_to_elt(slist)      ((void*)(slist))	    


#define slist_remove_elt(root, node)						\
	({	                                                    \
	    typeof(*root) *walk = root, *pwalk = NULL, *newr = root;  \
	    slist_foreach_cond(walk, root, ((walk) != (node))){       \
				pwalk = walk;	                                   \
			}	                                                 \
		  if(walk == node){	                                 \
	if(pwalk == NULL){                                      \
	  newr = slist_to_elt(to_slist(root)->next);            \
				} else{	                                         \
				  to_slist(pwalk)->next = to_slist(walk)->next;	 \
				}	                                               \
			}	                                                 \
			newr;	                                             \
	})


#define slist_insert_elt(root, link, comparator)	          \
 ({	                                                       \
		  typeof(*root) *walk, *pwalk = NULL, *newr = root;	   \
		  slist_foreach_cond(walk, root, comparator(walk, link)<0){   \
				pwalk = walk;	                                     \
		  }	                                                   \
		  if(!pwalk){	                                         \
	      to_slist(link)->next = to_slist(root);	            \
	      newr = link;	                                      \
	    } else{	                                             \
				to_slist(link)->next  = to_slist(pwalk)->next;	    \
				to_slist(pwalk)->next = to_slist(link);	           \
			}	                                                   \
			newr;	                                               \
 })



#define slist_foreach_cond(walk, start, cond)	                     \
	for(walk = start;	walk && (cond); walk = slist_to_elt(to_slist(walk)->next))

#define slist_foreach(c_walker, c_starter)	                        \
	      slist_foreach_cond(c_walker, c_starter, true)


#endif
