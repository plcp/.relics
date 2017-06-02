#ifndef QBINSTR_INS_T_H_GUARD_
#define QBINSTR_INS_T_H_GUARD_
#include "dr_api.h"

// ins_t: an instruction
//
struct ins_t
{
    // Leguacy
    app_pc loc;    // which instruction were read (pc)
    int opcode;    // opcode read

    // Registers state
    dr_mcontext_t mcontext; // general registers state
};

#endif
