#ifndef UTIL_H
#define UTIL_H 1

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "zend.h"
#include "zend_extensions.h"
#include "opcodes.h"
#include "php_phd.h"

int find_index (zend_op*, zend_op_array*);
char* zval_string (zval*);
void printf_zval (zval*);
void printf_znode (znode_op*, unsigned int, unsigned int, zend_execute_data *);
unsigned int get_special_flags (const zend_op*, unsigned int);
char* create_func_id (zend_execute_data *, zend_op*, unsigned int, unsigned int);
void parse_opline (zend_op*, uint*, uint*, uint*);

#endif
