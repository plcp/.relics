#ifndef QBINSTR_DEFINE_H_GUARD_
#define QBINSTR_DEFINE_H_GUARD_

// macro-related stuff (qbinstr namespac-ish macros)
#define __qbinstr__(key) __qbinstr_dereference key
#define __qbinstr_dereference(key) QBINSTR_##key

// global const chars
const char __qbinstr__((client_name))[]     = "qbinstr";
const char __qbinstr__((client_desc))[]     = "Somewhat qb-related exercise";
const char __qbinstr__((client_url))[]      = "http://github.com/plcp";
const char __qbinstr__((client_welcome))[]  = "Client 'qbinstr' started\n";
const char __qbinstr__((log_filename))[]    = "%s_pid_%d_th_%d.log";
const char __qbinstr__((log_format))[]      = "%p: %s\n";
const char __qbinstr__((log_format_help))[] = "<instruction pointer>: <opcode>";

// buffer size (hold instructions before flushing to logfile)
#define QBINSTR_buffer_size 8192

// log-line buffer size used to build-up disassembled instructions
#define QBINSTR_logline_size 255

#endif
