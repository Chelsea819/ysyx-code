/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/

module ysyx_22041211_ALU #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		src1,
	input		[DATA_LEN - 1:0]		src2,
	input 		[3:0]					alu_control,
	output		[DATA_LEN - 1:0]		result
);

	wire signed [31:0] signed_a  ;
	wire signed [31:0] signed_b  ;

	assign signed_a = src1;
	assign signed_b = src2;

	ysyx_22041211_MuxkeyWithDefault #(8,3,32) ALUmode (result, alu_control, 32'b0, {
		4'b0000, src1 + src2,
		4'b0001, src1 + (~src2 + 1),
		4'b0010, src1 << src2[4:0],
		4'b0011, signed_a < signed_b ? 32'b1 : 32'b0,
		4'b0100, src1 < src2 ? 32'b1 : 32'b0,
		4'b0101, src1 ^ src2,
		4'b0110, src1 >> src2[4:0],
		4'b0111, src1 >>> src2[4:0],
		4'b1000, src1 | src2,
		4'b1001, src1 & src2,
		4'b1010, src2
	});


endmodule
