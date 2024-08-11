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
	input		                		wd_i		,
    input		[4:0]		            wreg_i		,
	input		[1:0]					store_type_i,
	input       [2:0]                   load_type_i ,
	input		[2:0]					branch_type_i,
	output      [2:0]                   load_type_o ,
	output								branch_request_o,	
    output		                		mem_wen_o		,
	output		[DATA_LEN - 1:0]		mem_wdata_o	,
    output		                		wd_o		,
    output		[4:0]		            wreg_o		,
    output		[DATA_LEN - 1:0]		alu_result_o
	// output								zero
	// output		[DATA_LEN - 1:0]		result,
	// output								zero
	// output								OF,		//溢出标志
	// output								CF		//进/借位标志
);
	wire [31:0] src1;
	wire [31:0] src2;
	wire 		alu_zero;
	wire 		alu_sign;
	wire [31:0] mem_data_mask;
	assign wd_o = wd_i;
	assign wreg_o = wreg_i;
	assign mem_wen_o = |store_type_i;
	assign mem_data_mask = (store_type_i == `STORE_SB_8) ? `STORE_SB_MASK :
						   (store_type_i == `STORE_SH_16) ? `STORE_SH_MASK :
						   (store_type_i == `STORE_SW_32) ? `STORE_SW_MASK :
						   32'b0;
	assign mem_wdata_o = reg2_i & mem_data_mask;
	assign load_type_o = load_type_i;

	ysyx_22041211_MuxKeyWithDefault #(4,3,1) branch_request (branch_request_o, branch_type_i, 1'b0, {
		`BRANCH_BEQ, alu_zero,
		`BRANCH_BNE, ~alu_zero,
		`BRANCH_BLT, alu_sign,
		`BRANCH_BGE, ~alu_sign
		// `BRANCH_BLTU, ~alu_zero,
		// `BRANCH_BGEU, alu_sign
	});

	ysyx_22041211_ALU my_alu(
		.src1				(src1),
		.src2				(src2),
		.alu_control		(alu_control),
		.result				(alu_result_o),
		.alu_sign_o			(alu_zero),
		.alu_zero_o 		(alu_sign)
	);

	ysyx_22041211_MuxKeyWithDefault #(3,2,32) src1_choose (src1, alu_sel[1:0], 32'b0, {
		`ALU_SEL1_ZERO, 32'b0,
		`ALU_SEL1_REG1, reg1_i,
		`ALU_SEL1_PC,   pc_i
	});

	ysyx_22041211_MuxKeyWithDefault #(4,2,32) src2_choose (src2, alu_sel[3:2], 32'b0, {
		`ALU_SEL1_ZERO, 32'b0,
		`ALU_SEL2_REG2, reg2_i,
		`ALU_SEL2_IMM, 	imm_i,
		`ALU_SEL2_4, 	32'b100
	});

	// wire signed [31:0] signed_a  ;
	// wire signed [31:0] signed_b  ;
	// wire		[DATA_LEN - 1:0]		result_tmp;
	// wire				cout;
	// wire				SF;

	// assign signed_a = src1;
	// assign signed_b = src2;
	// assign SF = (alu_control == 4'b0011 || alu_control == 4'b0111 || alu_control == 4'b1101) ? ((signed_a[31] ^ signed_b[31] == 1'b1 && signed_a[31] == 1'b1) ? 1'b1 : 
	// 																							(signed_a[31] ^ signed_b[31] == 1'b1 && signed_b[31] == 1'b1) ? 1'b0 : result_tmp[31]) : ~cout ;
	// assign result = (alu_control == 4'b0011) ? {{31{1'b0}}, SF} : 
	// 				(alu_control == 4'b0100) ? {{31{1'b0}}, ~cout} : result_tmp;
	// wire [31:0] tmp;
	// wire [31:0] src2_tmp;
	// wire 		c_tmp;
	// assign {c_tmp,src2_tmp} = {1'b0,~src2} + 1;

	// assign sub = (alu_control == 4'b0001 || alu_control == 4'b0011 || alu_control == 4'b0100);

	// ysyx_22041211_MuxKeyWithDefault #(14,4,32) ALUmode (result_tmp, alu_control, 32'b0, {
	// 	4'b0000, src1 + src2,
	// 	4'b0001, src1 + (~src2 + 1),
	// 	4'b0010, src1 << src2,
	// 	4'b0011, signed_a + (~signed_b + 1),        //signed_a < signed_b ? 32'b1 : 32'b0,
	// 	4'b0100, src1 + (~src2 + 1),				//src1 < src2 ? 32'b1 : 32'b0,
	// 	4'b0101, src1 ^ src2,
	// 	4'b0110, src1 >> src2,
	// 	4'b0111, signed_a >>> src2,
	// 	4'b1000, src1 | src2,
	// 	4'b1001, src1 & src2,
	// 	4'b1010, src2,
	// 	4'b1011, src1 >> src2[4:0],
	// 	4'b1100, src1 << src2[4:0],
	// 	4'b1101, signed_a >>> src2[4:0]
	// });

	// assign zero = result_tmp == 32'b0 ;
	// assign {cout,tmp} = ((alu_control == 4'b0000) ? (({1'b0,src1} + {1'b0,src2})) :
	// 					(alu_control == 4'b0001) ? (({1'b0,src1} + {1'b0,~src2} + 1)) :
	// 			 		(alu_control == 4'b0100) ? (({1'b0,src1} + {1'b0,~src2} + 1)) : 
	// 					(alu_control == 4'b0011) ? (({1'b0,signed_a} + {1'b0,~signed_b} + 1)) : 33'b0);

	// assign {SF,cout,tmp} = ((alu_control == 4'b0100) ? (({2'b0,src1} + {2'b0,~src2} + 34'b1)) : 
	// 					   (alu_control == 4'b0011) ? (({src1[31],1'b0,src1[30:0]} + {src1[31],1'b1,~src1[30:0]} + 1)) : 33'b0);



endmodule
