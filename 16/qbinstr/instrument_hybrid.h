#ifndef QBINSTR_INSTRUMENT_HYBRID_H_GUARD_
#define QBINSTR_INSTRUMENT_HYBRID_H_GUARD_

#include "instrument.h"

// syntastic sugar
#define QBINSTR_sgreg(reg, sreg) qbinstr_inline_mc_sgreg(\
    context, list_ins, current_ins, reg_ptr, reg_acc,\
    DR_REG_##reg, offsetof(dr_mcontext_t, sreg));

// handle_spilled_registers: take care of spilled registers
//
static bool handle_spilled_registers(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins,
        reg_id_t reg_ptr,
        reg_id_t reg_acc,
        reg_id_t reg_used,
        reg_id_t target,
        size_t reg_offset
    )
{
    // return false if we still need to handle things later
    if(!reg_overlap(reg_used, target))
        return false;

    uint dr_slot_id;
    bool is_dr_slot;
    opnd_t tls_slot;
    if(
        drreg_reservation_info(
            context,
            reg_used,
            &tls_slot,
            &is_dr_slot,
            &dr_slot_id
        ) != DRREG_SUCCESS
      )
        DR_ASSERT(false);

    // TODO: take care of this case
    if(is_dr_slot)
    {
        fprintf(stderr, "Warning: Unable to retrieve spilled register.");
        return false;
    }

    // TODO: Verify this piece of code
    drmgr_insert_read_tls_field(
        context,
        dr_slot_id,
        list_ins,
        current_ins,
        reg_acc);

    // save-it back
    instrlist_meta_preinsert(list_ins, current_ins,
            XINST_CREATE_store(context,          // store
                    OPND_CREATE_MEMPTR(              // to this address (as operand)
                    reg_ptr,                     // (at the end of the buffer,
                    offsetof(struct ins_t, mcontext)  // in mcontext
                    + reg_offset                      // in target register
                ),                               // the value of
                opnd_create_reg(reg_acc)         // target register (as operand)
            )
        );

    return true;
}

// qbinstr_inline_mc_sgreg: insert inline code to store general register
//
static void qbinstr_inline_mc_sgreg(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins,
        reg_id_t reg_ptr,
        reg_id_t reg_acc,
        reg_id_t target,
        size_t reg_offset
    )
{
    // take care of reg_ptr
    if(handle_spilled_registers(
            context,
            list_ins,
            current_ins,
            reg_ptr, reg_acc, reg_ptr,
            target, reg_offset))
        return;

    // take care of reg_acc
    if(handle_spilled_registers(
            context,
            list_ins,
            current_ins,
            reg_ptr, reg_acc, reg_acc,
            target, reg_offset))
        return;

    // if dynamorio have stolen the target register (ARM-only)
    if(IF_ARM_ELSE(reg_is_stolen(target), false))
    {
        // get the register value in reg_acc
        dr_insert_get_stolen_reg_value(
            context,
            list_ins,
            current_ins,
            reg_acc);

        // save-it back
        instrlist_meta_preinsert(list_ins, current_ins,
                XINST_CREATE_store(context,          // store
                        OPND_CREATE_MEMPTR(              // to this address (as operand)
                        reg_ptr,                     // (at the end of the buffer,
                        offsetof(struct ins_t, mcontext)  // in mcontext
                        + reg_offset                      // in target register
                    ),                               // the value of
                    opnd_create_reg(reg_acc)         // target register (as operand)
                )
            );

        // TODO: test this piece of code
        return;
    }

    // (inline code) copy the value of target register to buffer
    instrlist_meta_preinsert(list_ins, current_ins,
            XINST_CREATE_store(context,          // store
                OPND_CREATE_MEMPTR(              // to this address (as operand)
                    reg_ptr,                     // (at the end of the buffer,
                    offsetof(struct ins_t, mcontext)  // in mcontext
                    + reg_offset                      // in target register
                ),                               // the value of
                opnd_create_reg(target)          // target register (as operand)
            )
        );
}

// qbinstr_instrument_hybrid: insert inline code to buffer pc & registers
//                            (returns false if no dead register could be used)
//
static void qbinstr_instrument_hybrid(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins
    )
{
    reg_id_t reg_ptr = DR_REG_NULL;
    if(drreg_reserve_register(
            context, list_ins, current_ins, NULL, &reg_ptr) != DRREG_SUCCESS)
        DR_ASSERT(false);

    reg_id_t reg_acc = DR_REG_NULL;
    if(drreg_reserve_register(
            context, list_ins, current_ins, NULL, &reg_acc) != DRREG_SUCCESS)
        DR_ASSERT(false);

    qbinstr_inline_init_ptr(context, list_ins, current_ins, reg_ptr);
    qbinstr_inline_save_loc(context, list_ins, current_ins, reg_ptr, reg_acc);
    qbinstr_inline_save_opc(context, list_ins, current_ins, reg_ptr, reg_acc);
    __qbinstr__((sgreg)) (RDI, rdi);
    __qbinstr__((sgreg)) (RSI, rsi);
    __qbinstr__((sgreg)) (RBP, rbp);
    __qbinstr__((sgreg)) (RSP, rsp);
    __qbinstr__((sgreg)) (RBX, rbx);
    __qbinstr__((sgreg)) (RDX, rdx);
    __qbinstr__((sgreg)) (RCX, rcx);
    __qbinstr__((sgreg)) (RAX, rax);
    __qbinstr__((sgreg)) (R8 , r8);
    __qbinstr__((sgreg)) (R9 , r9);
    __qbinstr__((sgreg)) (R10, r10);
    __qbinstr__((sgreg)) (R11, r11);
    __qbinstr__((sgreg)) (R12, r12);
    __qbinstr__((sgreg)) (R13, r13);
    __qbinstr__((sgreg)) (R14, r14);
    __qbinstr__((sgreg)) (R15, r15);
    // TODO: save eflags to buffer
    qbinstr_inline_move_ptr(context, list_ins, current_ins, reg_ptr);

    // restore the registers we used
    if(drreg_unreserve_register(context, list_ins, current_ins, reg_ptr)
            != DRREG_SUCCESS)
        DR_ASSERT(false);

    if(drreg_unreserve_register(context, list_ins, current_ins, reg_acc)
            != DRREG_SUCCESS)
        DR_ASSERT(false);
}

#endif
