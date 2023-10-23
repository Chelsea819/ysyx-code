/* verilator lint_off UNUSEDSIGNAL */

module ysyx_22041211_controller #(parameter DATA_LEN = 32)(
    input           [6:0]                         opcode,
    input           [2:0]                         funct3,
    input           [6:0]                         funct7,
    output                                        memToReg,
    output                                        memWrite, //写内存操作
    output                                        branch,
    output          [3:0]                         ALUcontrol,
    output                                        regWrite,
    output                                        ALUSrc
);
    //memToReg
    ysyx_22041211_MuxKey #(2, 7, 1) mux_memToReg (memToReg, opcode, (
        
    ));




endmodule

/* verilator lint_on UNUSEDSIGNAL */
