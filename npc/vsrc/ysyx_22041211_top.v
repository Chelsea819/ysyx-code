`include "./ysyx_22041211_define.v"
module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32) (
	input								clk 		,
	input								rst 		,
	output			[ADDR_LEN - 1:0]	pc			,
	output								invalid		,
	output								finish
);
	wire			[DATA_LEN - 1:0]	inst		;
	wire								inst_ren	;
	wire			[ADDR_LEN - 1:0]	inst_addr	;

	wire 			[DATA_LEN - 1:0] 	mem_waddr_i;
	wire 			[DATA_LEN - 1:0] 	mem_wdata_i;
    wire 			[DATA_LEN - 1:0] 	mem_raddr_i;
    reg  			[DATA_LEN - 1:0] 	mem_rdata_rare_o;
    reg  			[7:0]  				mem_rmask_i;
    wire 			[7:0]  				mem_wmask_i;
    wire  			      				mem_wen_i;
    wire  			      				mem_ren_i;

	// IFU-AXI
	// Addr Read
	wire	                			inst_addr_r_valid_o;
	wire	                			inst_addr_r_ready_i;

	// Read data
	// wire		[DATA_LEN - 1:0]		inst_r_data_i	;
	wire		[1:0]					inst_r_resp_i	;	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	wire		                		inst_r_valid_i	;
	wire		                		inst_r_ready_o	;


	

assign inst_addr = pc;

// 检测到ebreak
    import "DPI-C" function void ifebreak_func(int inst);
    always @(*)
        ifebreak_func(inst);

// 为ITRACE提供指令
    import "DPI-C" function void inst_get(int inst);
    always @(*)
        inst_get(inst);


	ysyx_22041211_cpu#(
		.DATA_LEN  ( 32 ),
		.ADDR_LEN  ( 32 )
	)u_ysyx_22041211_cpu(
		.clk       ( clk       ),
		.rst       ( rst       ),
		.inst_addr_r_valid_o	(	inst_addr_r_valid_o	),
		.inst_addr_r_ready_i	(	inst_addr_r_ready_i	),
		.inst_r_resp_i			(	inst_r_resp_i	),
		.inst_r_valid_i			(	inst_r_valid_i	),
		.inst_r_ready_o			(	inst_r_ready_o	),
		.inst_i    ( inst    ),
		.pc        ( pc  ),
		.inst_ren  ( inst_ren  ),
		.invalid   ( invalid   ),
		.mem_wen_o    ( mem_wen_i    ),
		.mem_ren_o    ( mem_ren_i    ),
        .mem_wdata_o  ( mem_wdata_i  ),
        .mem_waddr_o  ( mem_waddr_i    ),
        .mem_raddr_o  ( mem_raddr_i  ),
        .mem_wmask_o    ( mem_wmask_i    ),
        .mem_rmask_o    ( mem_rmask_i    ),
        .mem_rdata_i  ( mem_rdata_rare_o),
		.finish    ( finish    )
	);

	ysyx_22041211_AXI_SRAM#(
		.ADDR_LEN       ( 32 ),
		.DATA_LEN       ( 32 )
	)u_ysyx_22041211_inst_AXI_SRAM(
		.rst            ( rst            ),
		.clk            ( clk            ),
		.addr_r_addr_i  ( pc  ),
		.addr_r_valid_i ( inst_addr_r_valid_o ),
		.addr_r_ready_o ( inst_addr_r_ready_i      ),
		.r_data_o  		( inst  ),
		.r_resp_o 		( inst_r_resp_i ),
		.r_valid_o      ( inst_r_valid_i      ),
		.r_ready_i      ( inst_r_ready_o      )

		// .addr_w_addr_i  ( 		0		 ),
		// .addr_w_valid_i ( 		0		 ),
		// .addr_w_ready_o ( 				 ),
		// .w_data_o       ( 		0		 ),
		// .w_strb_o       ( 		0		 ),
		// .w_valid_o      ( 		0		 ),
		// .w_ready_i      ( 				 ),
		// .bkwd_resp__o   ( 				 ),
		// .bkwd_valid_o   ( 				 ),
		// .bkwd_ready_i   ( 		0		 )
	);



	// ysyx_22041211_SRAM#(
	// 	.ADDR_LEN     ( 32 ),
	// 	.DATA_LEN     ( 32 )
	// )u_ysyx_22041211_inst_SRAM(
	// 	.rst          ( rst          ),
	// 	.clk          ( clk          ),
	// 	.ren          ( inst_ren    ),
	// 	.mem_wen_i    ( 0    ),
	// 	.mem_wdata_i  ( 0  ),
	// 	.mem_waddr_i  ( 0  ),
	// 	.mem_raddr_i  ( inst_addr  ),
	// 	.mem_wmask    ( 0    ),
	// 	.mem_rmask    ( 8'b00001111 ),
	// 	.mem_rdata_usigned_o  ( inst  )
	// );

	ysyx_22041211_SRAM#(
        .ADDR_LEN     ( 32 ),
        .DATA_LEN     ( 32 )
    )u_ysyx_22041211_data_SRAM(
        .rst          ( rst          ),
        .clk          ( clk          ),
        .ren          ( mem_ren_i   ),
        .mem_wen_i    ( mem_wen_i    ),
        .mem_wdata_i  ( mem_wdata_i  ),
        .mem_waddr_i  ( mem_waddr_i    ),
        .mem_raddr_i  ( mem_raddr_i  ),
        .mem_wmask    ( mem_wmask_i    ),
        .mem_rmask    ( mem_rmask_i    ),
        .mem_rdata_usigned_o( mem_rdata_rare_o)
    );

endmodule
