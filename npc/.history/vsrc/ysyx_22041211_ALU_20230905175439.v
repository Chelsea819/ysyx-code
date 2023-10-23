/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/

module ysyx_22041211_ALU #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		src1,
	input		[DATA_LEN - 1:0]		src2,
	input								en,  
//	input 		[2:0]					alu_control,	
	input								rst,
//	input		[DATA_LEN - 1:0]		pc,

	output		[DATA_LEN - 1:0]		result
);

	// add 001
	assign	result = {DATA_LEN{~rst}} & ( src1 + src2 ) & {DATA_LEN{en}};
	
	// sub 010


endmodule

