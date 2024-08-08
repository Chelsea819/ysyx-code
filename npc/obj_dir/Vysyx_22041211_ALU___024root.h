// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vysyx_22041211_ALU.h for the primary calling header

#ifndef VERILATED_VYSYX_22041211_ALU___024ROOT_H_
#define VERILATED_VYSYX_22041211_ALU___024ROOT_H_  // guard

#include "verilated.h"

class Vysyx_22041211_ALU__Syms;

class Vysyx_22041211_ALU___024root final : public VerilatedModule {
  public:

    // DESIGN SPECIFIC STATE
    VL_IN8(clk,0,0);
    VL_OUT8(out,0,0);
    VL_IN8(key,0,0);
    VL_IN8(lut,3,0);
    VL_IN8(rst,0,0);
    VL_OUT8(invalid,0,0);
    CData/*0:0*/ ysyx_22041211_MuxKey__DOT__i0__DOT__lut_out;
    CData/*0:0*/ ysyx_22041211_top__DOT____VdfgTmp_heac76d33__0;
    CData/*0:0*/ __Vtrigrprev__TOP__clk;
    CData/*0:0*/ __VactContinue;
    VL_OUT(pc,31,0);
    IData/*31:0*/ ysyx_22041211_top__DOT__if_pc_next_i;
    IData/*31:0*/ ysyx_22041211_top__DOT__if_inst;
    IData/*31:0*/ ysyx_22041211_top__DOT__id_pc_i;
    IData/*31:0*/ __VstlIterCount;
    IData/*31:0*/ __VicoIterCount;
    IData/*31:0*/ __VactIterCount;
    VlUnpacked<CData/*0:0*/, 2> ysyx_22041211_MuxKey__DOT__i0__DOT__key_list;
    VlUnpacked<CData/*0:0*/, 2> ysyx_22041211_MuxKey__DOT__i0__DOT__data_list;
    VlTriggerVec<1> __VstlTriggered;
    VlTriggerVec<1> __VicoTriggered;
    VlTriggerVec<1> __VactTriggered;
    VlTriggerVec<1> __VnbaTriggered;

    // INTERNAL VARIABLES
    Vysyx_22041211_ALU__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vysyx_22041211_ALU___024root(Vysyx_22041211_ALU__Syms* symsp, const char* v__name);
    ~Vysyx_22041211_ALU___024root();
    VL_UNCOPYABLE(Vysyx_22041211_ALU___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
} VL_ATTR_ALIGNED(VL_CACHE_LINE_BYTES);


#endif  // guard
