/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
`include "ysyx_22041211_define.v"
module ysyx_22041211_counter #(parameter ADDR_LEN = 32)(
	input									clock				,
	input									reset				,
	input									branch_request_i,	
	input		[ADDR_LEN - 1:0]			branch_target_i	,
	input									branch_flag_i	,
	input	    [ADDR_LEN - 1:0]			pc_plus_4		,
	input                                   jmp_flag_i      ,
    input       [31:0]                   	jmp_target_i,
	input									csr_jmp_i	,
	input		[ADDR_LEN - 1:0]			csr_pc_i	,
	input		[1:0]						con_state	,
	input									last_finish	,
	// input	[ADDR_LEN - 1:0]			ce		,
	output reg	[ADDR_LEN - 1:0]			pc
);	
	wire 		[ADDR_LEN - 1:0]			pc_next;

	parameter IFU_WAIT_FINISH = 2'b10;

	// always @(*) begin
	// 	$display("csr_jmp_i = [%b] ",csr_jmp_i);
	// 	$display("csr_pc_i = [%b]\n",csr_pc_i);
	// end

	// ysyx_22041211_Reg #(ADDR_LEN, RESET_VAL) PC_Reg (clock,reset,pc_next,1'b1,pc);
	assign pc_next = (branch_flag_i & branch_request_i) ? branch_target_i : 
					 jmp_flag_i 						? jmp_target_i : 
					 csr_jmp_i 							? csr_pc_i : 
					 pc_plus_4;
	
	always @ (posedge clock) begin
		if(reset)
			pc <= `PC_RESET_VAL;
		else if (con_state == IFU_WAIT_FINISH && last_finish == 1'b1) 
			pc <= pc_next;
	end
	
	import "DPI-C" function void pc_get(int pc, int dnpc);
		always @(*) begin
			$display("pc = %x dpc = %x\n",pc,pc_next);
			pc_get(pc, pc_next);
		end
endmodule
