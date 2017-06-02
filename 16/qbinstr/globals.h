#ifndef QBINSTR_GLOBALS_H_GUARD_
#define QBINSTR_GLOBALS_H_GUARD_

// macros & const are here
#include "define.h"

// client's id (initialized in dr_client_main)
static client_id_t client_id;

// global instruction count
static void* global_inscount_mutex;
static uint64 global_inscount = 0;

static reg_id_t tls_segment; // base    (of the thread local storage we use)
static uint tls_offset;      // offset  (of the thread local storage we use)
static int tls_id;           // id      (of the thread local storage we use)

#endif
