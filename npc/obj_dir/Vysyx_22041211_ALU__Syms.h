// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VYSYX_22041211_ALU__SYMS_H_
#define VERILATED_VYSYX_22041211_ALU__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vysyx_22041211_ALU.h"

// INCLUDE MODULE CLASSES
#include "Vysyx_22041211_ALU___024root.h"

// DPI TYPES for DPI Export callbacks (Internal use)

// SYMS CLASS (contains all model state)
class Vysyx_22041211_ALU__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vysyx_22041211_ALU* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vysyx_22041211_ALU___024root   TOP;

    // SCOPE NAMES
    VerilatedScope __Vscope_ysyx_22041211_top;

    // CONSTRUCTORS
    Vysyx_22041211_ALU__Syms(VerilatedContext* contextp, const char* namep, Vysyx_22041211_ALU* modelp);
    ~Vysyx_22041211_ALU__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
} VL_ATTR_ALIGNED(VL_CACHE_LINE_BYTES);

#endif  // guard
