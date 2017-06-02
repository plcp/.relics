#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// access dynamorio's API
#include "dr_api.h"

// dynamorio's extension
#include "drmgr.h"  // intrumentation manager
#include "drreg.h"  // register & stuff
#include "drutil.h" // useful utils

// macros & globals
#include "define.h"
#include "globals.h"

// structures
#include "ins_t.h"
#include "th_data_t.h"
#include "slots_offset.h"

// args
#include "args.h"
#include "args_info.h"

struct args_info_t args_info;

// tools
#include "tools.h"
#include "tracer.h"
#include "callbacks.h"
#include "instrument.h"



//
// DynamicRIO client's entry point and exit callback
//



// qbinstr_exit : client's exit callback
//
static void qbinstr_exit()
{
    // free thread local storage raw slots
    if(!dr_raw_tls_cfree(tls_offset, __qbinstr__((tls_slots_counter))))
        DR_ASSERT(false);

    // unregister dynamorio-managed thread local storage slot
    if(!drmgr_unregister_tls_field(tls_id))
        DR_ASSERT(false);

    // unregister events
    if(!drmgr_unregister_thread_init_event(qbinstr_th_init))
        DR_ASSERT(false);
    if(!drmgr_unregister_thread_exit_event(qbinstr_th_exit))
        DR_ASSERT(false);
    if(args_info.inline_given)
    {
        if(!drmgr_unregister_bb_insertion_event(qbinstr_insins))
            DR_ASSERT(false);
    } else if(args_info.hybrid_given) {
        if(!drmgr_unregister_bb_insertion_event(qbinstr_inshybrid))
            DR_ASSERT(false);
    } else {
        if(!drmgr_unregister_bb_insertion_event(qbinstr_inscall))
            DR_ASSERT(false);
    }
    if(args_info.expand_given)
    {
        if(!drmgr_unregister_bb_app2app_event(qbinstr_expand))
            DR_ASSERT(false);
    }

    // destroy allocated mutex
    dr_mutex_destroy(global_inscount_mutex);

    // uninitialize extra drutil extension if expand arg is given.
    if(args_info.expand_given)
        drutil_exit();

    // un-initialize dynamorio's extension
    if(drreg_exit() != DRREG_SUCCESS)
        DR_ASSERT(false);
    drmgr_exit();
}

// dr_client_main : client's entry point
//
// (don't forget to export function visible from the outside)
DR_EXPORT
void dr_client_main(
        client_id_t id,    // client id
        int argc           __attribute__((unused)),
        const char** argv  __attribute__((unused))
    )
{
    // parse args
    args_parser(argc, (char**) argv, &args_info);
    if(args_info.inline_given && args_info.expand_given)
        fprintf(stderr,
                "Warning: --expand restrict --inline capabilities.\n");
    if(args_info.inline_given && args_info.general_given)
        fprintf(stderr,
                "Warning: --inline disable --general strictly.\n");

    // drreg options
    drreg_options_t drreg_options = {
        sizeof(drreg_options),          // cf. man (struct size)
        3,                              // register slots used (2 + eflags)
        false                           // cf. man (reduce overhead)
                                        // default to dr_abort()
    }; // man - http://dynamorio.org/docs/struct__drreg__options__t.html



    // custom error reporting
    dr_set_client_name(__qbinstr__((client_desc)), __qbinstr__((client_url)));

    // initialize dynamorio's extension
    if(!drmgr_init() || drreg_init(&drreg_options) != DRREG_SUCCESS)
        DR_ASSERT(false);

    // initialize extra drutil extension if expand arg is given.
    if(args_info.expand_given)
    {
        if(!drutil_init())
            DR_ASSERT(false);
    }

    // register a thread local storage slot forearch thread
    // (on slot tls_id, to store per-thread meta-related struct th_data_t)
    tls_id = drmgr_register_tls_field();
    DR_ASSERT(tls_id != 1);

    // allocate a raw thread local storage foreach thread
    // (on tls_segment, to store per-thread buffer-related structs ins_t)
    // (simpler & faster than using previously registered tls_id slot)
    if(!dr_raw_tls_calloc(
            &tls_segment,                       // cf. man (out)
            &tls_offset,                        // cf. man (out)
            __qbinstr__((tls_slots_counter)),   // number of slots required
            0                                   // (no alignement)
        ) // man - http://dynamorio.org/docs/dr__tools_8h.html#aea4bc51b4771fceb56f050d0f20cbc99
      )
        DR_ASSERT(false);

    // we use a mutex for concurrent access to global_inscount (qbinstr_th_exit)
    global_inscount_mutex = dr_mutex_create();

    // register client's exit callback (qbinstr_exit)
    dr_register_exit_event(qbinstr_exit);



    // register various events for intrumentation  :
    //  - one to track threads initialization (and init buffers)
    //  - one to track threads exiting and free associated ressources
    //  - one to inject required instructions into the traced application
    //

    // regiser thread initialization callback (qbinstr_th_init)
    if(!drmgr_register_thread_init_event(qbinstr_th_init))
        DR_ASSERT(false);

    // register thread exit callback (qbinstr_th_exit)
    if(!drmgr_register_thread_exit_event(qbinstr_th_exit))
        DR_ASSERT(false);

    // register app2app instrumentation event to unroll rep/repne loops
    if(args_info.expand_given)
    {
        if(!drmgr_register_bb_app2app_event(qbinstr_expand, NULL))
            DR_ASSERT(false);
    }

    // register instruction instrumentation event (qbinstr_insins)
    if(args_info.inline_given)
    {
        if(
            !drmgr_register_bb_instrumentation_event(
                NULL,                // no analysis callback
                qbinstr_insins,      // our insertion callback
                NULL                 // default priority (for both callbacks)
            ) // man - http://dynamorio.org/docs/group__drmgr.html#ga83a5fc96944e10bd7356e0c492c93966
          )
            DR_ASSERT(false);
    } else if(args_info.hybrid_given) {

        // save registers & pc to a TLS buffer to make fewers clean calls
        // (but still use fully-featured probe)
        if(
            !drmgr_register_bb_instrumentation_event(
                NULL,
                qbinstr_inshybrid,
                NULL)
          )
            DR_ASSERT(false);
    } else {

        // fully-featured (but awfully slow) trace based on clean calls only
        if(
            !drmgr_register_bb_instrumentation_event(
                NULL,
                qbinstr_inscall,
                NULL)
          )
            DR_ASSERT(false);
    }

    // remember client's id, log & let the control flow
    client_id = id;
    dr_log(
        NULL,                             // default log file
        LOG_ALL,                          // logging mask
        1,                                // logging level required
        __qbinstr__((client_welcome))     // log message
    ); // man - http://dynamorio.org/docs/dr__tools_8h.html#a332a14861f12823994465e8c9b6a3015
}

