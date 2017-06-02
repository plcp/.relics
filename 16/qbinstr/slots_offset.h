#ifndef QBINSTR_SLOTS_OFFSET_H_GUARD_
#define QBINSTR_SLOTS_OFFSET_H_GUARD_

// enum: Thread local storage slots offsets
//
enum slot_offsets {
    __qbinstr__((tls_buffer_offset)) = 0, // offset of the buffer (slot 0)
    __qbinstr__((tls_slots_counter))      // total count of slots used
};

#endif
