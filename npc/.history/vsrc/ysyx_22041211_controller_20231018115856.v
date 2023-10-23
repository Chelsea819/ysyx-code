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
    wire            [1:0]                          ALUop;

    //memToReg
    //0---ALUresult
    //1---mem_data 读内存
    ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) mux_memToReg (memToReg, opcode, 1'b0, (
        7'b0000011, 1'b1
    ));

    //memWrite
    //写内存
    ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) mux_memWrite (memWrite, opcode, 1'b0, (
        7'b0100011, 1'b1
    ));

    //branch
    //指令跳转
    ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) mux_branch (branch, opcode, 1'b0, (
        7'b1100011, 1'b1
    ));

    //ALUop
    //00 +
    //01 -
    //10 func
    //11 X
    ysyx_22041211_MuxKeyWithDefault #(3, 7, 2) mux_ALUop (ALUop, opcode, 2'b11, (
        7'b0000011, 2'b00,
        7'b0100011, 2'b00,
        7'b1100011, 2'b01
    ));


    //ALUop
    //00 +
    //01 -
    //10 func
    //11 X
    ysyx_22041211_MuxKeyWithDefault #(3, 7, 2) mux_ALUop (ALUop, opcode, 2'b11, (
        7'b0000011, 2'b00,
        7'b0100011, 2'b00,
        7'b0110011, 2'b10,
        7'b1100011, 2'b01
    ));


    //ALUSrc
    //0---reg_data2
    //1---imm
    ysyx_22041211_MuxKeyWithDefault #(2, 7, 1) mux_ALUSrc (ALUSrc, opcode, 1'b0, (
        7'b0000011, 1'b1,
        7'b0100011, 1'b1
    ));

    //regWrite
    //写寄存器
    ysyx_22041211_MuxKeyWithDefault #(2, 7, 1) mux_regWrite (regWrite, opcode, 1'b0, (
        7'b0110011, 1'b1,
        7'b0000011, 1'b1
    ));


endmodule

/* verilator lint_on UNUSEDSIGNAL */
