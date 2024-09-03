/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/
`include "./ysyx_22041211_define.v"
module ysyx_22041211_EXE #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		reg1_i		,
	input		[DATA_LEN - 1:0]		reg2_i		,
	input		[DATA_LEN - 1:0]		pc_i		,
    // input		[DATA_LEN - 1:0]		inst		,
	input 		[3:0]					alu_control	,
    input 		[3:0]					alu_sel		, // choose source number
	input       [DATA_LEN - 1:0]        imm_i		,
	input       [DATA_LEN - 1:0]        csr_rdata_i	,
	input 		[2:0]					csr_flag_i	,
	input		                		wd_i		,
    input		[4:0]		            wreg_i		,
	input		[1:0]					store_type_i,
	input       [2:0]                   load_type_i ,
	input		[2:0]					branch_type_i,
	output      [2:0]                   load_type_o ,
	output      [1:0]					store_type_o,
	output								branch_request_o,	
    output		                		mem_wen_o		,
	output		[DATA_LEN - 1:0]		mem_wdata_o	,
    output		                		wd_o		,
    output		[4:0]		            wreg_o		,
	output      [DATA_LEN - 1:0]        csr_wdata_o	,
	output      [DATA_LEN - 1:0]        csr_mcause_o,
	output		[DATA_LEN - 1:0]		pc_o		,
    output		[DATA_LEN - 1:0]		alu_result_o
);
	wire [31:0] src1;
	wire [31:0] src2;
	wire 		alu_zero;
	wire 		alu_less;
	assign wd_o = wd_i;
	assign wreg_o = wreg_i;
	assign mem_wen_o = |store_type_i;
	assign mem_wdata_o = reg2_i;
	assign load_type_o = load_type_i;
	assign store_type_o = store_type_i;
	assign pc_o = pc_i;
	assign csr_mcause_o = 32'hb;


	// always @(*) begin
	// 	$display("mem_wdata_o = [%h] mem_wen_o = [%b] reg2_i = [%h]",mem_wdata_o,mem_wen_o, reg2_i);
	// end

	ysyx_22041211_MuxKeyWithDefault #(6,3,1) branch_request (branch_request_o, branch_type_i, 1'b0, {
		`BRANCH_BEQ, alu_zero,
		`BRANCH_BNE, ~alu_zero,
		`BRANCH_BLT, alu_less,
		`BRANCH_BGE, ~alu_less,
		`BRANCH_BLTU, alu_less,
		`BRANCH_BGEU, ~alu_less
	});

	ysyx_22041211_MuxKeyWithDefault #(2,3,32) csr_wdata_choose (csr_wdata_o, csr_flag_i, 32'b0, {
		`CSR_CSRRW, reg1_i,
		`CSR_CSRRS, reg1_i | csr_rdata_i
	});

	ysyx_22041211_ALU my_alu(
		.src1				(src1),
		.src2				(src2),
		.alu_control		(alu_control),
		.result				(alu_result_o),
		.alu_less_o			(alu_less),
		.alu_zero_o 		(alu_zero)
	);

	ysyx_22041211_MuxKeyWithDefault #(4,2,32) src1_choose (src1, alu_sel[1:0], 32'b0, {
		`ALU_SEL1_ZERO, 32'b0,
		`ALU_SEL1_REG1, reg1_i,
		`ALU_SEL1_PC,   pc_i,
		`ALU_SEL1_CSR,  csr_rdata_i
	});

	ysyx_22041211_MuxKeyWithDefault #(4,2,32) src2_choose (src2, alu_sel[3:2], 32'b0, {
		`ALU_SEL1_ZERO, 32'b0,
		`ALU_SEL2_REG2, reg2_i,
		`ALU_SEL2_IMM, 	imm_i,
		`ALU_SEL2_4, 	32'b100
	});


endmodule
