#include "taint.h"
#include "tree.h"

#include <stdio.h>
#include <string.h>
#include <sys/queue.h>

#define TAINTED_LEN 32
/*
Need to index by:
func/execute_data, type, index
Could be a tree
*/
struct stack_entry {
	/**
	* Store the tainted argument
	* indexes in a bitmap
	*/
	zend_op* fcall_op;
	unsigned int args;
	phd_node* nodes[100];
	
	LIST_ENTRY(stack_entry) entries;
};

LIST_HEAD(list_head, stack_entry) stack_head;

phd_node* root;
int first_resv_op = 0;

void
taint_it (zend_execute_data* data, zend_op* o, uint type, uint index, phd_node* parent) 
{

	phd_node* entry = tree_create_node();
	
	entry->func = create_func_id(data, o, type, index);
	entry->type = type;
	entry->index = index;
	entry->data = (void *) data;

	if (type == IS_CV) {
		// set the var name
		zend_compiled_variable* var = data->op_array->vars;
		int name_len = strlen(var[index].name) + 1;
		char* var_name = (char*) emalloc(name_len);
		strcpy(var_name, var[index].name);
		entry->name = var_name;
	}

	tree_add_child(parent, entry);
}

void
taint (zend_execute_data* data, zend_op* o, uint op1_type, uint op2_type, uint res_type, uint base_address)
{
	char* op1_str;
	uint code = o->opcode;
	
	phd_node* entry;

	// check common input sources
	if (code == FETCH_R_OP && op1_type == IS_CONST) {
		op1_str = zval_string(o->op1.zv);

		if (
			strcmp("_GET", op1_str) == 0 ||
			strcmp("_POST", op1_str) == 0 ||
			strcmp("_COOKIE", op1_str) == 0 ||
			strcmp("_REQUEST", op1_str) == 0 ||
			strcmp("_SESSION", op1_str) == 0 
		) {
			taint_it(data, o, res_type, o->result.var, root);
		}

		efree(op1_str);
		return;
	}

	if ( // src = op1, dest = res
		code == FETCH_DIM_R_OP ||
		code == FETCH_OBJ_R_OP ||
		code == FETCH_R_OP
	) {
		phd_node* found = tree_search_node(
			root, (void *) data, op1_type, (o->op1.var)
		);
	
		phd_node* exists = tree_search_node(
			root, (void *) data, res_type, o->result.var
		);

		if (found) {
			taint_it(data, o, res_type, o->result.var, found);
		}

		return;
	}
	else
	if ( // src = op2, dest = op1
		(code >= CAST_OP && code <= ASSIGN_REF_OP)
	) {
		phd_node* found = tree_search_node(
			root, (void *) data, op2_type, o->op2.var
		);

		phd_node* exists = tree_search_node(
			root, (void *) data, op1_type, o->op1.var
		);
		
		// src tainted, dest can be tainted
		if (found) {
			taint_it(data, o, op1_type, o->op1.var, found);
		} 
		else // src not tainted, dest tainted
		if (!found && exists) {
			// untaint it
			tree_remove_child(exists);
		}

		return;
	}
	else
	if ( // src = op1/op2, dest = res
		code >= ADD_OP && 
		code <= BOOL_XOR_OP
	) {
		printf("\nO: %d", code);
		phd_node* found_op1 = tree_search_node(
			root, (void *) data, op1_type, o->op1.var
		);

		phd_node* found_op2 = tree_search_node(
			root, (void *) data, op2_type, o->op2.var
		);

		if (found_op1) {
			taint_it(data, o, res_type, o->result.var, found_op1);
		} else if (found_op2) {
			taint_it(data, o, res_type, o->result.var, found_op2);
		}
	}

	if (
		code == EXT_FCALL_BEGIN_OP
	) {
		zend_op* next_op = o + 1;
		int arg_index = 0;

		struct stack_entry* se = (struct stack_entry*) emalloc(sizeof(struct stack_entry));
		se->args = 0;
		se->fcall_op = NULL;

		// init to null pointers
		int i = 0;
		for (; i < 100; ++i)
			se->nodes[i] = NULL;

		LIST_INSERT_HEAD(&stack_head, se, entries);

		while (
			next_op->opcode == SEND_VAR_OP ||
			next_op->opcode == SEND_VAL_OP ||
			next_op->opcode == SEND_REF_OP
		) {
			printf("\nARG %d", next_op->op1.var);
			if (next_op->opcode == SEND_VAR_OP) {
				uint n_op1_type;
				uint n_op2_type;
				uint n_res_type;
				parse_opline(next_op, &n_op1_type, &n_op2_type, &n_res_type);
				phd_node* found = tree_search_node(
					root, 
					(void *) data, 
					n_op1_type, 
					next_op->op1.var
				);

				if (found) {
					printf("\nFOUND TAINTED ARG %d", arg_index);	
					se->nodes[arg_index] = found;
				}
			}

			next_op++;
			arg_index++;
		}
	
		if (
			next_op->opcode == DO_FCALL_BY_NAME_OP ||
			next_op->opcode == DO_FCALL_OP
		) {
			// store the pointer to 
			// the return var
			se->fcall_op = next_op;
		}
		
		se->args = arg_index;
		first_resv_op = 1;
		printf("\nMAP: %d", se->args);
		return;
	}

	if (
		(code == RECV_OP ||
		code == RECV_INIT_OP) &&
		first_resv_op
	) {
		first_resv_op = 0;

		zend_op* next_op = o;
		struct stack_entry* se = stack_head.lh_first;
		unsigned int arg_len = se->args;
		unsigned int arg_index = 0;
 
		while (
			next_op->opcode == RECV_OP ||
			next_op->opcode == RECV_INIT_OP
		) {
			phd_node* parent = se->nodes[arg_index];

			if (parent) {
				uint n_res_type;
				uint n_op1_type;
				uint n_op2_type;

				parse_opline(
					next_op, 
					&n_op1_type, 
					&n_op2_type,
					&n_res_type
				);
				
				taint_it(
					data, 
					next_op, 
					n_res_type, 
					next_op->result.var, 
					parent
				);
			}

			next_op++;
			arg_index++;
		}
	}

	if (
		code == RETURN_OP ||
		code == RETURN_BY_REF_OP
	) {
		struct stack_entry* se = stack_head.lh_first;
		

		/**
		Get stack head for res info
		Get op1, see if it's tainted
		If so, taint res info. Parent being
		~0 or found.
		`*/
		phd_node* found = tree_search_node(
			root,
			(void *) data,
			op1_type,
			o->op1.var
		);

		if (found && se->fcall_op) {
			uint n_op1_type;
			uint n_op2_type;
			uint n_res_type;
			parse_opline(se->fcall_op, &n_op1_type, &n_op2_type, &n_res_type);

			if (n_res_type != IS_UNUSED) {
				taint_it(
					data->prev_execute_data,
					se->fcall_op,
					n_res_type,
					se->fcall_op->result.var,
					found
				);
			}
		
			LIST_REMOVE(se, entries);
			efree(se);
		}
	}
}

void
taint_init (void)
{
	root = tree_create_node();
	root->func = "(root)";
	root->type = 0;
	root->index = 0;

	LIST_INIT(&stack_head);
}

void
taint_cleanup (void)
{
	tree_print(root, 0);	
	tree_free(root);
}
