// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vysyx_22041211_ALU.h for the primary calling header

#include "verilated.h"
#include "verilated_dpi.h"

#include "Vysyx_22041211_ALU__Syms.h"
#include "Vysyx_22041211_ALU___024root.h"

extern "C" void ifebreak_func(int id_inst_i);

VL_INLINE_OPT void Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__ifebreak_func_TOP(const VerilatedScope* __Vscopep, const char* __Vfilenamep, IData/*31:0*/ __Vlineno, IData/*31:0*/ id_inst_i) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__ifebreak_func_TOP\n"); );
    // Body
    int id_inst_i__Vcvt;
    for (size_t id_inst_i__Vidx = 0; id_inst_i__Vidx < 1; ++id_inst_i__Vidx) id_inst_i__Vcvt = id_inst_i;
    Verilated::dpiContext(__Vscopep, __Vfilenamep, __Vlineno);
    ifebreak_func(id_inst_i__Vcvt);
}

extern "C" int pmem_read_task(int raddr, char wmask);

VL_INLINE_OPT void Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__pmem_read_task_TOP(IData/*31:0*/ raddr, CData/*7:0*/ wmask, IData/*31:0*/ &pmem_read_task__Vfuncrtn) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__pmem_read_task_TOP\n"); );
    // Body
    int raddr__Vcvt;
    for (size_t raddr__Vidx = 0; raddr__Vidx < 1; ++raddr__Vidx) raddr__Vcvt = raddr;
    char wmask__Vcvt;
    for (size_t wmask__Vidx = 0; wmask__Vidx < 1; ++wmask__Vidx) wmask__Vcvt = wmask;
    int pmem_read_task__Vfuncrtn__Vcvt;
    pmem_read_task__Vfuncrtn__Vcvt = pmem_read_task(raddr__Vcvt, wmask__Vcvt);
    pmem_read_task__Vfuncrtn = pmem_read_task__Vfuncrtn__Vcvt;
}

extern "C" void pmem_write_task(int waddr, int wdata, char wmask);

VL_INLINE_OPT void Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__pmem_write_task_TOP(IData/*31:0*/ waddr, IData/*31:0*/ wdata, CData/*7:0*/ wmask) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__pmem_write_task_TOP\n"); );
    // Body
    int waddr__Vcvt;
    for (size_t waddr__Vidx = 0; waddr__Vidx < 1; ++waddr__Vidx) waddr__Vcvt = waddr;
    int wdata__Vcvt;
    for (size_t wdata__Vidx = 0; wdata__Vidx < 1; ++wdata__Vidx) wdata__Vcvt = wdata;
    char wmask__Vcvt;
    for (size_t wmask__Vidx = 0; wmask__Vidx < 1; ++wmask__Vidx) wmask__Vcvt = wmask;
    pmem_write_task(waddr__Vcvt, wdata__Vcvt, wmask__Vcvt);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vysyx_22041211_ALU___024root___dump_triggers__ico(Vysyx_22041211_ALU___024root* vlSelf);
#endif  // VL_DEBUG

void Vysyx_22041211_ALU___024root___eval_triggers__ico(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_triggers__ico\n"); );
    // Body
    vlSelf->__VicoTriggered.at(0U) = (0U == vlSelf->__VicoIterCount);
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vysyx_22041211_ALU___024root___dump_triggers__ico(vlSelf);
    }
#endif
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vysyx_22041211_ALU___024root___dump_triggers__act(Vysyx_22041211_ALU___024root* vlSelf);
#endif  // VL_DEBUG

void Vysyx_22041211_ALU___024root___eval_triggers__act(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___eval_triggers__act\n"); );
    // Body
    vlSelf->__VactTriggered.at(0U) = ((IData)(vlSelf->clk) 
                                      & (~ (IData)(vlSelf->__Vtrigrprev__TOP__clk)));
    vlSelf->__Vtrigrprev__TOP__clk = vlSelf->clk;
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vysyx_22041211_ALU___024root___dump_triggers__act(vlSelf);
    }
#endif
}

VL_INLINE_OPT void Vysyx_22041211_ALU___024root___nba_sequent__TOP__0(Vysyx_22041211_ALU___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vysyx_22041211_ALU__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vysyx_22041211_ALU___024root___nba_sequent__TOP__0\n"); );
    // Init
    IData/*31:0*/ __Vfunc_ysyx_22041211_top__DOT__pmem_read_task__1__Vfuncout;
    __Vfunc_ysyx_22041211_top__DOT__pmem_read_task__1__Vfuncout = 0;
    // Body
    Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__pmem_read_task_TOP(
                                                                                ((IData)(4U) 
                                                                                + vlSelf->ysyx_22041211_top__DOT__id_pc_i), 0xfU, __Vfunc_ysyx_22041211_top__DOT__pmem_read_task__1__Vfuncout);
    vlSelf->ysyx_22041211_top__DOT__if_inst = __Vfunc_ysyx_22041211_top__DOT__pmem_read_task__1__Vfuncout;
    Vysyx_22041211_ALU___024root____Vdpiimwrap_ysyx_22041211_top__DOT__ifebreak_func_TOP(
                                                                                (&(vlSymsp->__Vscope_ysyx_22041211_top)), 
                                                                                "vsrc/ysyx_22041211_top.v", 0x47U, vlSelf->ysyx_22041211_top__DOT__if_inst);
    vlSelf->ysyx_22041211_top__DOT____VdfgTmp_heac76d33__0 
        = ((0U == (7U & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                         >> 0xcU))) | ((1U == (7U & 
                                               (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                >> 0xcU))) 
                                       | (2U == (7U 
                                                 & (vlSelf->ysyx_22041211_top__DOT__if_inst 
                                                    >> 0xcU)))));
    vlSelf->ysyx_22041211_top__DOT__id_pc_i = ((IData)(vlSelf->rst)
                                                ? 0x80000000U
                                                : vlSelf->ysyx_22041211_top__DOT__if_pc_next_i);
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
    vlSelf->pc = vlSelf->ysyx_22041211_top__DOT__id_pc_i;
    vlSelf->ysyx_22041211_top__DOT__if_pc_next_i = 
        ((IData)(4U) + vlSelf->ysyx_22041211_top__DOT__id_pc_i);
}
