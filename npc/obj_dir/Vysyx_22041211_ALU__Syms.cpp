// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table implementation internals

#include "Vysyx_22041211_ALU__Syms.h"
#include "Vysyx_22041211_ALU.h"
#include "Vysyx_22041211_ALU___024root.h"

// FUNCTIONS
Vysyx_22041211_ALU__Syms::~Vysyx_22041211_ALU__Syms()
{
}

Vysyx_22041211_ALU__Syms::Vysyx_22041211_ALU__Syms(VerilatedContext* contextp, const char* namep, Vysyx_22041211_ALU* modelp)
    : VerilatedSyms{contextp}
    // Setup internal state of the Syms class
    , __Vm_modelp{modelp}
    // Setup module instances
    , TOP{this, namep}
{
    // Configure time unit / time precision
    _vm_contextp__->timeunit(-12);
    _vm_contextp__->timeprecision(-12);
    // Setup each module's pointers to their submodules
    // Setup each module's pointer back to symbol table (for public functions)
    TOP.__Vconfigure(true);
    // Setup scopes
    __Vscope_ysyx_22041211_top.configure(this, name(), "ysyx_22041211_top", "ysyx_22041211_top", -12, VerilatedScope::SCOPE_OTHER);
    // Setup export functions
    for (int __Vfinal = 0; __Vfinal < 2; ++__Vfinal) {
    }
}