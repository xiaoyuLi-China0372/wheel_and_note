#ifndef RCORE

#include "cmn_vector.h"
#include "../list/log.h"
#include "string.h"

CmnVector *cmn_vector_create(int unit_size)
{
    CmnVector *vt = (CmnVector *)cmn_alloc(sizeof(CmnVector));
    cmn_assert(vt);

    memset(vt, 0, sizeof(CmnVector));
    vt->unit_size = unit_size;

    return vt;
}

void cmn_vector_free(CmnVector *vt)
{
    if (vt == NULL)
        return;

    cmn_free(vt->data);
    vt->data = NULL;

    cmn_free(vt);
}

int cmn_vector_pushBack(CmnVector *vt, void *unit)
{
    if (vt->capacity <= vt->size) {
        cmn_vector_reserve(vt, vt->size + 1);
    }

    memcpy(cmn_vector_at(vt, vt->size), unit, vt->unit_size);
    vt->size += 1;
    return 0;
}

void *cmn_vector_getNewBack(CmnVector *vt)
{
    if (vt->capacity <= vt->size) {
        cmn_vector_reserve(vt, vt->size + 1);
    }

    vt->size += 1;
    return cmn_vector_at(vt, vt->size - 1);
}

int cmn_vector_reserve(CmnVector *vt, int new_cap)
{
    if (new_cap < vt->size)
        return -1;

    if (vt->capacity < new_cap) {
        vt->capacity = vt->capacity < 2 ? 2 : vt->capacity * 2;
        while (vt->capacity < new_cap) {
            vt->capacity *= 2;
        }
    } else {
        vt->capacity = new_cap;
    }

    uint8_t *tmp = (uint8_t *)cmn_alloc(vt->capacity * vt->unit_size);
    cmn_assert(tmp);
    memset(&tmp[vt->size * vt->unit_size], 0,
           (vt->capacity - vt->size) * vt->unit_size);

    memcpy(tmp, vt->data, vt->size * vt->unit_size);
    cmn_free(vt->data);
    vt->data = tmp;
    return 0;
}

void cmn_vector_shrinkToFit(CmnVector *vt)
{
    if (vt->size < 1)
        return;

    cmn_vector_reserve(vt, vt->size);
}

int cmn_vector_insert(CmnVector *vt, int idx, void *unit, int n)
{
    int ret = 0;
    if (vt->capacity < vt->size + n) {
        ret = cmn_vector_reserve(vt, vt->size + n);
    }

    void *src = cmn_vector_at(vt, idx);
    if (idx < vt->size) {
        void *dst = cmn_vector_at(vt, idx + n);
        memmove(dst, src, vt->unit_size * (vt->size - idx));
    }
    vt->size += n;

    memcpy(src, unit, vt->unit_size * n);

    return ret;
}
#endif

