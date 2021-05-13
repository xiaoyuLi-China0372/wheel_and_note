#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "cmn_vector.h"
#include "../list/log.h"

static void cmn_vector_test();

int main(int argc, char **argv)
{
    cmn_vector_test();

    cmn_log("program finished\n");
    return 0;
}

typedef struct CmnInstr {
    uint32_t member[3];
} CmnInstr;

#define MAX_DATA_BIT_WIDTH 32

static int cmn_set_instr(CmnInstr *instr, int msb, int lsb, uint32_t data)
{
    cmn_assert(lsb >= 0);
    cmn_assert(msb >= lsb);
    cmn_assert(msb < (int)(sizeof(CmnInstr) * CHAR_BIT));

    int start = lsb;
    int end = msb;
    int mem_bit = sizeof(instr->member[0]) * CHAR_BIT;
#if MAX_DATA_BIT_WIDTH > 32
    int s_bit = MAX(start % mem_bit, 0);
    int e_bit = mem_bit - 1;

    int s_idx = start / mem_bit;
    int e_idx = end / mem_bit;
    for (int i = s_idx; i <= e_idx; ++i) {
        if (i == e_idx)
            e_bit = end % mem_bit;

        int n_bit = e_bit + 1 - s_bit;
        uint32_t value = data & MASK_BITS(n_bit);
        instr->member[i] &= ~(MASK_BITS(n_bit) << s_bit);
        instr->member[i] |= (value << s_bit);
        s_bit = 0;
        data >>= n_bit;
    }
#else
    int s_bit = MAX(start % mem_bit, 0);
    int s_idx = start / mem_bit;
    int n_bit = end + 1 - start;
    uint64_t value = data & MASK_BITS(n_bit);
    uint64_t *dst = (uint64_t *)&instr->member[s_idx];
    *dst &= ~(MASK_BITS(n_bit) << s_bit);
    *dst |= (value << s_bit);
#endif

    return 0;
}

static uint32_t cmn_get_instr(CmnInstr *instr, int msb, int lsb)
{
    cmn_assert(lsb >= 0);
    cmn_assert(msb >= lsb);
    cmn_assert(msb < (int)(sizeof(CmnInstr) * CHAR_BIT));

    uint32_t data = 0;
    int start = lsb;
    int end = msb;
    int mem_bit = sizeof(instr->member[0]) * CHAR_BIT;
#if MAX_DATA_BIT_WIDTH > 32
    int s_bit = MAX(start % mem_bit, 0);
    int e_bit = mem_bit - 1;

    int s_idx = start / mem_bit;
    int e_idx = end / mem_bit;
    int x_bit = 0;
    for (int i = s_idx; i <= e_idx; ++i) {
        if (i == e_idx)
            e_bit = end % mem_bit;

        int n_bit = e_bit + 1 - s_bit;
        uint32_t value = (instr->member[i] >> s_bit) & MASK_BITS(n_bit);
        data |= value << x_bit;
        x_bit += n_bit;
        s_bit = 0;
    }
#else
    int s_bit = MAX(start % mem_bit, 0);
    int s_idx = start / mem_bit;
    int n_bit = end + 1 - start;
    uint64_t *src = (uint64_t *)&instr->member[s_idx];
    data = (*src >> s_bit) & MASK_BITS(n_bit);
#endif

    return data;
}


static void cmn_vector_test()
{
    cmn_log("%s begin\n", __FUNCTION__);

    CmnVector *vt = CMN_VECTOR_CREATE(CmnInstr);
    CmnInstr instr[100];

    cmn_vector_reserve(vt, 1000);

    for (size_t i = 0; i < sizeof(instr) / sizeof(instr[0]); ++i) {
        cmn_set_instr(&instr[i], 68, 37, 0x87654321);
        cmn_vector_pushBack(vt, &instr[i]);
        CmnInstr *ins = (CmnInstr *)cmn_vector_getNewBack(vt);
        cmn_set_instr(ins, 64, 33, 0x12345678);
    }

    cmn_assert(vt->size == 2 * sizeof(instr) / sizeof(instr[0]));
    cmn_log("vt->capacity:%d\n", vt->capacity);

    cmn_vector_shrinkToFit(vt);

    cmn_assert(vt->size == 2 * sizeof(instr) / sizeof(instr[0]));
    cmn_assert(vt->capacity ==  2 * sizeof(instr) / sizeof(instr[0]));
    cmn_log("vt->capacity:%d\n", vt->capacity);

    for (size_t i = 0; i < sizeof(instr) / sizeof(instr[0]); ++i) {
        CmnInstr *ins = (CmnInstr *)cmn_vector_at(vt, i * 2 + 0);
        cmn_assert(cmn_get_instr(ins, 68, 37) == 0x87654321);
        ins = (CmnInstr *)cmn_vector_at(vt, i * 2 + 1);
        cmn_assert(cmn_get_instr(ins, 64, 33) == 0x12345678);
    }

    cmn_vector_free(vt);
    cmn_log("%s done\n", __FUNCTION__);
}

