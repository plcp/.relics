#ifndef QBINSTR_TOOLS_H_GUARD_ // TODO: move code to .c
#define QBINSTR_TOOLS_H_GUARD_

#include "dr_api.h"

#include "define.h"
#include "globals.h"

#include "ins_t.h"
#include "th_data_t.h"
#include "slots_offset.h"



//
// Tools
//



// get thread local storage entry
//
static void** get_tls_slot(
        byte* segment,
        uint offset,
        enum slot_offsets entry)
{
    return (void **) (segment + offset + entry);
}

// read the thread's buffer address from tls
//
static struct ins_t* read_buffer_p(struct th_data_t* th_data)
{
    return *(struct ins_t**) get_tls_slot(
            th_data->segment,
            tls_offset,
            __qbinstr__((tls_buffer_offset)));
}

// write the thread's buffer pointer to tls
//
static void write_buffer_p(struct th_data_t* th_data)
{
    (
        *(struct ins_t **)
            get_tls_slot(
                th_data->segment,
                tls_offset,
                (__qbinstr__((tls_buffer_offset)))
            )
    ) = th_data->buffer;
}

#endif
