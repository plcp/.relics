#ifndef QBINSTR_PROBE_H_GUARD_
#define QBINSTR_PROBE_H_GUARD_

#include "define.h"

// qbinstr_probe: fully-featured prober
//
static void qbinstr_probe(
    void* context,
    app_pc pc,
    dr_mcontext_t mcontext,
    struct th_data_t* th_data)
{
    // Count instructions
    th_data->inscount += 1;

    // Exhaustive display
    int nbprint = 0;
    char buffer[__qbinstr__((logline_size)) + 1];
    disassemble_to_buffer(
        context,
        (byte*) pc,
        (byte*) pc,
        true,
        args_info.bytes_given,
        buffer,
        __qbinstr__((logline_size)),
        &nbprint
    );
    if(nbprint > 0)
        buffer[nbprint - 1] = '\0';

    fprintf(
        th_data->log_file,
        "%-8lu:\n",
        th_data->inscount
    );

    if(!args_info.quiet_given)
    {
        fprintf(
            th_data->log_file,
            "        + " "%s\n",
            buffer
        );
    }

    if(args_info.general_given)
    {

        fprintf(
            th_data->log_file,
            "        + " "rax: %-16lx rbx: %-16lx rcx: %-16lx rdx: %-16lx\n"
            "        + " "rdi: %-16lx rsi: %-16lx rbp: %-16lx rsp: %-16lx\n"
            "        + " "rflags: %-16lx\n"
            ,
            mcontext.rax,
            mcontext.rbx,
            mcontext.rcx,
            mcontext.rdx,
            mcontext.rdi,
            mcontext.rsi,
            mcontext.rbp,
            mcontext.rsp,
            mcontext.rflags
        );
    }

    if(args_info.memory_given)
    {
        instr_t current_ins_raw;
        instr_t* current_ins = &current_ins_raw;
        decode(context, pc, current_ins);

        __attribute__((unused))
        uint rsize = instr_memory_reference_size(current_ins);
        if(IF_ARM_ELSE(instr_is_predicated(current_ins), false))
            fprintf(th_data->log_file, "        + " "PREDICATED\n");

        uint i = 0;
        bool write;
        app_pc addr;
        while(rsize > 0 &&
            instr_compute_address_ex(
                current_ins,
                &mcontext,
                i,
                &addr,
                &write)
             )
        {
            i += 1;

            fprintf(
                th_data->log_file,
                "        + " "access at Ox%-16p -> ",
                addr
            );

            if(write)
                fprintf(th_data->log_file, "WRITE %u bytes: ", rsize);
            else
                fprintf(th_data->log_file, "READ  %u bytes: ", rsize);

            if(instr_has_rel_addr_reference(current_ins))
            {
                fprintf(th_data->log_file, "relative\n");
                continue;
            }

            size_t nbytes = 0;
            byte mem_buffer[8 + 1];
            if(!dr_safe_read(addr, (rsize > 8) ? 8 : rsize, mem_buffer, &nbytes))
            {
                fprintf(th_data->log_file, "error\n");
                continue;
            }

            for(size_t j = 0; j < nbytes; ++j)
                fprintf(
                    th_data->log_file,
                    "%02x ",
                    mem_buffer[j]);

            fprintf(
                th_data->log_file,
                " [dump of %lu bytes]\n",
                nbytes
            );
        }
    }
}

// qbinstr_clean_probe: clean call used as probe
//
static void qbinstr_clean_probe(app_pc pc)
{
    void* context = dr_get_current_drcontext();
    dr_mcontext_t mcontext = {
            sizeof(mcontext),
            DR_MC_ALL
        };

    struct th_data_t* th_data;
    th_data = (struct th_data_t*) drmgr_get_tls_field(context, tls_id);

    if(args_info.general_given || args_info.memory_given)
        dr_get_mcontext(context, &mcontext);

    qbinstr_probe(context, pc, mcontext, th_data);
}

#endif
