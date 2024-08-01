/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/

module ysyx_22041211_counter #(parameter ADDR_LEN = 32,RESET_VAL = 32'h80000000)(
	input								clk		,
	input								rst		,
	input	[ADDR_LEN - 1:0]			pc'	,
	output	[ADDR_LEN - 1:0]			pc
);
	ysyx_22041211_Reg #(ADDR_LEN, RESET_VAL) PC_Reg (clk,rst,pc_new,1'b1,pc);
	// always @(*)
	// 	$display();

endmodule
