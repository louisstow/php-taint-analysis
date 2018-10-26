#ifndef TREE_H
#define TREE_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "php.h"
#include "zend.h"
#include "zend_extensions.h"

typedef struct _phd_node {
    struct _phd_node* parent;
    struct _phd_node* child;
    struct _phd_node* next;

	void* data;
	char* func;
	char* name;
    unsigned int type;
    unsigned int index;
} phd_node;

void tree_add_child (phd_node*, phd_node*);
void tree_remove_child (phd_node*);
phd_node* tree_create_node (void);
void tree_print (phd_node*, int);
void tree_print_node (phd_node*, int);
phd_node* tree_search_node (phd_node*, void*, unsigned int, unsigned int);
void tree_free (phd_node*);

#endif
