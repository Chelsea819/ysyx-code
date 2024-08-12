`include "./ysyx_22041211_define.v"
module ysyx_22041211_decoder(
    input           [31:0]                        inst_i                    ,
    input           [31:0]                        reg1_data_i               ,
    input           [31:0]                        reg2_data_i               ,
    // output          [1:0]                         memToReg,
    // output                                        memWrite, //写内存操作
    // output          [2:0]                         branch,
    // output          [1:0]                         jmp,
    // output          [3:0]                         ALUcontrol,
    input           [31:0]                        pc_i                      ,
    output          [3:0]                         aluop_o                   ,
    output          [3:0]                         alusel_o                  ,
    output          [31:0]                        pc_o                      ,
    output          [31:0]                        reg1_o                    ,
    output          [31:0]                        reg2_o                    ,
    output		                		          wd_o                      ,
    output		    [4:0]		                  wreg_o                    ,
    output          [4:0]                         reg1_addr_o               ,
    output          [4:0]                         reg2_addr_o               ,
    output          [2:0]                         branch_type_o             ,
    output          [31:0]                        branch_target_o           ,
    output          [1:0]                         store_type_o              ,
    output          [2:0]                         load_type_o              ,
    output                                        jmp_flag_o                ,
    output          [31:0]                        jmp_target_o              ,
    // output                                        reg2_read_o   ,
    // output          [31:0]                        inst_o        ,
    output          [31:0]                        imm_o         
    // output 			[2:0]				          DataLen 	,  // 0 1 3
	// output								          DataSign	,
);
    wire            [6:0]                          func7;
    wire            [2:0]                          func3;
    wire            [6:0]                          opcode;
    wire            [31:0]                         imm;

    assign func3 = inst_i[14:12];
    assign func7 = inst_i[31:25];
    assign opcode = inst_i[6:0];
    // assign inst_o = inst_i;
    assign pc_o = pc_i;
    assign imm_o = imm;
    assign reg1_o = reg1_data_i;
    assign reg2_o = reg2_data_i;
    assign wreg_o = inst_i[11:7];
    assign reg1_addr_o = inst_i[19:15];
    assign reg2_addr_o = inst_i[24:20];
    assign branch_target_o = pc_i + imm;

    
// controller look-up lut
// inst: R-type: wd_o aluop_o alusel_o
// alu_sel  [1:0] res1/pc/0, [3:2] res2/imm/4/0
// alu_op + - 
// ysyx_22041211_MuxKeyWithDefault #(2,17,9) lut_mux ( {wd_o, aluop_o, alusel_o}, {opcode, func3, func7}, 9'b0, {
//             {7'b0000011, 3'b00, 7'b00}, {1'b1,4'b0000,4'b0000},				// add
// 			{7'b0010011, 3'b00, 7'b00}, {1'b1,4'b0000,4'b0000}              // addi
//     });

assign {wd_o, aluop_o, alusel_o, store_type_o, load_type_o} = ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_ADD_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-add
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_SUB_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-sub
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_XOR_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_XOR, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-xor
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_OR_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_OR, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-or
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_AND_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_AND, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-and
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_SLL_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_LEFT_LOGIC, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-sll
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_SRL_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_RIGHT_LOGIC, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-srl
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_SRA_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_RIGHT_ARITH, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-sra
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_SLT_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_LESS_SIGNED, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-slt
                                                              ({opcode, func3, func7}       == {`TYPE_R_OPCODE, `TYPE_R_SLTU_FUNC})        ? {`EN_REG_WRITE, `ALU_OP_LESS_UNSIGNED, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}} :        // R-sltu
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_ADDI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-addi
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_XORI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_XOR, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-xori
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_ORI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_OR, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-ori
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_ANDI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_AND, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-andi
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_SLTI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_LESS_SIGNED, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-ori
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_SLTIU_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_LESS_UNSIGNED, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-andi
                                                              ({opcode, func3}              == {`TYPE_I_JALR_OPCODE, `TYPE_I_JALR_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_4,`ALU_SEL1_PC, `STORE_INVALID, `LOAD_INVALID}}  :            // I-jalr
                                                              ({opcode, func3, imm[11:5]}   == {`TYPE_I_BASE_OPCODE, `TYPE_I_SLLI_FUNC3_IMM}) ? {`EN_REG_WRITE, `ALU_OP_LEFT_LOGIC, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-slli
                                                              ({opcode, func3, imm[11:5]}   == {`TYPE_I_BASE_OPCODE, `TYPE_I_SRLI_FUNC3_IMM}) ? {`EN_REG_WRITE, `ALU_OP_RIGHT_LOGIC, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-slli
                                                              ({opcode, func3, imm[11:5]}   == {`TYPE_I_BASE_OPCODE, `TYPE_I_SRAI_FUNC3_IMM}) ? {`EN_REG_WRITE, `ALU_OP_RIGHT_ARITH, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-slli
                                                              ({opcode, func3}              == {`TYPE_S_OPCODE, `TYPE_S_SB_FUNC3})        ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_SB_8, `LOAD_INVALID}}  :         // S-sb
                                                              ({opcode, func3}              == {`TYPE_S_OPCODE, `TYPE_S_SH_FUNC3})        ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_SH_16, `LOAD_INVALID}}  :        // S-sh
                                                              ({opcode, func3}              == {`TYPE_S_OPCODE, `TYPE_S_SW_FUNC3})        ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_SW_32, `LOAD_INVALID}}  :        // S-sw
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LB_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LB_8}}  :         // L-sb
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LH_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LH_16}}  :        // L-sh
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LW_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LW_32}}  :        // L-sw
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LBU_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LBU_8}}  :         // L-sb
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LHU_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LHU_16}}  :        // L-sh
                                                              ({opcode}                     == {`TYPE_J_JAL_OPCODE})                      ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_4,`ALU_SEL1_PC, `STORE_INVALID, `LOAD_INVALID}}  :            // J-jal
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BEQ_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BNE_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BLT_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BGE_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BLTU_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BGEU_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode}                     == {`TYPE_U_AUIPC_OPCODE})                    ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_PC, `STORE_INVALID, `LOAD_INVALID}}    :        // U-auipc
                                                              ({opcode}                     == {`TYPE_U_LUI_OPCODE})                      ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_ZERO, `STORE_INVALID, `LOAD_INVALID}}  : 
                                                              0;  // U-lui          

assign  {branch_type_o, jmp_target_o, jmp_flag_o} = ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BEQ_FUNC3})             ? {`BRANCH_BEQ,     32'b0,             ~`EN_JMP}:         // B-beq
                                                    ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BNE_FUNC3})             ? {`BRANCH_BNE,     32'b0,             ~`EN_JMP}:         // B-beq
                                                    ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BLT_FUNC3})             ? {`BRANCH_BLT,     32'b0,             ~`EN_JMP}:         // B-beq
                                                    ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BGE_FUNC3})             ? {`BRANCH_BGE,     32'b0,             ~`EN_JMP}:         // B-beq
                                                    ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BLTU_FUNC3})            ? {`BRANCH_BLTU,    32'b0,             ~`EN_JMP}:         // B-beq
                                                    ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BGEU_FUNC3})            ? {`BRANCH_BGEU,    32'b0,             ~`EN_JMP}:         // B-beq
                                                    ({opcode, func3} == {`TYPE_I_JALR_OPCODE, `TYPE_I_JALR_FUNC3})       ? {`BRANCH_INVALID, reg1_data_i + imm, `EN_JMP} :         // I-jalr
                                                    ({opcode}        == {`TYPE_J_JAL_OPCODE})                            ? {`BRANCH_INVALID, pc_i + imm,        `EN_JMP} :         // J-jal 
                                                    0;       

ysyx_22041211_immGen my_gen (
    .inst       (inst_i),
    .imm        (imm)
);

    // ysyx_22041211_MuxKeyWithDefault #(4, 7, 2) mux_memToReg ({}, opcode, 2'b00, {
    //     7'b0000011, 2'b01,  // I-type lb lh lw lbu lhu
    //     7'b1100111, 2'b10,  // jalr
    //     7'b1101111, 2'b10,  // J type
    //     7'b0010111, 2'b11   // U2 auipc      
    // });


    // //memToReg
    // //00---ALUresult
    // //01---mem_data 读内存 I type
    // //10---PC+4 jalr J
    // //11---PC+imm  U2
    // ysyx_22041211_MuxKeyWithDefault #(4, 7, 2) mux_memToReg (memToReg, opcode, 2'b00, {
    //     7'b0000011, 2'b01,  // I-type lb lh lw lbu lhu
    //     7'b1100111, 2'b10,  // jalr
    //     7'b1101111, 2'b10,  // J type
    //     7'b0010111, 2'b11   // U2 auipc      
    // });

    // //memWrite
    // //写内存
    // //S type
    // ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) mux_memWrite (memWrite, opcode, 1'b0, {
    //     7'b0100011, 1'b1    //S type
    // });

    // //branch
    // //指令跳转
    // //B 
    // //000 no branch
    // //001 equal
    // //010 unequal
    // //011 <
    // //100 >=
    // ysyx_22041211_MuxKeyWithDefault #(6, 10, 3) mux_branch (branch, {opcode,func3}, 3'b0, {
    //     10'b1100011000, 3'b001,   //B-beq =
    //     10'b1100011001, 3'b010,   //B-bne =/
    //     10'b1100011100, 3'b011,   //B-blt <
    //     10'b1100011101, 3'b100,   //B-bge >=
    //     10'b1100011110, 3'b011,   //B-bltu <
    //     10'b1100011111, 3'b100    //B-beq >=
    // });

    // //jmp
    // // J jal
    // ysyx_22041211_MuxKeyWithDefault #(2, 7, 2) mux_jmp (jmp,opcode, 2'b0, {
    //     7'b1101111, 2'b01,   //J 
    //     7'b1100111, 2'b10    //jalr
    // });
    
        

    // //有符号数1
    // assign DataSign = (opcode == 7'b00000011 && (func3 == 3'b000 || func3 == 3'b001)) ? 1'b1 : 1'b0;

    // //0-1 1-2 3-4
    // assign DataLen = (func3 == 3'b000 || func3 == 3'b100) ? 3'b001 : 
    //                  (func3 == 3'b001 || func3 == 3'b101) ? 3'b010 : 
    //                  (func3 == 3'b010 ) ? 3'b100 : 3'b000;
 
    // //ALUop
    // //00 +
    // //01 -
    // //10 func
    // //11 func3 + func7
    // ysyx_22041211_MuxKeyWithDefault #(5, 7, 2) mux_ALUop (ALUop, opcode, 2'b00, {
    //     7'b0000011, 2'b00,      //I lb lh lw lbu lhu
    //     7'b0100011, 2'b00,      //S sb sh sw
    //     7'b1100011, 2'b10,      //B beq
    //     7'b0010011, 2'b10,      //I addi
    //     7'b0110011, 2'b11       //R ok
    // });


    // assign ALUcontrol = (opcode == 7'b0110111) ? 4'b1010:                      //U lui src2
    //                     (ALUop == 2'b00)? 4'b0000:                             //根据op判断加法
    //                     (ALUop == 2'b01)? 4'b0001:                             //根据op判断减法
    //                     ({ALUop,func3,func7} == 12'b11_000_0000000)? 4'b0000:  //R + add
    //                     ({ALUop,func3,func7} == 12'b11_000_0100000)? 4'b0001:  //R - sub
    //                     ({ALUop,func3,func7} == 12'b110010000000)? 4'b0010:  //R << sll
    //                     ({ALUop,func3,func7} == 12'b110100000000)? 4'b0011:  //R <s slt
    //                     ({ALUop,func3,func7} == 12'b110110000000)? 4'b0100:  //R <u sltu
    //                     ({ALUop,func3,func7} == 12'b111000000000)? 4'b0101:  //R ^ xor
    //                     ({ALUop,func3,func7} == 12'b111010000000)? 4'b0110:  //R >>u srl
    //                     ({ALUop,func3,func7} == 12'b111010100000)? 4'b0111:  //R >>s sra
    //                     ({ALUop,func3,func7} == 12'b111100000000)? 4'b1000:  //R | or
    //                     ({ALUop,func3,func7} == 12'b111110000000)? 4'b1001:  //R & and
    //                     ({ALUop,branch,func3} == 8'b10_001_000)? 4'b0001:  //B + beq
    //                     ({ALUop,branch,func3} == 8'b10_010_001)? 4'b0001:  //B + bne
    //                     ({ALUop,branch,func3} == 8'b10_011_100)? 4'b0011:  //B + blt
    //                     ({ALUop,branch,func3} == 8'b10_100_101)? 4'b0011:  //B + bge
    //                     ({ALUop,branch,func3} == 8'b10_011_110)? 4'b0100:  //B + bltu
    //                     ({ALUop,branch,func3} == 8'b10_100_111)? 4'b0100:  //B + bgeu
    //                     ({ALUop,branch,func3} == 8'b10_000_000)? 4'b0000:  //I + addi
    //                     ({ALUop,branch,func3} == 8'b10_000_010)? 4'b0011:  //I <s slti
    //                     ({ALUop,branch,func3} == 8'b10_000_011)? 4'b0100:  //I <u sltiu
    //                     ({ALUop,branch,func3} == 8'b10_000_100)? 4'b0101:  //I ^ xori
    //                     ({ALUop,branch,func3} == 8'b10_000_110)? 4'b1000:  //I | ori
    //                     ({ALUop,branch,func3} == 8'b10_000_111)? 4'b1001:  //I & andi
    //                     ({ALUop,branch,func3,shamt_F} == 14'b10_000_001_000000)? 4'b1100:  //I << slli-shamt
    //                     ({ALUop,branch,func3,shamt_F} == 14'b10_000_101_000000)? 4'b1011:  //I >> srli-shamt
    //                     ({ALUop,branch,func3,shamt_F} == 14'b10_000_101_010000)? 4'b1101: 4'b1111;  //I >>> srai-shamt
                         



    // //ALUSrc
    // //0---reg_data2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
    // //1---imm
    // //I S U-lui
    // ysyx_22041211_MuxKeyWithDefault #(4, 7, 1) mux_ALUSrc (ALUSrc, opcode, 1'b0, {
    //     7'b0000011, 1'b1,   //I lb lh lw lbu lhu
    //     7'b0010011, 1'b1,   //I addi
    //     7'b0110111, 1'b1,   //U lui
    //     7'b0100011, 1'b1    //S type
    // });

    // //wd_o
    // //写寄存器
    // //R I J
    // ysyx_22041211_MuxKeyWithDefault #(7, 7, 1) mux_regWrite (wd_o, opcode, 1'b0, {
    //     7'b0110011, 1'b1,   //R type
    //     7'b0000011, 1'b1,   //I lb lh lw lbu lhu
    //     7'b0010011, 1'b1,   //I addi
    //     7'b1100111, 1'b1,   //I jalr
    //     7'b0110111, 1'b1,   //U lui
    //     7'b0010111, 1'b1,   //U auipc
    //     7'b1101111, 1'b1    //J
    // });


endmodule

