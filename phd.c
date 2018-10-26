#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "php.h"
#include "php_phd.h"
#include "opcodes.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "zend.h"
#include "zend_extensions.h"

#if PHP_VERSION_ID >= 50500
static void (*old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
static void phd_execute_ex(zend_execute_data *execute_data TSRMLS_DC);
#else
static void (*old_execute)(zend_op_array *op_array TSRMLS_DC);
static void phd_execute(zend_op_array *op_array TSRMLS_DC);
#endif

#ifndef ZEND_EXT_API
#define ZEND_EXT_API  ZEND_DLEXPORT
#endif
ZEND_EXTENSION();
ZEND_DECLARE_MODULE_GLOBALS(phd);

#define MAIN_FUNC_NAME "(main)"

static void 
phd_execute_ex (zend_execute_data *data TSRMLS_DC)
{
	printf("\nEXECUTE EX");
	if (!data->op_array->function_name) {
		char* name = emalloc(strlen(MAIN_FUNC_NAME) + 1);
		strcpy(name, MAIN_FUNC_NAME);
		data->op_array->function_name = name;
	}
	
	old_execute_ex(data TSRMLS_CC);
}

void
phd_opline_handler (zend_execute_data *data)
{
	unsigned int base_address = (unsigned int)(zend_intptr_t)&(data->opline);
	unsigned int flags, op1_type, op2_type, res_type;
	
	zend_op* tmp = data->opline;
	op1_type = tmp->op1_type;
	op2_type = tmp->op2_type;
	res_type = tmp->result_type;
	flags = opcodes[tmp->opcode].flags;

	if (flags == SPECIAL) {
		flags = get_special_flags(tmp, base_address);
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

	taint(data, tmp, op1_type, op2_type, res_type, base_address);

	// log the OPCODE
	printf("\n    %s", opcodes[tmp->opcode].name);
	printf(" ");
	printf_znode(&tmp->op1, op1_type, base_address, data);
	printf(" , ");
	printf_znode(&tmp->op2, op2_type, base_address, data);
	printf(" => ");
	printf_znode(&tmp->result, res_type, base_address, data);
}

int 
on_startup (zend_extension *extension)
{
	old_execute_ex = zend_execute_ex;
	zend_execute_ex = phd_execute_ex;
	zend_opline_handler = phd_opline_handler;

	TSRMLS_FETCH();
	CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;

	taint_init();
	printf("BEGIN");
	return SUCCESS;
}

void
on_stop (zend_extension *extension)
{
}

void
on_deactivate (void)
{
	printf("\nEND\n");
	taint_cleanup();
}

zend_function_entry phd_functions[] = {
    {NULL, NULL, NULL}
};

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	"phd",
	"1.0",
	"0pc0d3",
	"",
	"",
	on_startup,
	on_stop,
	NULL,
	on_deactivate,
	NULL,   // message_handler_func_t
	NULL,   // op_array_handler_func_t
	NULL,   // statement_handler_func_t
	NULL,   // fcall_begin_handler_func_t
	NULL,   // fcall_end_handler_func_t
	NULL,   // op_array_ctor_func_t
	NULL,   // op_array_dtor_func_t
	STANDARD_ZEND_EXTENSION_PROPERTIES
};

zend_module_entry phd_module_entry = {
	STANDARD_MODULE_HEADER,
	"phd",
	phd_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"1.0",
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHD
ZEND_GET_MODULE(phd)
#endif
