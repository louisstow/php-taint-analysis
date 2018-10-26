#ifndef TAINT_H
#define TAINT_H 1

#include <string.h>
#include "util.h"
#include "php.h"
#include "zend.h"
#include "zend_extensions.h"

// fetch opcodes
#define FETCH_R_OP 80
#define FETCH_DIM_R_OP 81
#define FETCH_OBJ_R_OP 82
#define FETCH_W_OP 83
#define FETCH_DIM_W_OP 84
#define FETCH_OBJ_W_OP 85
#define FETCH_DIM_RW_OP 87
#define FETCH_OBJ_RW_OP 88

// func call opcodes 
#define RECV_OP 63
#define RECV_INIT_OP 64
#define SEND_VAL_OP 65
#define SEND_VAR_OP 66
#define SEND_REF_OP 67
#define EXT_FCALL_BEGIN_OP 102
#define EXT_FCALL_END_OP 103
#define DO_FCALL_OP 60
#define DO_FCALL_BY_NAME_OP 61
#define RETURN_OP 62
#define RETURN_BY_REF_OP 111

// assignment opcodes
#define CAST_OP 21
#define QM_ASSIGN_OP 22
#define ASSIGN_ADD_OP 23
#define ASSIGN_SUB_OP 24
#define ASSIGN_MUL_OP 25
#define ASSIGN_DIV_OP 26
#define ASSIGN_MOD_OP 27
#define ASSIGN_SL_OP 28
#define ASSIGN_SR_OP 29
#define ASSIGN_CONCAT_OP 30
#define ASSIGN_BW_OR_OP 31
#define ASSIGN_BW_AND_OP 32
#define ASSIGN_BW_XOR_OP 33
#define PRE_INC_OP 34
#define PRE_DEC_OP 35
#define POST_INC_OP 36
#define POST_DEC_OP 37
#define ASSIGN_OP 38
#define ASSIGN_REF_OP 39

// modification opcodes
#define ADD_OP 1
#define SUB_OP 2
#define BOOL_XOR_OP 14

typedef unsigned int uint;

void taint (zend_execute_data*, zend_op*, uint, uint, uint, uint);
void taint_init (void);
void taint_cleanup (void);

#endif
