/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/
 `include "./ysyx_22041211_define.v"
module ysyx_22041211_ALU #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		src1		,
	input		[DATA_LEN - 1:0]		src2		,
	input 		[3:0]					alu_control	,
	output								alu_less_o, // 其实是用来判断有没有出现借位的
	output								alu_zero_o	,
	output		[DATA_LEN - 1:0]		result
	// output								zero
	// output								OF,		//溢出标志
	// output								CF		//进/借位标志
);
	wire		[DATA_LEN - 1:0]		result_tmp;
	wire		[DATA_LEN - 1:0]		s_compare_result;
	wire		[DATA_LEN - 1:0]		u_compare_result;
	wire		[DATA_LEN - 1:0]		sub_result;
	// wire 								alu_sign;
	wire 								sub_cout;
	
	ysyx_22041211_MuxKeyWithDefault #(10,4,32) ALUmode (result_tmp, alu_control, 32'b0, {
		`ALU_OP_ADD, 			src1 + src2,
		`ALU_OP_SUB, 			sub_result, 
		`ALU_OP_XOR, 			src1 ^ src2,
		`ALU_OP_OR,  			src1 | src2,
		`ALU_OP_AND, 			src1 & src2,
		`ALU_OP_RIGHT_LOGIC, 	src1 >> src2[4:0],
		`ALU_OP_RIGHT_ARITH,  	$signed(src1) >>> src2[4:0],
		`ALU_OP_LEFT_LOGIC, 	src1 << src2[4:0],
		`ALU_OP_LESS_SIGNED,  	s_compare_result,
		`ALU_OP_LESS_UNSIGNED, 	u_compare_result
		// 4'b0010, src1 << src2,
		// 4'b0011, signed_a + (~signed_b + 1),        //signed_a < signed_b ? 32'b1 : 32'b0,
		// 4'b0100, src1 + (~src2 + 1),				//src1 < src2 ? 32'b1 : 32'b0,
		// 4'b0101, src1 ^ src2,
		// 4'b0110, src1 >> src2,
		// 4'b0111, signed_a >>> src2,
		// 4'b1000, src1 | src2,
		// 4'b1001, src1 & src2,
		// 4'b1010, src2,
		// 4'b1011, src1 >> src2[4:0],
		// 4'b1100, src1 << src2[4:0],
		// 4'b1101, signed_a >>> src2[4:0]
	});

	// assign alu_sign = sub_result[31];
	assign alu_zero_o = (result_tmp == 32'b0);
	assign result = result_tmp;
	assign s_compare_result = {{31{1'b0}}, sub_result[31]};
	assign u_compare_result = {{31{1'b0}}, ~sub_cout};
	assign {sub_cout, sub_result} = {1'b0, src1} + (~{1'b1, src2} + 1);
	assign alu_less_o = (alu_control == `ALU_OP_LESS_SIGNED) ? sub_result[31] : 
						   (alu_control == `ALU_OP_LESS_UNSIGNED) ? ~sub_cout : 
						   result_tmp[31];


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

	

	
	// assign {cout,tmp} = ((alu_control == 4'b0000) ? (({1'b0,src1} + {1'b0,src2})) :
	// 					(alu_control == 4'b0001) ? (({1'b0,src1} + {1'b0,~src2} + 1)) :
	// 			 		(alu_control == 4'b0100) ? (({1'b0,src1} + {1'b0,~src2} + 1)) : 
	// 					(alu_control == 4'b0011) ? (({1'b0,signed_a} + {1'b0,~signed_b} + 1)) : 33'b0);

	// assign {SF,cout,tmp} = ((alu_control == 4'b0100) ? (({2'b0,src1} + {2'b0,~src2} + 34'b1)) : 
	// 					   (alu_control == 4'b0011) ? (({src1[31],1'b0,src1[30:0]} + {src1[31],1'b1,~src1[30:0]} + 1)) : 33'b0);



endmodule
