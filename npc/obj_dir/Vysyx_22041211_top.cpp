// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Model implementation (design independent parts)

#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top__Syms.h"
#include "verilated_dpi.h"

//============================================================
// Constructors

Vysyx_22041211_top::Vysyx_22041211_top(VerilatedContext* _vcontextp__, const char* _vcname__)
    : VerilatedModel{*_vcontextp__}
    , vlSymsp{new Vysyx_22041211_top__Syms(contextp(), _vcname__, this)}
    , clk{vlSymsp->TOP.clk}
    , rst{vlSymsp->TOP.rst}
    , invalid{vlSymsp->TOP.invalid}
    , pc{vlSymsp->TOP.pc}
    , rootp{&(vlSymsp->TOP)}
{
    // Register model with the context
    contextp()->addModel(this);
}

Vysyx_22041211_top::Vysyx_22041211_top(const char* _vcname__)
    : Vysyx_22041211_top(Verilated::threadContextp(), _vcname__)
{
}

//============================================================
// Destructor

Vysyx_22041211_top::~Vysyx_22041211_top() {
    delete vlSymsp;
}

//============================================================
// Evaluation function

#ifdef VL_DEBUG
void Vysyx_22041211_top___024root___eval_debug_assertions(Vysyx_22041211_top___024root* vlSelf);
#endif  // VL_DEBUG
void Vysyx_22041211_top___024root___eval_static(Vysyx_22041211_top___024root* vlSelf);
void Vysyx_22041211_top___024root___eval_initial(Vysyx_22041211_top___024root* vlSelf);
void Vysyx_22041211_top___024root___eval_settle(Vysyx_22041211_top___024root* vlSelf);
void Vysyx_22041211_top___024root___eval(Vysyx_22041211_top___024root* vlSelf);

void Vysyx_22041211_top::eval_step() {
    VL_DEBUG_IF(VL_DBG_MSGF("+++++TOP Evaluate Vysyx_22041211_top::eval_step\n"); );
#ifdef VL_DEBUG
    // Debug assertions
    Vysyx_22041211_top___024root___eval_debug_assertions(&(vlSymsp->TOP));
#endif  // VL_DEBUG
    vlSymsp->__Vm_deleter.deleteAll();
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) {
        vlSymsp->__Vm_didInit = true;
        VL_DEBUG_IF(VL_DBG_MSGF("+ Initial\n"););
        Vysyx_22041211_top___024root___eval_static(&(vlSymsp->TOP));
        Vysyx_22041211_top___024root___eval_initial(&(vlSymsp->TOP));
        Vysyx_22041211_top___024root___eval_settle(&(vlSymsp->TOP));
    }
    // MTask 0 start
    VL_DEBUG_IF(VL_DBG_MSGF("MTask0 starting\n"););
    Verilated::mtaskId(0);
    VL_DEBUG_IF(VL_DBG_MSGF("+ Eval\n"););
    Vysyx_22041211_top___024root___eval(&(vlSymsp->TOP));
    // Evaluate cleanup
    Verilated::endOfThreadMTask(vlSymsp->__Vm_evalMsgQp);
    Verilated::endOfEval(vlSymsp->__Vm_evalMsgQp);
}

//============================================================
// Events and timing
bool Vysyx_22041211_top::eventsPending() { return false; }

uint64_t Vysyx_22041211_top::nextTimeSlot() {
    VL_FATAL_MT(__FILE__, __LINE__, "", "%Error: No delays in the design");
    return 0;
}

//============================================================
// Utilities

const char* Vysyx_22041211_top::name() const {
    return vlSymsp->name();
}

//============================================================
// Invoke final blocks

void Vysyx_22041211_top___024root___eval_final(Vysyx_22041211_top___024root* vlSelf);

VL_ATTR_COLD void Vysyx_22041211_top::final() {
    Vysyx_22041211_top___024root___eval_final(&(vlSymsp->TOP));
}

//============================================================
// Implementations of abstract methods from VerilatedModel

const char* Vysyx_22041211_top::hierName() const { return vlSymsp->name(); }
const char* Vysyx_22041211_top::modelName() const { return "Vysyx_22041211_top"; }
unsigned Vysyx_22041211_top::threads() const { return 1; }
