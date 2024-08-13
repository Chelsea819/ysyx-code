/* verilator lint_off UNUSEDSIGNAL */

module ysyx_22041211_controller(
    input           [6:0]                         opcode,
    input           [2:0]                         func3,
    input           [6:0]                         func7,
    input           [5:0]                         shamt_F,
    output          [1:0]                         memToReg,
    output                                        memWrite, //写内存操作
    output          [2:0]                         branch,
    output          [1:0]                         jmp,
    output          [3:0]                         ALUcontrol,
    output                                        regWrite,
    output 			[2:0]				          DataLen 	,  // 0 1 3
	output								          DataSign	,
    output                                        ALUSrc
);
    wire            [1:0]                          ALUop;


    //memToReg
    //00---ALUresult
    //01---mem_data 读内存 I type
    //10---PC+4 jalr J
    //11---PC+imm  U2
    ysyx_22041211_MuxKeyWithDefault #(4, 7, 2) mux_memToReg (memToReg, opcode, 2'b00, {
        7'b0000011, 2'b01,  // I-type lb lh lw lbu lhu
        7'b1100111, 2'b10,  // jalr
        7'b1101111, 2'b10,  // J type
        7'b0010111, 2'b11   // U2 auipc      
    });

    //memWrite
    //写内存
    //S type
    ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) mux_memWrite (memWrite, opcode, 1'b0, {
        7'b0100011, 1'b1    //S type
    });

    //branch
    //指令跳转
    //B 
    //000 no branch
    //001 equal
    //010 unequal
    //011 <
    //100 >=
    ysyx_22041211_MuxKeyWithDefault #(6, 10, 3) mux_branch (branch, {opcode,func3}, 3'b0, {
        10'b1100011000, 3'b001,   //B-beq =
        10'b1100011001, 3'b010,   //B-bne =/
        10'b1100011100, 3'b011,   //B-blt <
        10'b1100011101, 3'b100,   //B-bge >=
        10'b1100011110, 3'b011,   //B-bltu <
        10'b1100011111, 3'b100    //B-beq >=
    });

    //jmp
    // J jal
    ysyx_22041211_MuxKeyWithDefault #(2, 7, 2) mux_jmp (jmp,opcode, 2'b0, {
        7'b1101111, 2'b01,   //J 
        7'b1100111, 2'b10    //jalr
    });
    
        

    //有符号数1
    assign DataSign = (opcode == 7'b00000011 && (func3 == 3'b000 || func3 == 3'b001)) ? 1'b1 : 1'b0;

    //0-1 1-2 3-4
    assign DataLen = (func3 == 3'b000 || func3 == 3'b100) ? 3'b001 : 
                     (func3 == 3'b001 || func3 == 3'b101) ? 3'b010 : 
                     (func3 == 3'b010 ) ? 3'b100 : 3'b000;
 
    //ALUop
    //00 +
    //01 -
    //10 func
    //11 func3 + func7
    ysyx_22041211_MuxKeyWithDefault #(5, 7, 2) mux_ALUop (ALUop, opcode, 2'b00, {
        7'b0000011, 2'b00,      //I lb lh lw lbu lhu
        7'b0100011, 2'b00,      //S sb sh sw
        7'b1100011, 2'b10,      //B beq
        7'b0010011, 2'b10,      //I addi
        7'b0110011, 2'b11       //R ok
    });


    assign ALUcontrol = (opcode == 7'b0110111) ? 4'b1010:                      //U lui src2
                        (ALUop == 2'b00)? 4'b0000:                             //根据op判断加法
                        (ALUop == 2'b01)? 4'b0001:                             //根据op判断减法
                        ({ALUop,func3,func7} == 12'b11_000_0000000)? 4'b0000:  //R + add
                        ({ALUop,func3,func7} == 12'b11_000_0100000)? 4'b0001:  //R - sub
                        ({ALUop,func3,func7} == 12'b110010000000)? 4'b0010:  //R << sll
                        ({ALUop,func3,func7} == 12'b110100000000)? 4'b0011:  //R <s slt
                        ({ALUop,func3,func7} == 12'b110110000000)? 4'b0100:  //R <u sltu
                        ({ALUop,func3,func7} == 12'b111000000000)? 4'b0101:  //R ^ xor
                        ({ALUop,func3,func7} == 12'b111010000000)? 4'b0110:  //R >>u srl
                        ({ALUop,func3,func7} == 12'b111010100000)? 4'b0111:  //R >>s sra
                        ({ALUop,func3,func7} == 12'b111100000000)? 4'b1000:  //R | or
                        ({ALUop,func3,func7} == 12'b111110000000)? 4'b1001:  //R & and
                        ({ALUop,branch,func3} == 8'b10_001_000)? 4'b0001:  //B + beq
                        ({ALUop,branch,func3} == 8'b10_010_001)? 4'b0001:  //B + bne
                        ({ALUop,branch,func3} == 8'b10_011_100)? 4'b0011:  //B + blt
                        ({ALUop,branch,func3} == 8'b10_100_101)? 4'b0011:  //B + bge
                        ({ALUop,branch,func3} == 8'b10_011_110)? 4'b0100:  //B + bltu
                        ({ALUop,branch,func3} == 8'b10_100_111)? 4'b0100:  //B + bgeu
                        ({ALUop,branch,func3} == 8'b10_000_000)? 4'b0000:  //I + addi
                        ({ALUop,branch,func3} == 8'b10_000_010)? 4'b0011:  //I <s slti
                        ({ALUop,branch,func3} == 8'b10_000_011)? 4'b0100:  //I <u sltiu
                        ({ALUop,branch,func3} == 8'b10_000_100)? 4'b0101:  //I ^ xori
                        ({ALUop,branch,func3} == 8'b10_000_110)? 4'b1000:  //I | ori
                        ({ALUop,branch,func3} == 8'b10_000_111)? 4'b1001:  //I & andi
                        ({ALUop,branch,func3,shamt_F} == 14'b10_000_001_000000)? 4'b1100:  //I << slli-shamt
                        ({ALUop,branch,func3,shamt_F} == 14'b10_000_101_000000)? 4'b1011:  //I >> srli-shamt
                        ({ALUop,branch,func3,shamt_F} == 14'b10_000_101_010000)? 4'b1101: 4'b1111;  //I >>> srai-shamt
                         



    //ALUSrc
    //0---reg_data2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
    //1---imm
    //I S U-lui
    ysyx_22041211_MuxKeyWithDefault #(4, 7, 1) mux_ALUSrc (ALUSrc, opcode, 1'b0, {
        7'b0000011, 1'b1,   //I lb lh lw lbu lhu
        7'b0010011, 1'b1,   //I addi
        7'b0110111, 1'b1,   //U lui
        7'b0100011, 1'b1    //S type
    });

    //regWrite
    //写寄存器
    //R I J
    ysyx_22041211_MuxKeyWithDefault #(7, 7, 1) mux_regWrite (regWrite, opcode, 1'b0, {
        7'b0110011, 1'b1,   //R type
        7'b0000011, 1'b1,   //I lb lh lw lbu lhu
        7'b0010011, 1'b1,   //I addi
        7'b1100111, 1'b1,   //I jalr
        7'b0110111, 1'b1,   //U lui
        7'b0010111, 1'b1,   //U auipc
        7'b1101111, 1'b1    //J
    });


endmodule

/* verilator lint_on UNUSEDSIGNAL */
