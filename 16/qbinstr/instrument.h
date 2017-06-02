#ifndef QBINSTR_INSTRUMENT_H_GUARD_ // TODO: move code to .c
#define QBINSTR_INSTRUMENT_H_GUARD_
#include <stdlib.h>
#include "dr_api.h"
#include "drreg.h"

#include "define.h"
#include "globals.h"

#include "ins_t.h"

//
// Instrumentation
//



// qbinstr_inline_init_ptr: insert inline code to load buffer address
//
static void qbinstr_inline_init_ptr(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins,
        reg_id_t reg_ptr
    )
{
    // (inline code) read from the thread local storage the buffer pointer
    dr_insert_read_raw_tls(context, list_ins, current_ins,
            tls_segment,                                    // from
            tls_offset + __qbinstr__((tls_buffer_offset)),  // where
            reg_ptr                                         // to
        );
}

// qbinstr_inline_save_loc: insert inline code to save program counter to ins_t
//
static void qbinstr_inline_save_loc(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins,
        reg_id_t reg_ptr,
        reg_id_t reg_acc
    )
{
    // get current instruction program counter (so-called instruction pointer)
    app_pc program_counter = instr_get_app_pc(current_ins);

    // (inline code) load the value of the current program counter into reg_acc
    instrlist_insert_mov_immed_ptrsz(
            context,                               // (context)
            (ptr_int_t) (size_t) program_counter,  //  - mov program_counter
            opnd_create_reg(reg_acc),              //  - to reg_acc (as operand)
            list_ins,                              // (instruction list)
            current_ins,                           // (current instruction)
            NULL,                                  // (unused)
            NULL                                   // (unused)
        );
    // man - http://dynamorio.org/docs/dr__ir__utils_8h.html#a271b766e4daebc20fcec8b9956855f6a

    // (inline code) copy the value of reg_acc to the buffer's struct ins_t
    instrlist_meta_preinsert(list_ins, current_ins,
            XINST_CREATE_store(context,          // store
                OPND_CREATE_MEMPTR(              // to this address (as operand)
                    reg_ptr,                     // (at the end of the buffer,
                    offsetof(struct ins_t, loc)  //  as a ins_t, in field loc)
                ),                               // the value of
                opnd_create_reg(reg_acc)         // reg_acc (as operand)
            )
        );
    // man - http://dynamorio.org/docs/dr__ir__macros__aarch64_8h.html#a1e3d6b69547cf435c33abe09e08cc5ad
}

// qbinstr_inline_save_opc: insert inline code to save current opcode to struct
//
static void qbinstr_inline_save_opc(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins,
        reg_id_t reg_ptr,
        reg_id_t reg_acc
    )
{
    // get the current instruction opcode
    int opcode = instr_get_opcode(current_ins);

    // !! don't forget to fit your register to opcode size !!
    reg_acc = reg_resize_to_opsz(reg_acc, OPSZ_2);

    // (inline code) load the value of the current opcode into reg_acc
    instrlist_meta_preinsert(list_ins, current_ins,
            XINST_CREATE_load_int(context,   // load an integer
                opnd_create_reg(reg_acc),    // into reg_acc (as operand)
                OPND_CREATE_INT16(opcode)    // with value opcode (as operand)
            )
        );
    // man - http://dynamorio.org/docs/dr__ir__macros__aarch64_8h.html#ad432bb80a82d76edae2071473f9b4318

    // (inline code) copy the value of reg_acc to the buffer's struct ins_t
    instrlist_meta_preinsert(list_ins, current_ins,
            XINST_CREATE_store_2bytes(context,     // store 2 bytes (16b)
                OPND_CREATE_MEM16(                 // to this address (as opnd)
                    reg_ptr,                       // (at the end of the buffer
                    offsetof(struct ins_t, opcode) //  as a ins_t, in opcode)
                ),                                 // the value of
                opnd_create_reg(reg_acc)           // reg_acc (as operand)
            )
        );
    // man - http://dynamorio.org/docs/dr__ir__macros__aarch64_8h.html#a53192b93a4055d4700421104b2af3e90
}

// qbinstr_inline_move_ptr: insert inline code to update the buffer pointer
//
static void qbinstr_inline_move_ptr(
        void* context,
        instrlist_t* list_ins,
        instr_t* current_ins,
        reg_id_t reg_ptr
    )
{
    // (inline code) reserve one more ins_t at the end of the buffer
    instrlist_meta_preinsert(list_ins, current_ins,
            XINST_CREATE_add(context,                   // add
                opnd_create_reg(reg_ptr),               // to reg_ptr (as opnd)
                OPND_CREATE_INT16(sizeof(struct ins_t)) // one ins_t (as opnd)
            )
        );
    // man - http://dynamorio.org/docs/dr__ir__macros__aarch64_8h.html#a0cda7689cd077cc10bd2ffded339ff51

    dr_insert_write_raw_tls(context, list_ins, current_ins,
            tls_segment,                                    // from
            tls_offset + __qbinstr__((tls_buffer_offset)),  // where
            reg_ptr                                         // what
        );
    // man - http://dynamorio.org/docs/dr__tools_8h.html#a481c4c988efd4d81ce0a92fe0cc3f276
}

// qbinstr_instrument: insert inline code to buffer a new instruction
//
static void qbinstr_instrument(
        void* context,        // context
        instrlist_t* list_ins,  // instruction list
        instr_t* current_ins    // current instruction
    )
{
    // we use one register to store the buffer ptr
    reg_id_t reg_ptr = DR_REG_NULL;
    if(drreg_reserve_register(context, list_ins, current_ins, NULL, &reg_ptr)
            != DRREG_SUCCESS)
        DR_ASSERT(false);

    // we use a second register for general purpose use
    reg_id_t reg_acc = DR_REG_NULL;
    if(drreg_reserve_register(context, list_ins, current_ins, NULL, &reg_acc)
            != DRREG_SUCCESS)
        DR_ASSERT(false);

    // write inline code to :
    //  - read the buffer pointer from the thread local storage
    //  - write the program counter onto the buffer
    //  - write the opcode onto the buffer
    //  - move the buffer pointer to the next ins_t
    //  - write back the updated value of buffer pointer
    //
    qbinstr_inline_init_ptr(context, list_ins, current_ins, reg_ptr);
    qbinstr_inline_save_loc(context, list_ins, current_ins, reg_ptr, reg_acc);
    qbinstr_inline_save_opc(context, list_ins, current_ins, reg_ptr, reg_acc);
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
