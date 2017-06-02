#ifndef QBINSTR_TH_DATA_T_H_GUARD_
#define QBINSTR_TH_DATA_T_H_GUARD_
#include <stdio.h>
#include <stdint.h>
#include "dr_api.h"

// ins_t structure
#include "ins_t.h"

// th_data: storage local to the thread
//
struct th_data_t
{
    byte*           segment;  // segment (thread local storage base)
    struct ins_t*   buffer;   // buffer's start pointer
    FILE*           log_file; // log file used for this thread
    uint64_t        inscount; // instruction count for this thread
};

#endif
