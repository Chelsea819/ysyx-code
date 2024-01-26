/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/
/* verilator lint_off WIDTHEXPAND */
module ysyx_22041211_ALU #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		src1,
	input		[DATA_LEN - 1:0]		src2,
	input 		[3:0]					alu_control,
	output		[DATA_LEN - 1:0]		result,
	output								zero,
	output								SF		//符号标志
	// output								OF,		//溢出标志
	// output								CF		//进/借位标志
);

	wire signed [31:0] signed_a  ;
	wire signed [31:0] signed_b  ;
	wire		[DATA_LEN - 1:0]		result_tmp;
	wire				cout;
	// wire				sub;

	assign signed_a = src1;
	assign signed_b = src2;
	assign SF = result_tmp[DATA_LEN - 1];
	assign result = (alu_control == 4'b0011) ? {{31{1'b0}},{SF}} : 
					(alu_control == 4'b0100) ? {{31{1'b0}},{(~cout & zero)}} : result_tmp;

	// assign sub = (alu_control == 4'b0001 || alu_control == 4'b0011 || alu_control == 4'b0100);

	ysyx_22041211_MuxKeyWithDefault #(14,4,32) ALUmode (result_tmp, alu_control, 32'b0, {
		4'b0000, src1 + src2,
		4'b0001, src1 + (~src2 + 1),
		4'b0010, src1 << src2,
		4'b0011, signed_a + (~signed_b + 1),        //signed_a < signed_b ? 32'b1 : 32'b0,
		4'b0100, src1 + (~src2 + 1),				//src1 < src2 ? 32'b1 : 32'b0,
		4'b0101, src1 ^ src2,
		4'b0110, src1 >> src2,
		4'b0111, signed_a >>> src2,
		4'b1000, src1 | src2,
		4'b1001, src1 & src2,
		4'b1010, src2,
		4'b1011, src1 >> src2[4:0],
		4'b1100, src1 << src2[4:0],
		4'b1101, signed_a >>> src2[4:0]
	});

	assign zero = (result_tmp == 32'b0) ;
	assign cout = (alu_control == 4'b0000) ? ((src1 + src2)[32]) :
				  (alu_control == 4'b0100) ? ((src1 + (~src2 + 1))[32]) : 1'b0;
	
	// assign OF = ~src1[DATA_LEN - 1] & ~src2[DATA_LEN - 1] & ~src1[DATA_LEN - 1]
	// assign CF = cout ^ sub;


endmodule
/* verilator lint_on WIDTHEXPAND */
