/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/
`include "ysyx_22041211_define.v"
module ysyx_22041211_pcPlus #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		pc_old,
	input								reset,
	output	reg	[DATA_LEN - 1:0]		pc_new
);
	always @(*) begin
		if(reset)
			pc_new = `PC_RESET_VAL;
		else 
			pc_new = pc_old + 32'b100;
	
	end


endmodule

