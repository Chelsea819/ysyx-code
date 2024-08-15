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
	output								alu_less_o	, // 其实是用来判断有没有出现借位的
	output								alu_zero_o	,
	output		[DATA_LEN - 1:0]		result
);
	wire		[DATA_LEN - 1:0]		result_tmp;
	wire		[DATA_LEN - 1:0]		s_compare_result;
	wire		[DATA_LEN - 1:0]		u_compare_result;
	wire		[DATA_LEN - 1:0]		sub_result;
	wire 								sub_cout;

	// always @(*) begin
	// 	$display("src1 = [%b], src2 = [%b] alu_zero_o = [%b]  alu_less = [%b]",src1, src2, alu_zero_o, alu_less_o);
	// 	$display("src1 = [%h], src2 = [%h] sub_result = [%b] s_compare_result = [%b] u_compare_result = [%b]",src1, src2, sub_result, s_compare_result, u_compare_result);
	// end
	
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
	});

	/* 比较两数大小，查看a-b相关标志位
	有符号整数：当OF = SF时，a > b; OF != SF时，a >= b
	无符号整数：当CF = 1时，即产生借位，此时a < b; CF = 0时，a > b
	SF：符号位，即结果最高位
	OF：(有符号数溢出判断)溢出判断位，OF=C(n) ^ C(n-1)最高位进位异或次高位进位
	CF：进位/借位判断位，CF = Cout ^ Cin, Cin = Sub
	Sub = 1 表示减法运算，此时CF = ~Cout
	Sub = 0 表示加法运算，此时CF = Cout
	 */

	wire [31:0] get_second_cout;
	assign get_second_cout = {1'b0, src1[30:0]} + (~{1'b1, src2[30:0]} + 1); // get the count to bit-31th
	assign alu_zero_o = (sub_result == 32'b0);
	assign result = result_tmp;
	assign s_compare_result =  {{31{1'b0}}, ((get_second_cout[31] ^ sub_cout) != sub_result[31]) & ~alu_zero_o}; 
	assign u_compare_result = {{31{1'b0}}, ~sub_cout};
	assign {sub_cout, sub_result} = {1'b0, src1} + (~{1'b1, src2} + 1);
	assign alu_less_o = (alu_control == `ALU_OP_LESS_SIGNED) ? s_compare_result[0] : 
						(alu_control == `ALU_OP_LESS_UNSIGNED) ? u_compare_result[0] : 
						result_tmp[31];

endmodule
