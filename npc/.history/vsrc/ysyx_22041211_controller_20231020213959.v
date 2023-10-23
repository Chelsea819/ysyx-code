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
    //11 func3 + func7
    ysyx_22041211_MuxKeyWithDefault #(3, 7, 2) mux_ALUop (ALUop, opcode, 2'b00, (
        7'b0000011, 2'b00,      //I lb lh lw lbu lhu
        7'b0100011, 2'b00,      //S sb sh sw
        7'b1100011, 2'b01,      //B beq
        7'b0110011, 2'b11       //R 
        
    ));


    //ALUControl[3:1]


    //ALUControl[0]
    ysyx_22041211_MuxKeyWithDefault #(3, 7, 2) mux_ALUControl_0 (ALUControl[0], funct7, 2'b11, (
        
    ));

    ALUControl = (ALUop == 2'b00)? 4'b0000:                             //根据op判断加法
                 (ALUop == 2'b01)? 4'b0001:                             //根据op判断减法
                 ({ALUop,funct3,funct7} == 12'b110000000000)? 4'b0000:  //R + add
                 ({ALUop,funct3,funct7} == 12'b110000000001)? 4'b0001:  //R - sub
                 ({ALUop,funct3,funct7} == 12'b110010000000)? 4'b0110:  //R << sll
                 ({ALUop,funct3,funct7} == 12'b110100000000)? 4'b0011:  //R <s slt
                 ({ALUop,funct3,funct7} == 12'b110110000000)? 4'b0100:  //R <u sltu
                 ({ALUop,funct3,funct7} == 12'b111000000000)? 4'b0101:  //R ^ xor
                 ({ALUop,funct3,funct7} == 12'b111010000000)? 4'b0110:  //R >>u srl
                 ({ALUop,funct3,funct7} == 12'b111010100000)? 4'b0111:  //R >>s sra
                 ({ALUop,funct3,funct7} == 12'b111100000000)? 4'b0111:  //R | or
                 ({ALUop,funct3,funct7} == 12'b111110000000)? 4'b0111: 4'b1111;  //R & and



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
