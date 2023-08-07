/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/

module ysyx_22041211_counter #(parameter DATA_LEN = 32, ADDR_LEN = 32,RESET_VAL = 32'h80000000)(
	input								clk		,
	input								rst		,
	input	[ADDR_LEN - 1:0]			pc_old	,
	output	[ADDR_LEN - 1:0]			pc_new
);
	ysyx_22041211_Reg #(ADDR_LEN, RESET_VAL) my_Reg (clk,rst,pc_old + DATA_LEN / 8,1'b1,pc_new);

endmodule
