#ifndef _CMN_VECTOR_H_INCLUDED_
#define _CMN_VECTOR_H_INCLUDED_
#include "../list/common.h"
BEGIN_DECLS

typedef struct CmnVector {
    int size;
    int capacity;
    int unit_size;
    void *data;
} CmnVector;

CmnVector *cmn_vector_create(int unit_size);
#define CMN_VECTOR_CREATE(unit_type) cmn_vector_create(sizeof(unit_type))

void cmn_vector_free(CmnVector *vt);

int cmn_vector_pushBack(CmnVector *vt, void *unit);

void *cmn_vector_getNewBack(CmnVector *vt);

static inline void *cmn_vector_at(CmnVector *vt, int idx)
{
    return (uint8_t *)vt->data + (idx * vt->unit_size);
}

int cmn_vector_reserve(CmnVector *vt, int new_cap);

void cmn_vector_shrinkToFit(CmnVector *vt);

//insert n * unit at the front of the unit whose index is idx
int cmn_vector_insert(CmnVector *vt, int idx, void *unit, int n);

static inline void cmn_vector_resize(CmnVector *vt, int count)
{
    if (count > vt->capacity)
        cmn_vector_reserve(vt, count);
    vt->size = count;
}

END_DECLS
#endif
