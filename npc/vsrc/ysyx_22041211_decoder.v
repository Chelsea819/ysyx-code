`include "./ysyx_22041211_define.v"
module ysyx_22041211_decoder(
    input									      clk				        ,
    input									      rst				        ,
    input           [31:0]                        inst_i                    ,
    input           [31:0]                        reg1_data_i               ,
    input           [31:0]                        reg2_data_i               ,
    input           [31:0]                        pc_i                      ,
    input                                         ifu_valid                 ,
    // input                                         exu_ready                 ,
    // output                                        idu_ready_o               ,
    // output  reg                                   idu_valid_o               ,
    output          [3:0]                         aluop_o                   ,
    output          [3:0]                         alusel_o                  ,
    output          [31:0]                        pc_o                      ,
    output          [31:0]                        reg1_o                    ,
    output          [31:0]                        reg2_o                    ,
    output	                   		              wd_o                      ,
    output	        [4:0]		                  wreg_o                    ,
    output          [4:0]                         reg1_addr_o               ,
    output          [4:0]                         reg2_addr_o               ,
    output          [2:0]                         branch_type_o             ,
    output          [31:0]                        branch_target_o           ,
    output          [1:0]                         store_type_o              ,
    output          [2:0]                         load_type_o              ,
    output                                        jmp_flag_o                ,
    output          [31:0]                        jmp_target_o              ,
    output          [11:0]                        csr_addr_o                ,
    output          [2:0]                         csr_flag_o                ,
    output          [31:0]                        imm_o         
);
    wire            [6:0]                          func7;
    wire            [2:0]                          func3;
    wire            [6:0]                          opcode;
    reg                                            idu_valid_o;

    assign func3 = inst_i[14:12];
    assign func7 = inst_i[31:25];
    assign opcode = inst_i[6:0];
    assign pc_o = pc_i;
    assign reg1_o = reg1_data_i;
    assign reg2_o = reg2_data_i;
    assign wreg_o = inst_i[11:7];
    assign reg1_addr_o = inst_i[19:15];
    assign reg2_addr_o = inst_i[24:20];
    assign branch_target_o = pc_i + imm_o;
    assign csr_addr_o = inst_i[31:20];
    // assign idu_ready_o = 1'b1;
    // assign idu_valid_o = 1'b1;
    // always @(*) begin
	// 	$display("csr_flag_o = [%b]",csr_flag_o);
	// 	$display("jmp_flag_o = [%b]",jmp_flag_o);
	// end
    reg			[1:0]			        	con_state	;
	reg			[1:0]			        	next_state	;
    parameter [1:0] IDU_WAIT_IFU_VALID = 2'b00, IDU_WAIT_IDU_VALID = 2'b01, IDU_WAIT_EXU_READY = 2'b10;

    always @(posedge clk ) begin
		if(next_state == IDU_WAIT_IDU_VALID)
			idu_valid_o <= 1'b1;
		else 
			idu_valid_o <= 1'b0;
	end

	// state trans
	always @(posedge clk ) begin
		if(rst)
			con_state <= IDU_WAIT_IFU_VALID;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
            // 等待ifu取指，下一个时钟周期开始译码
			IDU_WAIT_IFU_VALID: begin
				if (ifu_valid == 1'b0) begin
					next_state = IDU_WAIT_IFU_VALID;
				end else begin 
					next_state = IDU_WAIT_IDU_VALID;
				end
			end
            // 等待idu完成译码
			IDU_WAIT_IDU_VALID: begin 
				if (idu_valid_o == 1'b0) begin
					next_state = IDU_WAIT_IDU_VALID;
				end else begin 
					next_state = IDU_WAIT_EXU_READY;
				end
			end
            // 等待exu空闲，下个时钟周期传递信息
            IDU_WAIT_EXU_READY: begin 
				// if (exu_ready == 1'b0) begin
				// 	next_state = IDU_WAIT_EXU_READY;
				// end else begin 
					next_state = IDU_WAIT_IFU_VALID;
				// end
			end
            default: begin 
				next_state = 2'b11;
			end
		endcase
	end
    
// controller look-up lut
// inst: R-type: wd_o aluop_o alusel_o
// alu_sel  [1:0] res1/pc_o/0, [3:2] res2/imm_o/4/0
// alu_op + - 
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
                                                              ({inst_i}                     == {`TYPE_I_ECALL})                           ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_ZERO,`ALU_SEL1_CSR, `STORE_INVALID, `LOAD_INVALID}}  :       // I-ecall
                                                              ({inst_i}                     == {`TYPE_I_MRET})                            ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_ZERO,`ALU_SEL1_CSR, `STORE_INVALID, `LOAD_INVALID}}  :       // I-mret
                                                              ({opcode, func3}              == {`TYPE_I_CSR_OPCODE, `TYPE_I_CSRRW_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_ZERO,`ALU_SEL1_CSR, `STORE_INVALID, `LOAD_INVALID}}  :        // I-csrrw
                                                              ({opcode, func3}              == {`TYPE_I_CSR_OPCODE, `TYPE_I_CSRRS_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_ZERO,`ALU_SEL1_CSR, `STORE_INVALID, `LOAD_INVALID}}  :        // I-csrrs
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_ADDI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-addi
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_XORI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_XOR, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-xori
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_ORI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_OR, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-ori
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_ANDI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_AND, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-andi
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_SLTI_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_LESS_SIGNED, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-ori
                                                              ({opcode, func3}              == {`TYPE_I_BASE_OPCODE, `TYPE_I_SLTIU_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_LESS_UNSIGNED, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :        // I-andi
                                                              ({opcode, func3}              == {`TYPE_I_JALR_OPCODE, `TYPE_I_JALR_FUNC3}) ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_4,`ALU_SEL1_PC, `STORE_INVALID, `LOAD_INVALID}}  :            // I-jalr
                                                              ({opcode, func3, imm_o[11:5]}   == {`TYPE_I_BASE_OPCODE, `TYPE_I_SLLI_FUNC3_IMM}) ? {`EN_REG_WRITE, `ALU_OP_LEFT_LOGIC, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-slli
                                                              ({opcode, func3, imm_o[11:5]}   == {`TYPE_I_BASE_OPCODE, `TYPE_I_SRLI_FUNC3_IMM}) ? {`EN_REG_WRITE, `ALU_OP_RIGHT_LOGIC, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-slli
                                                              ({opcode, func3, imm_o[11:5]}   == {`TYPE_I_BASE_OPCODE, `TYPE_I_SRAI_FUNC3_IMM}) ? {`EN_REG_WRITE, `ALU_OP_RIGHT_ARITH, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}  :         // I-slli
                                                              ({opcode, func3}              == {`TYPE_S_OPCODE, `TYPE_S_SB_FUNC3})        ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_SB_8, `LOAD_INVALID}}  :         // S-sb
                                                              ({opcode, func3}              == {`TYPE_S_OPCODE, `TYPE_S_SH_FUNC3})        ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_SH_16, `LOAD_INVALID}}  :        // S-sh
                                                              ({opcode, func3}              == {`TYPE_S_OPCODE, `TYPE_S_SW_FUNC3})        ? {~`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_SW_32, `LOAD_INVALID}}  :        // S-sw
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LB_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LB_8}}  :         // L-sb
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LH_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LH_16}}  :        // L-sh
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LW_FUNC3})        ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LW_32}}  :        // L-sw
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LBU_FUNC3})       ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LBU_8}}  :         // L-sb
                                                              ({opcode, func3}              == {`TYPE_I_LOAD_OPCODE, `TYPE_I_LHU_FUNC3})       ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_LHU_16}}  :        // L-sh
                                                              ({opcode}                     == {`TYPE_J_JAL_OPCODE})                      ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_4,`ALU_SEL1_PC, `STORE_INVALID, `LOAD_INVALID}}  :            // J-jal
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BEQ_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BNE_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_SUB, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BLT_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_LESS_SIGNED, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BGE_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_LESS_SIGNED, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BLTU_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_LESS_UNSIGNED, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode, func3}              == {`TYPE_B_OPCODE, `TYPE_B_BGEU_FUNC3})       ? {~`EN_REG_WRITE, `ALU_OP_LESS_UNSIGNED, {`ALU_SEL2_REG2,`ALU_SEL1_REG1, `STORE_INVALID, `LOAD_INVALID}}:        // B-beq
                                                              ({opcode}                     == {`TYPE_U_AUIPC_OPCODE})                    ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_PC, `STORE_INVALID, `LOAD_INVALID}}    :        // U-auipc
                                                              ({opcode}                     == {`TYPE_U_LUI_OPCODE})                      ? {`EN_REG_WRITE, `ALU_OP_ADD, {`ALU_SEL2_IMM,`ALU_SEL1_ZERO, `STORE_INVALID, `LOAD_INVALID}}  :         // U-lui  
                                                              0;         

