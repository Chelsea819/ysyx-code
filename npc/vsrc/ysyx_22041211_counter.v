/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/

module ysyx_22041211_counter #(parameter ADDR_LEN = 32)(
	input									clk				,
	input									rst				,
	input									branch_request_i,	
	input		[ADDR_LEN - 1:0]			branch_target_i	,
	input									branch_flag_i	,
	input	    [ADDR_LEN - 1:0]			pc_plus_4		,
	// input	[ADDR_LEN - 1:0]			ce		,
	output reg	[ADDR_LEN - 1:0]			pc
);	
	wire 		[ADDR_LEN - 1:0]			pc_next;

	

	// ysyx_22041211_Reg #(ADDR_LEN, RESET_VAL) PC_Reg (clk,rst,pc_next,1'b1,pc);
	assign pc_next = (branch_flag_i & branch_request_i) ? branch_target_i : pc_plus_4;
	
	always @ (posedge clk) begin
		pc <= pc_next;
	end
	import "DPI-C" function void pc_get(int pc, int dnpc);
		always @(*)
			pc_get(pc, pc_next);
endmodule
