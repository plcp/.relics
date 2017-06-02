#ifndef QBINSTR_CALLBACKS_H_GUARD_ // TODO: move code to .c
#define QBINSTR_CALLBACKS_H_GUARD_
#include <stdio.h>
#include "dr_api.h"
#include "drmgr.h"

#include "define.h"
#include "globals.h"

#include "ins_t.h"
#include "th_data_t.h"

#include "tools.h"
#include "probe.h"
#include "hybrid.h"
#include "tracer.h"
#include "instrument.h"
#include "instrument_hybrid.h"

//
// Callbacks
//



// qbinstr_insins: instrument traced application (Leguacy, see --hybrid)
//
static dr_emit_flags_t qbinstr_insins(
        void* context,          // context
        void* tag               __attribute__((unused)),
        instrlist_t* list_ins,  // instruction list
        instr_t* current_ins,   // current instruction
        bool trace              __attribute__((unused)),
        bool trans              __attribute__((unused)),
        void* user_data         __attribute__((unused))
    )
{
    // return now if it's not an instruction from the traced application
    if(!instr_is_app(current_ins))
        return DR_EMIT_DEFAULT;

    // restrict instrumentation when we unroll rep/repne (-e && -i)
    // TODO: ?? use labels to re-encode jumps with updated addresses
    if(args_info.expand_given)
    {
        if(
            !instr_reads_memory(current_ins) &&
            !instr_writes_memory(current_ins)
          )
            return DR_EMIT_DEFAULT;
    }

    // insert required code to add an entry to the instruction buffer
    qbinstr_instrument(context, list_ins, current_ins);



    //
    // make a clean call once per basic block + avoid corner cases on ARM
    //

    // skip if current_ins do not start a basic block
    if(!drmgr_is_first_instr(context, current_ins))
        return DR_EMIT_DEFAULT;

    // no clean-calls on If-Then blocks on ARM
    if(IF_ARM_ELSE(instr_is_predicated(current_ins), false))
        return DR_EMIT_DEFAULT;
    // man - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489g/Cjabicci.html
    //  « Other restrictions when using an IT block are:
    //      - [...] any instruction that modifies the PC [...]
    //      - You cannot branch to any instruction in an IT block [...]
    //      - You cannot use any assembler directives in an IT block. »

    // avoid clean-calls between ldrex and strex on AR^W AARCHXX
    // (do not interfer with the exclusive monitor with instrumentation)
    if(false IF_AARCHXX(|| instr_is_exclusive_store(current_ins)))
        return DR_EMIT_DEFAULT;
    // man - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489e/Cihbghef.html



    // make a clean call, flush the buffer to logfile
    // TODO: make a clean call only when the buffer is full
    dr_insert_clean_call(context, list_ins, current_ins,
            (void*) qbinstr_clean,
            false,
            0
        );

    return DR_EMIT_DEFAULT;
}

// qbinstr_inscall: instrume^W insert clean calls everywhere
//
static dr_emit_flags_t qbinstr_inscall(
        void* context,          // context
        void* tag               __attribute__((unused)),
        instrlist_t* list_ins,  // instruction list
        instr_t* current_ins,   // current instruction
        bool trace              __attribute__((unused)),
        bool trans              __attribute__((unused)),
        void* user_data         __attribute__((unused))
    )
{
    // return now if it's not an instruction from the traced application
    if(!instr_is_app(current_ins))
        return DR_EMIT_DEFAULT;

    // restrict instrumentation when we unroll rep/repne (-e && -i)
    // TODO: ?? use labels to re-encode jumps with updated addresses
    if(args_info.expand_given)
    {
        if(
            !instr_reads_memory(current_ins) &&
            !instr_writes_memory(current_ins)
          )
            return DR_EMIT_DEFAULT;
    }

    // cf. qbinstr_insins
    if(IF_ARM_ELSE(instr_is_predicated(current_ins), false))
        return DR_EMIT_DEFAULT;

    if(false IF_AARCHXX(|| instr_is_exclusive_store(current_ins)))
        return DR_EMIT_DEFAULT;

    // make a clean call to our high-level probe
    // note: no performance here
    dr_insert_clean_call(context, list_ins, current_ins,
            (void*) qbinstr_clean_probe,
           false,
            1,
            OPND_CREATE_INTPTR(instr_get_app_pc(current_ins))
        );

    return DR_EMIT_DEFAULT;
}

