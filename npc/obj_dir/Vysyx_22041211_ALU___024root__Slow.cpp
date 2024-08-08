// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vysyx_22041211_ALU.h for the primary calling header

#include "verilated.h"
#include "verilated_dpi.h"

#include "Vysyx_22041211_ALU__Syms.h"
#include "Vysyx_22041211_ALU___024root.h"

void Vysyx_22041211_ALU___024root___ctor_var_reset(Vysyx_22041211_ALU___024root* vlSelf);

Vysyx_22041211_ALU___024root::Vysyx_22041211_ALU___024root(Vysyx_22041211_ALU__Syms* symsp, const char* v__name)
    : VerilatedModule{v__name}
    , vlSymsp{symsp}
 {
    // Reset structure values
    Vysyx_22041211_ALU___024root___ctor_var_reset(this);
}

void Vysyx_22041211_ALU___024root::__Vconfigure(bool first) {
    if (false && first) {}  // Prevent unused
}

Vysyx_22041211_ALU___024root::~Vysyx_22041211_ALU___024root() {
}
