/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/
`include "./ysyx_22041211_define.v"
module ysyx_22041211_CSR #(parameter DATA_WIDTH = 32)(
	input								clk		,
	input								rst		,
	input	    [11:0]					csr_addr,	// 要读的csr
	input		[DATA_WIDTH - 1:0]		wdata	,	// 要写入csr的数据	
	input		[2:0]					csr_type_i,
	input		[DATA_WIDTH - 1:0]		csr_mepc_i	,
	input		[DATA_WIDTH - 1:0]		csr_mcause_i	,
	output		[DATA_WIDTH - 1:0]		csr_pc_o	,
	output		[DATA_WIDTH - 1:0]		r_data	
);
	reg 	[DATA_WIDTH - 1:0] 		csr [3:0]	;
	wire 	[1:0]  					csr_idx		;

	assign csr_idx = (csr_addr == `CSR_MCAUSE_ADDR)	? `CSR_MCAUSE_IDX :
					 (csr_addr == `CSR_MSTATUS_ADDR)? `CSR_MSTATUS_IDX :
					 (csr_addr == `CSR_MEPC_ADDR)	? `CSR_MEPC_IDX :
					 (csr_addr == `CSR_MTVEC_ADDR)	? `CSR_MTVEC_IDX :
					 `CSR_MTVEC_IDX ;

	assign csr_pc_o = (csr_type_i == `CSR_ECALL)	? csr[`CSR_MTVEC_IDX] :
					  (csr_type_i == `CSR_MRET)		? csr[`CSR_MEPC_IDX] :
					  32'b0 ;

	assign r_data = csr[csr_idx];

	always @(*) begin
		$display("csr_type_i = [%b] csr_idx = [%b]  wdata = [%b]",csr_type_i,csr_idx,wdata);
	end

	always @(posedge clk) begin
		if(rst)
			csr[`CSR_MSTATUS_IDX] <= 32'h1800;
		else if (^csr_type_i == 1'b1) 
			csr[csr_idx] <= wdata;
		else if(csr_type_i == `CSR_ECALL) begin
			csr[`CSR_MCAUSE_IDX] <= csr_mcause_i;
			csr[`CSR_MEPC_IDX] <= csr_mepc_i;
		end
	end

	// //读取操作数
	// assign r_data1 = rf[rsc1[3:0]];
	// assign r_data2 = rf[rsc2[3:0]];

endmodule