// qbinstr_inshybrid: instrument traced application
//
static dr_emit_flags_t qbinstr_inshybrid(
        void* context,          // context
        void* tag               __attribute__((unused)),
        instrlist_t* list_ins,  // instruction list
        instr_t* current_ins,   // current instruction
        bool trace              __attribute__((unused)),
        bool trans              __attribute__((unused)),
        void* user_data         __attribute__((unused))
    )
{
    // return now if it's not an instruction from the traced application
    if(!instr_is_app(current_ins))
        return DR_EMIT_DEFAULT;

    // restrict instrumentation when we unroll rep/repne (-e && -i)
    // TODO: ?? use labels to re-encode jumps with updated addresses
    if(args_info.expand_given)
    {
        if(
            !instr_reads_memory(current_ins) &&
            !instr_writes_memory(current_ins)
          )
            return DR_EMIT_DEFAULT;
    }

    // insert required code to add an entry to the instruction buffer
    qbinstr_instrument_hybrid(context, list_ins, current_ins);

    // make a clean call once per basic block + avoid corner cases on ARM
    if(!drmgr_is_first_instr(context, current_ins))
        return DR_EMIT_DEFAULT;

    if(IF_ARM_ELSE(instr_is_predicated(current_ins), false))
        return DR_EMIT_DEFAULT;
    if(false IF_AARCHXX(|| instr_is_exclusive_store(current_ins)))
        return DR_EMIT_DEFAULT;

    // make a clean call, flush the buffer to logfile
    // TODO: make a clean call only when the buffer is full
    dr_insert_clean_call(context, list_ins, current_ins,
            (void*) qbinstr_hybrid,
            false,
            0
        );

    return DR_EMIT_DEFAULT;
}

// qbinstr_th_init: initialize per-thread structs & slots
//
static void qbinstr_th_init(void* context)
{
    // initialize per-thread data structure
    struct th_data_t* th_data =
        (struct th_data_t*) dr_thread_alloc(context, sizeof(struct th_data_t));
    DR_ASSERT(th_data != NULL);

    // use the thread local storage slot tls_id to store thread's meta-data
    drmgr_set_tls_field(context, tls_id, th_data);

    // store tls_segment in th_data for simpler & faster access to the buffer
    // (the raw thread local storage allocated for buffer-related purpose)
    th_data->segment = (byte*) dr_get_dr_segment_base(tls_segment);
    DR_ASSERT(th_data->segment != NULL);

    // allocate required buffer size into the thread local storage
    th_data->buffer = (struct ins_t*)
        dr_raw_mem_alloc(
                __qbinstr__((buffer_size)) * sizeof(struct ins_t), // buf. size
                DR_MEMPROT_READ | DR_MEMPROT_WRITE,                // r/w mem
                NULL                                               // (unused)
            ); // man - http://dynamorio.org/docs/dr__tools_8h.html#ad6201fa3676b0afb76f91f15822cf0d1
    DR_ASSERT(th_data->buffer != NULL);

    // initialize the buffer address on the raw thread local storage slot
    write_buffer_p(th_data);

    // init inscount
    th_data->inscount = 0;

    // anticipate filename lenght
    size_t filename_size = snprintf(NULL, 0, // dry-run of the format
        __qbinstr__((log_filename)),
        __qbinstr__((client_name)),
        dr_get_process_id(),
        dr_get_thread_id(context)) + 1; // don't forget to null-terminate~

    // one frame for hideous static-allocation
    {
        char filename[filename_size];

        // generate the filename
        sprintf(filename,
            __qbinstr__((log_filename)),
            __qbinstr__((client_name)),
            dr_get_process_id(),
            dr_get_thread_id(context));

            // fopen target logging file
            th_data->log_file = fopen(filename, "w+");
    }

    if(args_info.inline_given)
        // log format help
        fprintf(th_data->log_file, "Format: %s\n", __qbinstr__((log_format_help)));
}

// qbinstr_th_exit: free per-thread structs & slots
//
static void qbinstr_th_exit(void* context)
{
    // first, flush to log before any other thing
    qbinstr_flush(context);

    // get the thread's meta-data stored in the thread local storage
    struct th_data_t* th_data;
    th_data = (struct th_data_t*) drmgr_get_tls_field(context, tls_id);

    // update global_inscount
    dr_mutex_lock(global_inscount_mutex);
    global_inscount += th_data->inscount;
    dr_mutex_unlock(global_inscount_mutex);

    // close the logfile
    fclose(th_data->log_file);

    // free the memory allocated for the buffer
    dr_raw_mem_free(
        th_data->buffer,
        __qbinstr__((buffer_size)) * sizeof(struct ins_t));

    // free the struct th_data & the thread from its duty
    dr_thread_free(context, th_data, sizeof(struct th_data_t));
}

static dr_emit_flags_t qbinstr_expand(
        void* context,
        void* tag               __attribute__((unused)),
        instrlist_t* list_ins,
        bool trace              __attribute__((unused)),
        bool trans              __attribute__((unused))
    )
{
    // expand rep/repne string loops
    if(!drutil_expand_rep_string(context, list_ins))
        DR_ASSERT(false);

    return DR_EMIT_DEFAULT;
}

#endif
