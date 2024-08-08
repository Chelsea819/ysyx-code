// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vysyx_22041211_ALU.h for the primary calling header

#include "verilated.h"
#include "verilated_dpi.h"

#include "Vysyx_22041211_ALU___024root.h"

VL_INLINE_OPT void Vysyx_22041211_ALU___024root___ico_sequent__TOP__0(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___ico_sequent__TOP__0\n"); );
    // Body
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__key_list[0U] 
        = (1U & ((IData)(vlSelf->lut) >> 1U));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__key_list[1U] 
        = (1U & ((IData)(vlSelf->lut) >> 3U));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__data_list[0U] 
        = (1U & (IData)(vlSelf->lut));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__data_list[1U] 
        = (1U & ((IData)(vlSelf->lut) >> 2U));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__lut_out 
        = (((IData)(vlSelf->key) == vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__key_list
            [0U]) & vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__data_list
           [0U]);
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__lut_out 
        = ((IData)(vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__lut_out) 
           | (((IData)(vlSelf->key) == vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__key_list
               [1U]) & vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__data_list
              [1U]));
    vlSelf->out = vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__lut_out;
}

void Vysyx_22041211_ALU___024root___eval_ico(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_ico\n"); );
    // Body
    if (vlSelf->__VicoTriggered.at(0U)) {
        Vysyx_22041211_ALU___024root___ico_sequent__TOP__0(vlSelf);
    }
}

void Vysyx_22041211_ALU___024root___eval_act(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_act\n"); );
}

void Vysyx_22041211_ALU___024root___nba_sequent__TOP__0(Vysyx_22041211_ALU___024root* vlSelf);

void Vysyx_22041211_ALU___024root___eval_nba(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_nba\n"); );
    // Body
    if (vlSelf->__VnbaTriggered.at(0U)) {
        Vysyx_22041211_ALU___024root___nba_sequent__TOP__0(vlSelf);
    }
}

void Vysyx_22041211_ALU___024root___eval_triggers__ico(Vysyx_22041211_ALU___024root* vlSelf);
#ifdef VL_DEBUG
VL_ATTR_COLD void Vysyx_22041211_ALU___024root___dump_triggers__ico(Vysyx_22041211_ALU___024root* vlSelf);
#endif  // VL_DEBUG
void Vysyx_22041211_ALU___024root___eval_triggers__act(Vysyx_22041211_ALU___024root* vlSelf);
#ifdef VL_DEBUG
VL_ATTR_COLD void Vysyx_22041211_ALU___024root___dump_triggers__act(Vysyx_22041211_ALU___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void Vysyx_22041211_ALU___024root___dump_triggers__nba(Vysyx_22041211_ALU___024root* vlSelf);
#endif  // VL_DEBUG

void Vysyx_22041211_ALU___024root___eval(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval\n"); );
    // Init
    CData/*0:0*/ __VicoContinue;
    VlTriggerVec<1> __VpreTriggered;
    IData/*31:0*/ __VnbaIterCount;
    CData/*0:0*/ __VnbaContinue;
    // Body
    vlSelf->__VicoIterCount = 0U;
    __VicoContinue = 1U;
    while (__VicoContinue) {
        __VicoContinue = 0U;
        Vysyx_22041211_ALU___024root___eval_triggers__ico(vlSelf);
        if (vlSelf->__VicoTriggered.any()) {
            __VicoContinue = 1U;
            if (VL_UNLIKELY((0x64U < vlSelf->__VicoIterCount))) {
#ifdef VL_DEBUG
                Vysyx_22041211_ALU___024root___dump_triggers__ico(vlSelf);
#endif
                VL_FATAL_MT("vsrc/ysyx_22041211_MuxKey.v", 8, "", "Input combinational region did not converge.");
            }
            vlSelf->__VicoIterCount = ((IData)(1U) 
                                       + vlSelf->__VicoIterCount);
            Vysyx_22041211_ALU___024root___eval_ico(vlSelf);
        }
    }
    __VnbaIterCount = 0U;
    __VnbaContinue = 1U;
    while (__VnbaContinue) {
        __VnbaContinue = 0U;
        vlSelf->__VnbaTriggered.clear();
        vlSelf->__VactIterCount = 0U;
        vlSelf->__VactContinue = 1U;
        while (vlSelf->__VactContinue) {
            vlSelf->__VactContinue = 0U;
            Vysyx_22041211_ALU___024root___eval_triggers__act(vlSelf);
            if (vlSelf->__VactTriggered.any()) {
                vlSelf->__VactContinue = 1U;
                if (VL_UNLIKELY((0x64U < vlSelf->__VactIterCount))) {
#ifdef VL_DEBUG
                    Vysyx_22041211_ALU___024root___dump_triggers__act(vlSelf);
#endif
                    VL_FATAL_MT("vsrc/ysyx_22041211_MuxKey.v", 8, "", "Active region did not converge.");
                }
                vlSelf->__VactIterCount = ((IData)(1U) 
                                           + vlSelf->__VactIterCount);
                __VpreTriggered.andNot(vlSelf->__VactTriggered, vlSelf->__VnbaTriggered);
                vlSelf->__VnbaTriggered.set(vlSelf->__VactTriggered);
                Vysyx_22041211_ALU___024root___eval_act(vlSelf);
            }
        }
        if (vlSelf->__VnbaTriggered.any()) {
            __VnbaContinue = 1U;
            if (VL_UNLIKELY((0x64U < __VnbaIterCount))) {
#ifdef VL_DEBUG
                Vysyx_22041211_ALU___024root___dump_triggers__nba(vlSelf);
#endif
                VL_FATAL_MT("vsrc/ysyx_22041211_MuxKey.v", 8, "", "NBA region did not converge.");
            }
            __VnbaIterCount = ((IData)(1U) + __VnbaIterCount);
            Vysyx_22041211_ALU___024root___eval_nba(vlSelf);
        }
    }
}

#ifdef VL_DEBUG
void Vysyx_22041211_ALU___024root___eval_debug_assertions(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_debug_assertions\n"); );
    // Body
    if (VL_UNLIKELY((vlSelf->key & 0xfeU))) {
        Verilated::overWidthError("key");}
    if (VL_UNLIKELY((vlSelf->lut & 0xf0U))) {
        Verilated::overWidthError("lut");}
    if (VL_UNLIKELY((vlSelf->clk & 0xfeU))) {
        Verilated::overWidthError("clk");}
    if (VL_UNLIKELY((vlSelf->rst & 0xfeU))) {
        Verilated::overWidthError("rst");}
}
#endif  // VL_DEBUG
