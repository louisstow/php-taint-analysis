#ifndef PHP_PHD_H
#define PHP_PHD_H 1

#define PHP_PHD_WORLD_VERSION "1.0"
#define PHP_PHD_WORLD_EXTNAME "phd"

#include "php.h"
#include "php_globals.h"

extern zend_module_entry phd_module_entry;
#define phpext_phd_ptr &phd_module_entry

ZEND_BEGIN_MODULE_GLOBALS(phd)
ZEND_END_MODULE_GLOBALS(phd)

PHPAPI ZEND_EXTERN_MODULE_GLOBALS(phd)

#ifdef ZTS
	#define PHD_GLOBALS(v) TSRMG(phd_globals_id, zend_phd_globals *, v)
#else
	#define PHD_GLOBALS(v) (phd_globals.v)
#endif

#define IS_OPLINE 1 << 13
#define IS_OPNUM  1 << 14
#define IS_CLASS  1 << 15

// range of fetch opcodes
#define FETCH_OP_START 80
#define FETCH_OP_END   99

#if PHP_VERSION_ID >= 70000
	#define ZHASHKEYSTR(k) ((k)->key->val)
	#define ZHASHKEYLEN(k) ((k)->key->len)
	#define PHP_URLENCODE_NEW_LEN(v)

	#define ZVAL_VALUE_TYPE                zend_value
	#define ZVAL_VALUE_STRING_TYPE         zend_string
	#define ZVAL_STRING_VALUE(s)           (s.str)->val
	#define ZVAL_STRING_LEN(s)             (s.str)->len
	#define ZSTRING_VALUE(s)               (s ? (s)->val : NULL)
	#define OPARRAY_VAR_NAME(v)            (v)->val
#else
	#define ZHASHKEYSTR(k) ((k)->arKey)
	#define ZHASHKEYLEN(k) ((k)->nKeyLength)
	#define PHP_URLENCODE_NEW_LEN(v) , &(v)

	#define ZVAL_VALUE_TYPE                zvalue_value
	#define ZVAL_VALUE_STRING_TYPE         char
	#define ZVAL_STRING_VALUE(s)           (s.str.val)
	#define ZVAL_STRING_LEN(s)             (s.str.len)
	#define ZSTRING_VALUE(s)               (s)
	#define OPARRAY_VAR_NAME(v)            (v).name
#endif

#if PHP_VERSION_ID >= 50700
	#define VAR_NUM(v) EX_VAR_TO_NUM(v)
#else
	#if PHP_VERSION_ID >= 50500
		#define VAR_NUM(v) ((zend_uint)(EX_TMP_VAR_NUM(0, 0) - EX_TMP_VAR(0, v)))
	#else
		#define VAR_NUM(v) ((v)/(sizeof(temp_variable)))
	#endif
#endif

#endif
