#include "util.h"
/**
* Find the index of an opcode in an opcode array
* return -1 if not found
*/
int 
find_index (zend_op* op, zend_op_array* opa)
{
	int i = 0;
	for (; i < opa->last; ++i) {
		if (&opa->opcodes[i] == op) {
			return i;
		}
	}

	return -1;
}

/**
* Some strange function to allocate a string
* from a zval
*/
inline char* 
zval_string (zval* value)
{
	ZVAL_VALUE_STRING_TYPE *new_str;
	int new_len;

	new_str = php_url_encode(value->value.str.val, value->value.str.len, &new_len);
	return new_str;
}

/**
* Print a zval
*/
void 
printf_zval (zval* v)
{
	char* str;

	switch (v->type) {
		case IS_NULL:
			printf("(null)");
			break;
		case IS_LONG:
			printf("%ld", v->value.lval);
			break;
		case IS_DOUBLE:
			printf("%g", v->value.dval);
			break;
		case IS_STRING:
		case IS_CONSTANT:
			str = zval_string(v);
			printf("'%s'", str);
			efree(str);
			break;
		case IS_ARRAY:
			printf("(array)");
			break;
		case IS_OBJECT:
			printf("(object)");
			break;
		case IS_RESOURCE:
			printf("(resource)");
			break;
#if PHP_VERSION_ID >= 70000
		case IS_PTR:
			printf("(ptr)");
			break;
		case IS_UNDEF:
			printf("(undef)");
			break;
		case IS_TRUE:
			printf("true");
			break;
		case IS_FALSE:
			printf("false");
			break;
#else
		case IS_BOOL:
			printf("(bool)");
			break;	
#endif
		default:
			printf("?");
			break;
	}
}

char*
create_func_id (zend_execute_data *data, zend_op* o, unsigned int type, unsigned int index)
{
	char* func;
	int f_len = 0;
	f_len += strlen(data->op_array->function_name);
	f_len += strlen(data->op_array->filename);
	f_len += 20; // buffer room for lineno and seperators

	zend_compiled_variable* var = data->op_array->vars;
	if (type == IS_CV) {
		f_len += strlen(var[index].name);
	}

	func = (char *) emalloc(f_len);
	snprintf(
		func, 
		f_len, 
		"%s#%s:%d ",
		data->op_array->function_name,
		data->op_array->filename,
		o->lineno	
	);

	switch (type) {
		case IS_UNUSED:
			strcat(func, "-");
			break;

		case IS_VAR:
			sprintf(func + strlen(func), "%%%d", VAR_NUM(index));
			break;

		case IS_TMP_VAR:
			sprintf(func + strlen(func), "~%d", VAR_NUM(index));
			break;

		case IS_CV:
			sprintf(func + strlen(func), "$%s", var[index].name);
			break;

		default:
			sprintf(func + strlen(func), "[%d %d]", type, index);
	}
	
	return func;
}

/**
* Print a znode
*/
void 
printf_znode (znode_op* n, unsigned int type, unsigned int base_address, zend_execute_data *data)
{
	zend_compiled_variable* var = data->op_array->vars;

	switch (type) {
		case IS_UNUSED:
			printf("-");
			break;

		case IS_CONST:
			printf_zval(n->zv);
			break;

		case IS_VAR:
			printf("%%%d", VAR_NUM(n->var));
			break;

		case IS_TMP_VAR:
			printf("~%d", VAR_NUM(n->var));
			break;

		case IS_CV:
			printf("$%s", var[n->var].name);
			break;

		case IS_OPLINE:
			printf("=>%d", (int) ((n->opline_num - base_address) / sizeof(zend_op)));
			break;

		case IS_OPNUM:
			printf("->%d", n->opline_num);
			break;

		case IS_CLASS:
			printf(":%d", n->var);
			break;

		default:
			printf("[%d]", type);
	}
}

