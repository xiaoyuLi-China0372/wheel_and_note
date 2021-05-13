#ifndef _CMN_COMMON_H_INCLUDED_
#define _CMN_COMMON_H_INCLUDED_

#include <stdint.h>
#include <stddef.h>

/* This must be placed around external function declaration for C++
 * support. */
#ifdef __cplusplus
# define BEGIN_DECLS extern "C" {
# define END_DECLS }
#else
# define BEGIN_DECLS
# define END_DECLS
#endif

#ifndef MASK_BITS
#define MASK_BITS(m) ((1ll << (m)) - 1)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif
