#ifndef QBINSTR_TRACER_H_GUARD_ // TODO: move code to .c
#define QBINSTR_TRACER_H_GUARD_
#include <stdio.h>
#include <stdint.h>
#include "dr_api.h"

#include "define.h"

#include "tools.h"
#include "th_data_t.h"



//
// Tracer
//



// qbinstr_flush: flush the buffer to file on a clean call
//
static void qbinstr_flush(void* context)
{
    struct th_data_t* th_data;
    struct ins_t* buffer_end;
    struct ins_t* it;

    // get the thread's meta-data stored in the thread local storage
    th_data = (struct th_data_t*) drmgr_get_tls_field(context, tls_id);

    // get the buffer from the dedictated raw thread local storage slot
    buffer_end = read_buffer_p(th_data);

    // foreach entry in the buffer
    for(it = th_data->buffer; it < buffer_end; it++)
    {
        fprintf(
            th_data->log_file,                  // log file
            __qbinstr__((log_format)),          // log format
            (void*) (uintptr_t) it->loc,        // program counter (address)
            decode_opcode_name(it->opcode)      // opcode (string)
        );
        th_data->inscount++;
    }

    // move the buffer's end back to its start (into the raw tls slot)
    write_buffer_p(th_data);
}

// qbinstr_clean: clean call
//
static void qbinstr_clean(void)
{
    qbinstr_flush(dr_get_current_drcontext());
}

#endif