/**
* Get our custom flags 
*/
unsigned int 
get_special_flags(const zend_op *op, unsigned int base_address)
{
	unsigned int flags = 0;

	switch (op->opcode) {
#if PHP_VERSION_ID >= 70000
		case ZEND_FE_RESET_R:
		case ZEND_FE_RESET_RW:
#else
		case ZEND_FE_RESET:
#endif
			flags = ALL_USED;
#if (PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1)
			flags |= OP2_OPNUM;
#elif (PHP_MAJOR_VERSION > 4) || (PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION > 3) || (PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION == 3 && PHP_RELEASE_VERSION >= 11)
			flags |= NOP2_OPNUM;
#endif
			break;

		case ZEND_ASSIGN_REF:
			flags = OP1_USED | OP2_USED;
			if (op->VLD_TYPE(result) != IS_UNUSED) {
				flags |= RES_USED;
			}
			break;

#if PHP_VERSION_ID < 70000
		case ZEND_DO_FCALL_BY_NAME:
#endif
		case ZEND_DO_FCALL:
			flags = OP1_USED | RES_USED | EXT_VAL;
			break;

		case ZEND_INIT_FCALL_BY_NAME:
			flags = OP2_USED;
			if (op->VLD_TYPE(op1) != IS_UNUSED) {
				flags |= OP1_USED;
			}
			break;

		case ZEND_JMPZNZ:
			flags = OP1_USED | OP2_USED | EXT_VAL | OP2_OPNUM;
			break;

#if (PHP_MAJOR_VERSION < 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 1)
		case ZEND_JMP_NO_CTOR:
			flags = OP2_USED | OP2_OPNUM;
			if (op->VLD_TYPE(op1) != IS_UNUSED) {
				flags |= OP1_USED;
			}
			break;
#endif

#if defined(ZEND_ENGINE_2) || defined(ZEND_ENGINE_3)
		case ZEND_FETCH_CLASS:
			flags = EXT_VAL|RES_USED|OP2_USED|RES_CLASS;
			break;
#endif

		case ZEND_NEW:
			flags = RES_USED|OP1_USED;
#if defined(ZEND_ENGINE_2) || defined(ZEND_ENGINE_3)
			flags |= OP1_CLASS;
#endif
			break;

		case ZEND_BRK:
		case ZEND_CONT:
			flags = OP2_USED|OP2_BRK_CONT;
			break;
#if PHP_VERSION_ID >= 50500
		case ZEND_FAST_CALL:
			flags = OP1_USED|OP1_OPLINE;
			if (op->extended_value) {
				flags |= OP2_USED|OP2_OPNUM|EXT_VAL;
			}
			break;
		case ZEND_FAST_RET:
			if (op->extended_value) {
				flags = OP2_USED|OP2_OPNUM|EXT_VAL;
			}
			break;
#endif
	}
	return flags;
}

void
parse_opline (zend_op* o, uint* op1, uint* op2, uint* res)
{
	
	uint base_address = (uint)(zend_intptr_t)&(o);
	uint flags, op1_type, op2_type, res_type;
	flags = opcodes[o->opcode].flags;
	op1_type = o->op1_type;
	op2_type = o->op2_type;
	res_type = o->result_type;

    if (flags == SPECIAL) {
        flags = get_special_flags(o, base_address);
    }
    if (flags & OP1_OPLINE) {
        op1_type = IS_OPLINE;
    }
    if (flags & OP2_OPLINE) {
        op2_type = IS_OPLINE;
    }
    if (flags & OP1_OPNUM) {
        op1_type = IS_OPNUM;
    }
    if (flags & OP2_OPNUM) {
        op2_type = IS_OPNUM;
    }
    if (flags & OP1_CLASS) {
        op1_type = IS_CLASS;
    }
    if (flags & RES_CLASS) {
        res_type = IS_CLASS;
    }

	*op1 = op1_type;
	*op2 = op2_type;
	*res = res_type;
}
