#ifndef QBINSTR_HYBRID_H_GUARD_
#define QBINSTR_HYBRID_H_GUARD_

#include "tracer.h"

static void qbinstr_hybrid(void)
{
    void* context = dr_get_current_drcontext();
    dr_mcontext_t mcontext = {
            sizeof(mcontext),
            DR_MC_ALL
        };

    struct th_data_t* th_data;
    th_data = (struct th_data_t*) drmgr_get_tls_field(context, tls_id);

    struct ins_t* buffer_end;
    struct ins_t* it;

    buffer_end = read_buffer_p(th_data);

    for(it = th_data->buffer; it < buffer_end; ++it)
    {
        it->mcontext.size = sizeof(dr_mcontext_t);
        it->mcontext.flags = DR_MC_ALL;
        it->mcontext.pc = it->loc;

        qbinstr_probe(
            context,
            it->loc,
            it->mcontext,
            th_data);
    }

    // don't forget to update the buffer
    write_buffer_p(th_data);
}

#endif
