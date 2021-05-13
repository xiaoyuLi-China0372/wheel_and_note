#ifndef _CMN_LOG_H_INCLUDED_
#define _CMN_LOG_H_INCLUDED_
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
BEGIN_DECLS

#define cmn_log(...) printf("[Info]: " __VA_ARGS__)
#define cmn_war(...) printf("[Warning]: " __VA_ARGS__)
#define cmn_err(...) printf("[Error]: " __VA_ARGS__)
#define cmn_assert(expr) if(!(expr))printf("[assert]: %s(%d): %s failed\n",__FUNCTION__,__LINE__, #expr)

#define cmn_alloc(size) malloc(size)
#define cmn_free(ptr) free(ptr)

END_DECLS
#endif
