// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vysyx_22041211_ALU.h for the primary calling header

#include "verilated.h"
#include "verilated_dpi.h"

#include "Vysyx_22041211_ALU__Syms.h"
#include "Vysyx_22041211_ALU___024root.h"

#ifdef VL_DEBUG
VL_ATTR_COLD void Vysyx_22041211_ALU___024root___dump_triggers__stl(Vysyx_22041211_ALU___024root* vlSelf);
#endif  // VL_DEBUG

VL_ATTR_COLD void Vysyx_22041211_ALU___024root___eval_triggers__stl(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_triggers__stl\n"); );
    // Body
    vlSelf->__VstlTriggered.at(0U) = (0U == vlSelf->__VstlIterCount);
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vysyx_22041211_ALU___024root___dump_triggers__stl(vlSelf);
    }
#endif
}

void Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__ifebreak_func_TOP(const VerilatedScope* __Vscopep, const char* __Vfilenamep, IData/*31:0*/ __Vlineno, IData/*31:0*/ id_inst_i);

VL_ATTR_COLD void Vysyx_22041211_ALU___024root___stl_sequent__TOP__0(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___stl_sequent__TOP__0\n"); );
    // Body
    Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__ifebreak_func_TOP(
                                                                                (&(vlSymsp->__Vscope_ysyx_22041211_top)), 
                                                                                "vsrc/ysyx_22041211_top.v", 0x47U, vlSelf->ysyx_22041211_top__DOT__if_inst);
    vlSelf->pc = vlSelf->ysyx_22041211_top__DOT__id_pc_i;
    vlSelf->ysyx_22041211_top__DOT__if_pc_next_i = 
        ((IData)(4U) + vlSelf->ysyx_22041211_top__DOT__id_pc_i);
    vlSelf->ysyx_22041211_top__DOT____VdfgTmp_heac76d33__0 
        = ((0U == (7U & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                         >> 0xcU))) | ((1U == (7U & 
                                               (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                >> 0xcU))) 
                                       | (2U == (7U 
                                                 & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                    >> 0xcU)))));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__key_list[0U] 
        = (1U & ((IData)(vlSelf->lut) >> 1U));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__key_list[1U] 
        = (1U & ((IData)(vlSelf->lut) >> 3U));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__data_list[0U] 
        = (1U & (IData)(vlSelf->lut));
    vlSelf->ysyx_22041211_MuxKey__DOT__i0__DOT__data_list[1U] 
        = (1U & ((IData)(vlSelf->lut) >> 2U));
    vlSelf->invalid = (1U & (~ ((0x17U == (0x7fU & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                | ((0x37U == (0x7fU 
                                              & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                   | ((0x6fU == (0x7fU 
                                                 & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                      | ((0x67U == 
                                          ((0x380U 
                                            & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                               >> 5U)) 
                                           | (0x7fU 
                                              & vlSelf->ysyx_22041211_top__DOT__if_inst))) 
                                         | ((0x63U 
                                             == (0x7fU 
                                                 & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                            | (((3U 
                                                 == 
                                                 (0x7fU 
                                                  & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                                & ((IData)(vlSelf->ysyx_22041211_top__DOT____VdfgTmp_heac76d33__0) 
                                                   | ((4U 
                                                       == 
                                                       (7U 
                                                        & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                           >> 0xcU))) 
                                                      | (5U 
                                                         == 
                                                         (7U 
                                                          & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                             >> 0xcU)))))) 
                                               | (((0x23U 
                                                    == 
                                                    (0x7fU 
                                                     & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                                   & (IData)(vlSelf->ysyx_22041211_top__DOT____VdfgTmp_heac76d33__0)) 
                                                  | (((0x13U 
                                                       == 
                                                       (0x7fU 
                                                        & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                                      & ((0U 
                                                          == 
                                                          (7U 
                                                           & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                              >> 0xcU))) 
                                                         | ((2U 
                                                             == 
                                                             (7U 
                                                              & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                 >> 0xcU))) 
                                                            | ((3U 
                                                                == 
                                                                (7U 
                                                                 & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                    >> 0xcU))) 
                                                               | ((4U 
                                                                   == 
                                                                   (7U 
                                                                    & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                       >> 0xcU))) 
                                                                  | ((6U 
                                                                      == 
                                                                      (7U 
                                                                       & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                          >> 0xcU))) 
                                                                     | (7U 
                                                                        == 
                                                                        (7U 
                                                                         & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                            >> 0xcU))))))))) 
                                                     | (((0x13U 
                                                          == 
                                                          (0x7fU 
                                                           & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                                         & ((IData)(
                                                                    (0x1000U 
                                                                     == 
                                                                     (0xfc007000U 
                                                                      & vlSelf->ysyx_22041211_top__DOT__if_inst))) 
                                                            | (IData)(
                                                                      ((0x5000U 
                                                                        == 
                                                                        (0x7000U 
                                                                         & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                                                       & ((0U 
                                                                           == 
                                                                           (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                            >> 0x1aU)) 
                                                                          | (0x10U 
                                                                             == 
                                                                             (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                                              >> 0x1aU))))))) 
                                                        | ((0x33U 
                                                            == 
                                                            (0x7fU 
                                                             & vlSelf->ysyx_22041211_top__DOT__if_inst)) 
                                                           | (0x100073U 
                                                              == vlSelf->ysyx_22041211_top__DOT__if_inst)))))))))))));
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
