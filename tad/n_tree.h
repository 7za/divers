#ifndef __N__TREE_H
#define __N__TREE_H

#include <stdbool.h>

struct ntree_node;

struct ntree_node {
	struct ntree_node *next;
	struct ntree_node *parent;
	struct ntree_node *child;
};

#define NTREE_ELT_INIT  { (void*)0, (void*)0, (void*)0 }

#define ntree_elt_is_root(elt)   (to_ntree((elt))->parent == (void*)0)
#define ntree_elt_has_child(elt) (((struct ntree_node*)(elt))->child != (void*)0)
#define ntree_elt_has_next(elt)  (((struct ntree_node*)(elt))->next  != (void*)0)
#define to_ntree(elt)            ((struct ntree_node*)(elt))
#define ntree_to_elt(tree)       ((void*)(tree))

#define ntree_prepend_child(elt, __child)						\
	do {										\
     		struct ntree_node *_tmp = to_ntree(elt)->child;				\
		 to_ntree(__child)->parent = to_ntree(elt);				\
		 to_ntree(elt)->child = to_ntree(__child);				\
		 to_ntree(__child)->next  = _tmp;					\
	} while(0)

#define ntree_remove_child_if(father, condition)				\
	do {									\
		typeof(*father) *walk = (void*)0, *pwalk = (void*)0;		\
		ntree_foreach_child_cond(walk, father,(condition) ) {		\
			pwalk = walk;						\
	    	}								\
      		if(walk) {							\
        		if(pwalk) {						\
		      		to_ntree(pwalk)->next = to_ntree(walk)->next;	\
			} else {						\
		      		to_ntree(father)->child = to_ntree(walk)->next;	\
			}							\
		}								\
	} while(0)

#define ntree_remove_child(father, __child)					\
	do {									\
		typeof(*father) *walk = (void*)0, *pwalk = (void*)0;		\
		ntree_foreach_child_cond(walk, father,(walk != __child) ) {	\
			pwalk = walk;						\
		}								\
		if(walk) {							\
			if(pwalk) {						\
				to_ntree(pwalk)->next = to_ntree(walk)->next;	\
			} else {						\
				to_ntree(father)->child = to_ntree(walk)->next; \
			}							\
		}								\
	} while(0)

#define ntree_insert_child(father, __child, comparator)					\
	do{										\
		typeof(*father) *walk = (void*)0, *pwalk = (void*)0;			\
		to_ntree(__child)->parent = to_ntree(father);				\
		if(!to_ntree(father)->child) {						\
			to_ntree(father)->child = to_ntree(__child);			\
		} else {								\
			ntree_foreach_child_cond(walk,					\
						father,					\
						(comparator(walk,__child) <= 0)) {	\
        						pwalk = walk;			\
			}								\
			if(!pwalk) {							\
				to_ntree(__child)->next = to_ntree(father)->child;	\
				to_ntree(father)->child = to_ntree(__child);		\
			} else {							\
				to_ntree(__child)->next = to_ntree(pwalk)->next;	\
				to_ntree(pwalk)->next   = to_ntree(__child);		\
			}								\
		}									\
	}while(0)

#define ntree_foreach_child_cond(c_walker, c_starter, condition)			\
	for(c_walker = (typeof(c_walker))to_ntree(c_starter)->child;			\
			c_walker != (void*)0 && (condition);				\
			c_walker = (typeof(c_walker))to_ntree(c_walker)->next)

#define ntree_foreach_child(c_walker, c_starter)			\
	      ntree_foreach_child_cond(c_walker, c_starter, true)

#define ntree_foreach_ancestor(c_walker, c_starter)					\
	for(c_walker = (typeof(c_walker))to_ntree(c_starter)->parent;			\
			c_walker != (void*)0;						\
			c_walker = (typeof(c_walker))to_ntree(c_walker)->parent)

#endif
