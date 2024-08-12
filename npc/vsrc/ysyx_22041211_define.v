// DECODER
`define TYPE_R_OPCODE 7'b0110011
`define TYPE_R_ADD_FUNC  10'b000_000_0000
`define TYPE_R_SUB_FUNC  10'b000_010_0000
`define TYPE_R_XOR_FUNC  10'b100_000_0000
`define TYPE_R_OR_FUNC   10'b110_000_0000
`define TYPE_R_AND_FUNC  10'b111_000_0000
`define TYPE_R_SLL_FUNC  10'b001_000_0000
`define TYPE_R_SRL_FUNC  10'b101_000_0000
`define TYPE_R_SRA_FUNC  10'b101_010_0000
`define TYPE_R_SLT_FUNC  10'b010_000_0000
`define TYPE_R_SLTU_FUNC 10'b011_000_0000

`define TYPE_I_BASE_OPCODE 7'b0010011
`define TYPE_I_ADDI_FUNC3 3'b000
`define TYPE_I_XORI_FUNC3 3'b100
`define TYPE_I_ORI_FUNC3  3'b110
`define TYPE_I_ANDI_FUNC3 3'b111
`define TYPE_I_SLLI_FUNC3_IMM 10'b001_0000000
`define TYPE_I_SRLI_FUNC3_IMM 10'b101_0000000
`define TYPE_I_SRAI_FUNC3_IMM 10'b101_0100000
`define TYPE_I_SLTI_FUNC3  3'b010
`define TYPE_I_SLTIU_FUNC3 3'b011

// `define TYPE_I_ADDI_FUNC3 3'b000

`define TYPE_I_JALR_OPCODE 7'b1100111
`define TYPE_I_JALR_FUNC3 3'b0

`define TYPE_I_EBREAK 32'b00000000000100000000000001110011

`define TYPE_U_LUI_OPCODE   7'b0110111 
`define TYPE_U_AUIPC_OPCODE 7'b0010111

`define TYPE_B_OPCODE       7'b1100011
`define TYPE_B_BEQ_FUNC3    3'b000
`define TYPE_B_BNE_FUNC3    3'b001
`define TYPE_B_BLT_FUNC3    3'b100
`define TYPE_B_BGE_FUNC3    3'b101
`define TYPE_B_BLTU_FUNC3   3'b110
`define TYPE_B_BGEU_FUNC3   3'b111

`define TYPE_J_JAL_OPCODE 7'b1101111

`define TYPE_S_OPCODE      7'b0100011
`define TYPE_S_SB_FUNC3    3'b000
`define TYPE_S_SH_FUNC3    3'b001
`define TYPE_S_SW_FUNC3    3'b010

`define STORE_SB_8      2'b11
`define STORE_SH_16     2'b01
`define STORE_SW_32     2'b10
`define STORE_INVALID   2'b00

`define STORE_SB_MASK     32'h000000ff
`define STORE_SH_MASK     32'h0000ffff
`define STORE_SW_MASK     32'hffffffff

`define TYPE_I_LOAD_OPCODE 7'b0000011
`define TYPE_I_LB_FUNC3    3'b000
`define TYPE_I_LH_FUNC3    3'b001
`define TYPE_I_LW_FUNC3    3'b010
`define TYPE_I_LBU_FUNC3   3'b100
`define TYPE_I_LHU_FUNC3   3'b101

`define LOAD_LB_8      3'b011
`define LOAD_LH_16     3'b001
`define LOAD_LW_32     3'b010
`define LOAD_LBU_8     3'b111
`define LOAD_LHU_16    3'b101
`define LOAD_INVALID   3'b000

`define LOAD_MASK_8     8'b1
`define LOAD_MASK_16    8'b11
`define LOAD_MASK_32    8'b1111


`define BRANCH_INVALID      3'b000 
`define BRANCH_BEQ          3'b001 
`define BRANCH_BNE          3'b010  
`define BRANCH_BLT          3'b011 
`define BRANCH_BGE          3'b100 
`define BRANCH_BLTU         3'b101 
`define BRANCH_BGEU         3'b110 

`define EN_REG_WRITE        1'b1
`define EN_JMP       1'b1

// `define TYPE_I_OPCODE 7'b0000011
// `define TYPE_I_OPCODE 7'b0000011


// ALU
//alu_sel source digit
`define ALU_SEL1_ZERO  2'b00
`define ALU_SEL1_REG1  2'b01
`define ALU_SEL1_PC    2'b10

`define ALU_SEL2_ZERO  2'b00
`define ALU_SEL2_REG2  2'b01
`define ALU_SEL2_IMM   2'b10
`define ALU_SEL2_4     2'b11

// alu_op operator
`define ALU_OP_ADD          4'b0000
`define ALU_OP_SUB          4'b0001
`define ALU_OP_XOR          4'b0010
`define ALU_OP_OR           4'b0011
`define ALU_OP_AND          4'b0100
`define ALU_OP_RIGHT_LOGIC  4'b0101
`define ALU_OP_RIGHT_ARITH  4'b0110
`define ALU_OP_LEFT_LOGIC   4'b0111
`define ALU_OP_LESS_SIGNED    4'b1000
`define ALU_OP_LESS_UNSIGNED   4'b1001
// `define TYPE_ALU_OP_PC   4'b0010