assign  {branch_type_o, jmp_target_o, jmp_flag_o, csr_flag_o} = ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BEQ_FUNC3})             ? {`BRANCH_BEQ,     32'b0,             ~`EN_JMP, `CSR_INVALID}:         // B-beq
                                                                ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BNE_FUNC3})             ? {`BRANCH_BNE,     32'b0,             ~`EN_JMP, `CSR_INVALID}:         // B-beq
                                                                ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BLT_FUNC3})             ? {`BRANCH_BLT,     32'b0,             ~`EN_JMP, `CSR_INVALID}:         // B-beq
                                                                ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BGE_FUNC3})             ? {`BRANCH_BGE,     32'b0,             ~`EN_JMP, `CSR_INVALID}:         // B-beq
                                                                ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BLTU_FUNC3})            ? {`BRANCH_BLTU,    32'b0,             ~`EN_JMP, `CSR_INVALID}:         // B-beq
                                                                ({opcode, func3} == {`TYPE_B_OPCODE, `TYPE_B_BGEU_FUNC3})            ? {`BRANCH_BGEU,    32'b0,             ~`EN_JMP, `CSR_INVALID}:         // B-beq
                                                                ({opcode, func3} == {`TYPE_I_JALR_OPCODE, `TYPE_I_JALR_FUNC3})       ? {`BRANCH_INVALID, reg1_data_i + imm_o, `EN_JMP , `CSR_INVALID} :         // I-jalr
                                                                ({opcode}        == {`TYPE_J_JAL_OPCODE})                            ? {`BRANCH_INVALID, pc_i + imm_o,        `EN_JMP, `CSR_INVALID} :         // J-jal 
                                                                ({inst_i}        == {`TYPE_I_ECALL})                                 ? {`BRANCH_INVALID, 32'b0,             ~`EN_JMP, `CSR_ECALL}  :         // I-ecall 
                                                                ({inst_i}        == {`TYPE_I_MRET})                                  ? {`BRANCH_INVALID, 32'b0,             ~`EN_JMP, `CSR_MRET}   :         // I-mret 
                                                                ({opcode, func3} == {`TYPE_I_CSR_OPCODE, `TYPE_I_CSRRW_FUNC3})       ? {`BRANCH_INVALID, 32'b0,             ~`EN_JMP, `CSR_CSRRW}  :         // I-csrrw
                                                                ({opcode, func3} == {`TYPE_I_CSR_OPCODE, `TYPE_I_CSRRS_FUNC3})       ? {`BRANCH_INVALID, 32'b0,             ~`EN_JMP, `CSR_CSRRS}  :         // I-csrrs
                                                                0;       

ysyx_22041211_immGen my_gen (
    .inst       (inst_i),
    .imm        (imm_o)
);

endmodule

