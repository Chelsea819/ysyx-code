// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Prototypes for DPI import and export functions.
//
// Verilator includes this file in all generated .cpp files that use DPI functions.
// Manually include this file where DPI .c import functions are declared to ensure
// the C functions match the expectations of the DPI imports.

#ifndef VERILATED_VYSYX_22041211_TOP__DPI_H_
#define VERILATED_VYSYX_22041211_TOP__DPI_H_  // guard

#include "svdpi.h"

#ifdef __cplusplus
extern "C" {
#endif


    // DPI IMPORTS
    // DPI import at vsrc/ysyx_22041211_top.v:69:42
    extern void ifebreak_func(int id_inst_i);
    // DPI import at vsrc/ysyx_22041211_top.v:73:30
    extern int pmem_read_task(int raddr, char wmask);
    // DPI import at vsrc/ysyx_22041211_top.v:74:31
    extern void pmem_write_task(int waddr, int wdata, char wmask);

#ifdef __cplusplus
}
#endif

#endif  // guard
