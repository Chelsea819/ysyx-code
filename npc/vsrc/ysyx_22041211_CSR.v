/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/
`include "./ysyx_22041211_define.v"
module ysyx_22041211_CSR #(parameter ADDR_WIDTH = 32, DATA_WIDTH = 32)(
	input								clk		,
	input								rst		,
	input	    [11:0]					csr_addr,	// 要读的csr
	input		[DATA_WIDTH - 1:0]		wdata	,	// 要写入csr的数据	
	input								csrWrite,
	output		[DATA_WIDTH - 1:0]		r_data	
);
	reg 	[DATA_WIDTH - 1:0] 		csr [3:0]	;
	wire 	[1:0]  					csr_idx		;

	assign csr_idx = (csr_addr == `CSR_MCAUSE_ADDR)	? `CSR_MCAUSE_IDX :
					 (csr_addr == `CSR_MSTATUS_ADDR)? `CSR_MSTATUS_IDX :
					 (csr_addr == `CSR_MEPC_ADDR)	? `CSR_MEPC_IDX :
					 (csr_addr == `CSR_MTVEC_ADDR)	? `CSR_MTVEC_IDX :
					 `CSR_MTVEC_IDX ;

	assign r_data = csr[csr_idx];

	always @(posedge clk) begin
		if(rst)
			csr[`CSR_MSTATUS_IDX] <= 32'h1800;
		else if (csrWrite) 
			csr[csr_idx] <= wdata;
	end

	// //读取操作数
	// assign r_data1 = rf[rsc1[3:0]];
	// assign r_data2 = rf[rsc2[3:0]];

endmodule
