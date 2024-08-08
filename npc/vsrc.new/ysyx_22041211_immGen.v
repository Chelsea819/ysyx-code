module ysyx_22041211_immGen #(parameter DATA_LEN = 32)(
    input		[DATA_LEN - 1:0]		inst,
    output		[DATA_LEN - 1:0]		imm
);

wire [6:0]      opcode;

assign opcode = inst[6:0];

ysyx_22041211_MuxKeyWithDefault #(10,7,32) imm_gen (imm, opcode, 32'b0, {
		7'b0110011, 32'b0,       // R-type   add sub xor or and sll srl sra slt sltu
        7'b0010011, {{20{inst[31]}}, inst[31:20]},       // I-type   addi xori ori andi slli srli srai slti sltiu
        7'b0000011, {{20{inst[31]}}, inst[31:20]},       // I-type   lb lh lw lbu lhu
        7'b1110011, {{20{inst[31]}}, inst[31:20]},       // I-type   ecall ebreak
        7'b1100111, {{20{inst[31]}}, inst[31:20]},       // I-type   jalr
        7'b0100011, {{20{inst[31]}}, inst[31:25], inst[11:7]},       // S-type   sb sh sw
        7'b1100011, {{20{inst[31]}}, inst[7], inst[30:25], inst[11:8], 1'b0},       // B-type   beq bne blt bge bltu bgeu
        7'b0110111, {inst[31:12], 12'b0},       // U-type   lui 
        7'b0010111, {inst[31:12], 12'b0},       // U-type   auipc
        7'b1101111, {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0}        // J-type   jal
	});


endmodule
