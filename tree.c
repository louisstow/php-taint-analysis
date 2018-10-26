#include "tree.h"
#include "php.h"
#include "php_phd.h"
#include "opcodes.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "zend.h"
#include "zend_extensions.h"

void
tree_add_child (phd_node* n, phd_node* entry)
{
	entry->parent = n;

	// get the current head child
	phd_node* old_head = n->child;
	
	// make the entry the new head
	if (old_head) entry->next = old_head;
	
	n->child = entry;
}

void
tree_remove_child (phd_node* entry)
{
	phd_node* parent = entry->parent;
	phd_node* next = entry->next;
	phd_node* before;

	// if the node is the first child
	// of the parent
	if (parent && parent->child == entry) {
		if (next) parent->child = next;
		else parent->child = NULL;
	} else if (parent) {
		// find the position in the linked
		// list and join together
		before = parent->child;
		while (before->next != entry)
			before = before->next;
		
		before->next = next;	
	}
}

phd_node*
tree_create_node (void)
{
	phd_node* n = (phd_node *) emalloc(sizeof(phd_node));
	n->next   = NULL;
	n->child  = NULL;
	n->parent = NULL;
	return n;
}

phd_node*
tree_search_node (phd_node* n, void* data, unsigned int type, unsigned int index)
{
	phd_node* i = n;
	phd_node* result;
	zend_op_array* oa = ((zend_execute_data*) data)->op_array;
	zend_compiled_variable* var = oa->vars;

	while (i != NULL) {
		/*printf("\n************************\n");
		printf("TYPE: %d vs %d \n", i->type, type);
		printf("INDEX: %d vs %d \n", i->index, index);
		printf("DATA: %p vs %p \n", i->data, data);
		if (i->type == IS_CV && index < oa->last_var)
			printf("NAME: %s vs %s \n", i->name, var[index].name);
		printf("************************\n\n");*/
		if (
			(i->type == type && 
			i->index == index &&
			i->data == data) ||
			(i->type == IS_CV && index < oa->last_var && strcmp(i->name, var[index].name) == 0)
		) {
			return i;
		}
	
		if (i->child) {
			result = tree_search_node(i->child, data, type, index);
			if (result != NULL) 
				return result;
		}
	
		i = i->next;
	}

	return NULL;
}

void
tree_print (phd_node* n, int depth)
{
	if (!n) return;

	phd_node* i = n->child;

	tree_print_node(n, depth);

	while (i != NULL) {
		tree_print(i, depth + 1);
		i = i->next;
	}
}

void
tree_print_node (phd_node* n, int depth)
{
	if (!n) return;
	int indent = depth * 4;
	printf("%.*s%s (%p)\n", indent, "                                  ", n->func, n->data);
}

void
tree_free (phd_node* n)
{
	return; // cant free right now sorry
	phd_node* i = n;
	phd_node* prev;

	while (i != NULL) {
		if (i->child) {
			tree_free(i->child);
		}

		//efree(i->func);
		prev = i;
		i = i->next;
		//efree(prev);
	}
}
