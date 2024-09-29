`include "ysyx_22041211_define.v"
module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32) (
	input								clk 		,
	input								rst 		,
	output			[ADDR_LEN - 1:0]	pc			,
	output								invalid		,
	output								finish
);
	wire			[DATA_LEN - 1:0]	inst		;

	// IFU-AXI
	// Addr Read
	wire	[ADDR_LEN - 1:0]			inst_addr_r_addr_o;
	wire	                			inst_addr_r_valid_o;
	wire	                			inst_addr_r_ready_i;

	// Read data
	// wire		[DATA_LEN - 1:0]		inst_r_data_i	;
	wire		[1:0]					inst_r_resp_i	;	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	wire		                		inst_r_valid_i	;
	wire		                		inst_r_ready_o	;

	wire	[ADDR_LEN - 1:0]		data_addr_r_addr_o	;
	wire	                		data_addr_r_valid_o	;
	wire	                		data_addr_r_ready_i	;


	// Read data
	wire		[DATA_LEN - 1:0]		data_r_data_i	;
	wire		[1:0]					data_r_resp_i	;	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	wire		                		data_r_valid_i	;
	wire		                		data_r_ready_o	;

	// Addr Write
	wire	[ADDR_LEN - 1:0]		data_addr_w_addr_o	;	// 写地址
	wire	                		data_addr_w_valid_o	;	// 主设备给出的地址和相关控制信号有效
	wire	                		data_addr_w_ready_i	; // 从设备已准备好接收地址和相关的控制信号

	// Write data
	wire	[DATA_LEN - 1:0]		data_w_data_o	;	// 写出的数据
	wire	[3:0]					data_w_strb_o	;	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	wire	                		data_w_valid_o	;	// 主设备给出的数据和字节选通信号有效
	wire	                		data_w_ready_i	;	// 从设备已准备好接收数据选通信号

	// Backward
	wire		[1:0]					data_bkwd_resp_i	;	// 写回复信号，写操作是否成功
	wire		                		data_bkwd_valid_i	;	// 从设备给出的写回复信号是否有效
	wire		                		data_bkwd_ready_o	;	// 主设备已准备好接收写回复信号

	// Xbar
    //Addr Read
	wire		[1:0]						xbar_device		;
	wire	reg	[ADDR_LEN - 1:0]		xbar_addr_r_addr_o	;
	wire		                		xbar_addr_r_valid_o	;
	wire		                		xbar_addr_r_ready_i	;

	// Read data
	wire		[DATA_LEN - 1:0]		xbar_r_data_i		;
	wire		[1:0]					xbar_r_resp_i		;	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	wire		                		xbar_r_valid_i		;
	wire		                		xbar_r_ready_o		;

	// Addr Write
	wire	reg	[ADDR_LEN - 1:0]		xbar_addr_w_addr_o	;	// 写地址
	wire		                		xbar_addr_w_valid_o	;	// 主设备给出的地址和相关控制信号有效
	wire		                		xbar_addr_w_ready_i	; // 从设备已准备好接收地址和相关的控制信号

	// Write data
	wire	reg	[DATA_LEN - 1:0]		xbar_w_data_o	;	// 写出的数据
	wire	reg	[3:0]					xbar_w_strb_o	;	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	wire		                		xbar_w_valid_o	;	// 主设备给出的数据和字节选通信号有效
	wire		                		xbar_w_ready_i	;	// 从设备已准备好接收数据选通信号

	// Backward
	wire		[1:0]					xbar_bkwd_resp_i	;	// 写回复信号，写操作是否成功
	wire		                		xbar_bkwd_valid_i	;	// 从设备给出的写回复信号是否有效
	wire		                		xbar_bkwd_ready_o	;// 主设备已准备好接收写回复信号

	// UART
	// Addr Write
	wire		                		uart_addr_w_valid_i;	// 主设备给出的地址和相关控制信号有效
	wire		                		uart_addr_w_ready_o; // 从设备已准备好接收地址和相关的控制信号

	// Write data
	wire		[DATA_LEN - 1:0]		uart_w_data_i	;	// 写出的数据
	wire		                		uart_w_valid_i	;	// 主设备给出的数据和字节选通信号有效
	wire		                		uart_w_ready_o	;	// 从设备已准备好接收数据选通信号

	// Backward
	wire		[1:0]					uart_bkwd_resp_o	;	// 写回复信号，写操作是否成功
	wire		                		uart_bkwd_valid_o	;	// 从设备给出的写回复信号是否有效
	wire		                		uart_bkwd_ready_i	;	// 主设备已准备好接收写回复信号

	// AXI-SRAM
	wire	[ADDR_LEN - 1:0]		sram_addr_r_addr_o	;
	wire	                		sram_addr_r_valid_o	;
	wire	                		sram_addr_r_ready_i	;


	// Read data
	wire		[DATA_LEN - 1:0]	sram_r_data_i	;
	wire		[1:0]				sram_r_resp_i	;	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	wire		                	sram_r_valid_i	;
	wire		                	sram_r_ready_o	;

	// Addr Write
	wire	[ADDR_LEN - 1:0]		sram_addr_w_addr_o	;	// 写地址
	wire	                		sram_addr_w_valid_o	;	// 主设备给出的地址和相关控制信号有效
	wire	                		sram_addr_w_ready_i	; // 从设备已准备好接收地址和相关的控制信号

	// Write data
	wire	[DATA_LEN - 1:0]		sram_w_data_o	;	// 写出的数据
	wire	[3:0]					sram_w_strb_o	;	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	wire	                		sram_w_valid_o	;	// 主设备给出的数据和字节选通信号有效
	wire	                		sram_w_ready_i	;	// 从设备已准备好接收数据选通信号

	// Backward
	wire		[1:0]				sram_bkwd_resp_i	;	// 写回复信号，写操作是否成功
	wire		                	sram_bkwd_valid_i	;	// 从设备给出的写回复信号是否有效
	wire		                	sram_bkwd_ready_o	;	// 主设备已准备好接收写回复信号

	// CLINT
	wire	[ADDR_LEN - 1:0]		clint_addr_r_addr_o	;
	wire	                		clint_addr_r_valid_o	;
	wire	                		clint_addr_r_ready_i	;


	// Read data
	wire		[DATA_LEN - 1:0]	clint_r_data_i	;
	wire		[1:0]				clint_r_resp_i	;	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	wire		                	clint_r_valid_i	;
	wire		                	clint_r_ready_o	;

// 检测到ebreak
    import "DPI-C" function void ifebreak_func(int inst);
    always @(*)
        ifebreak_func(inst);

// 为ITRACE提供指令
    import "DPI-C" function void inst_get(int inst);
    always @(*)
        inst_get(inst);

	ysyx_22041211_cpu#(
		.DATA_LEN            ( 32 ),
		.ADDR_LEN 	         ( 32 )
	)u_ysyx_22041211_cpu(
		.clk                 ( clk                 ),
		.rst                 ( rst                 ),
		.inst_addr_r_addr_o	 ( inst_addr_r_addr_o  ),
		.inst_addr_r_valid_o ( inst_addr_r_valid_o ),
		.inst_addr_r_ready_i ( inst_addr_r_ready_i ),
		.inst_r_resp_i       ( inst_r_resp_i       ),
		.inst_r_valid_i      ( inst_r_valid_i      ),
		.inst_r_ready_o      ( inst_r_ready_o      ),
		.data_addr_r_addr_o  ( data_addr_r_addr_o  ),
		.data_addr_r_valid_o ( data_addr_r_valid_o ),
		.data_addr_r_ready_i ( data_addr_r_ready_i ),
		.data_r_data_i       ( data_r_data_i       ),
		.data_r_resp_i       ( data_r_resp_i       ),
		.data_r_valid_i      ( data_r_valid_i      ),
		.data_r_ready_o      ( data_r_ready_o      ),
		.data_addr_w_addr_o  ( data_addr_w_addr_o  ),
		.data_addr_w_valid_o ( data_addr_w_valid_o ),
		.data_addr_w_ready_i ( data_addr_w_ready_i ),
		.data_w_data_o       ( data_w_data_o       ),
		.data_w_strb_o       ( data_w_strb_o       ),
		.data_w_valid_o      ( data_w_valid_o      ),
		.data_w_ready_i      ( data_w_ready_i      ),
		.data_bkwd_resp_i    ( data_bkwd_resp_i    ),
		.data_bkwd_valid_i   ( data_bkwd_valid_i   ),
		.data_bkwd_ready_o   ( data_bkwd_ready_o   ),
		.inst_i              ( inst              ),
		.pc                  ( pc                  ),
		.invalid             ( invalid             ),
		.finish              ( finish              )
	);

	ysyx_22041211_AXI_CTL#(
		.ADDR_LEN            ( 32 ),
		.DATA_LEN            ( 32 )
	)u_ysyx_22041211_AXI_CTL(
		.rst                 ( rst                ),
		.clk                 ( clk                 ),
		.inst_addr_r_addr_i  ( inst_addr_r_addr_o  ),
		.inst_addr_r_valid_i ( inst_addr_r_valid_o ),
		.inst_addr_r_ready_o ( inst_addr_r_ready_i ),
		.inst_r_data_o       ( inst       ),
		.inst_r_resp_o       ( inst_r_resp_i       ),
		.inst_r_valid_o      ( inst_r_valid_i      ),
		.inst_r_ready_i      ( inst_r_ready_o      ),
		.data_addr_r_addr_i  ( data_addr_r_addr_o  ),
		.data_addr_r_valid_i ( data_addr_r_valid_o ),
		.data_addr_r_ready_o ( data_addr_r_ready_i ),
		.data_r_data_o       ( data_r_data_i       ),
		.data_r_resp_o       ( data_r_resp_i       ),
		.data_r_valid_o      ( data_r_valid_i      ),
		.data_r_ready_i      ( data_r_ready_o      ),
		.data_addr_w_addr_i  ( data_addr_w_addr_o  ),
		.data_addr_w_valid_i ( data_addr_w_valid_o ),
		.data_addr_w_ready_o ( data_addr_w_ready_i ),
		.data_w_data_i       ( data_w_data_o       ),
		.data_w_strb_i       ( data_w_strb_o       ),
		.data_w_valid_i      ( data_w_valid_o      ),
		.data_w_ready_o      ( data_w_ready_i      ),
		.data_bkwd_resp_o    ( data_bkwd_resp_i    ),
		.data_bkwd_valid_o   ( data_bkwd_valid_i   ),
		.data_bkwd_ready_i   ( data_bkwd_ready_o   ),
		.xbar_device		 ( xbar_device 		   ),
		.xbar_addr_r_addr_o  ( xbar_addr_r_addr_o  ),
		.xbar_addr_r_valid_o ( xbar_addr_r_valid_o ),
		.xbar_addr_r_ready_i ( xbar_addr_r_ready_i ),
		.xbar_r_data_i       ( xbar_r_data_i       ),
		.xbar_r_resp_i       ( xbar_r_resp_i       ),
		.xbar_r_valid_i      ( xbar_r_valid_i      ),
		.xbar_r_ready_o      ( xbar_r_ready_o      ),
		.xbar_addr_w_addr_o  ( xbar_addr_w_addr_o  ),
		.xbar_addr_w_valid_o ( xbar_addr_w_valid_o ),
		.xbar_addr_w_ready_i ( xbar_addr_w_ready_i ),
		.xbar_w_data_o       ( xbar_w_data_o       ),
		.xbar_w_strb_o       ( xbar_w_strb_o       ),
		.xbar_w_valid_o      ( xbar_w_valid_o      ),
		.xbar_w_ready_i      ( xbar_w_ready_i      ),
		.xbar_bkwd_resp_i    ( xbar_bkwd_resp_i    ),
		.xbar_bkwd_valid_i   ( xbar_bkwd_valid_i   ),
		.xbar_bkwd_ready_o   ( xbar_bkwd_ready_o   )
	);

	ysyx_22041211_xbar#(
		.ADDR_LEN               ( 32 ),
		.DATA_LEN 		        ( 32 )
	)u_ysyx_22041211_xbar(
		.rstn                   ( ~rst                   ),
		.clk                    ( clk                    ),
		.axi_device				( xbar_device			 ),
		.axi_ctl_addr_r_addr_i  ( xbar_addr_r_addr_o  ),
		.axi_ctl_addr_r_valid_i ( xbar_addr_r_valid_o ),
		.axi_ctl_addr_r_ready_o ( xbar_addr_r_ready_i ),
		.axi_ctl_r_data_o       ( xbar_r_data_i       ),
		.axi_ctl_r_resp_o       ( xbar_r_resp_i       ),
		.axi_ctl_r_valid_o      ( xbar_r_valid_i      ),
		.axi_ctl_r_ready_i      ( xbar_r_ready_o      ),
		.axi_ctl_addr_w_addr_i  ( xbar_addr_w_addr_o  ),
		.axi_ctl_addr_w_valid_i ( xbar_addr_w_valid_o ),
		.axi_ctl_addr_w_ready_o ( xbar_addr_w_ready_i ),
		.axi_ctl_w_data_i       ( xbar_w_data_o       ),
		.axi_ctl_w_strb_i       ( xbar_w_strb_o       ),
		.axi_ctl_w_valid_i      ( xbar_w_valid_o      ),
		.axi_ctl_w_ready_o      ( xbar_w_ready_i      ),
		.axi_ctl_bkwd_resp_o    ( xbar_bkwd_resp_i    ),
		.axi_ctl_bkwd_valid_o   ( xbar_bkwd_valid_i   ),
		.axi_ctl_bkwd_ready_i   ( xbar_bkwd_ready_o   ),
		.uart_addr_w_valid_i    ( uart_addr_w_valid_i    ),
		.uart_addr_w_ready_o    ( uart_addr_w_ready_o    ),
		.uart_w_data_i          ( uart_w_data_i          ),
		.uart_w_valid_i         ( uart_w_valid_i         ),
		.uart_w_ready_o         ( uart_w_ready_o         ),
		.uart_bkwd_resp_o       ( uart_bkwd_resp_o       ),
		.uart_bkwd_valid_o      ( uart_bkwd_valid_o      ),
		.uart_bkwd_ready_i      ( uart_bkwd_ready_i      ),
		.sram_addr_r_addr_o     ( sram_addr_r_addr_o     ),
		.sram_addr_r_valid_o    ( sram_addr_r_valid_o    ),
		.sram_addr_r_ready_i    ( sram_addr_r_ready_i    ),
		.sram_r_data_i          ( sram_r_data_i          ),
		.sram_r_resp_i          ( sram_r_resp_i          ),
		.sram_r_valid_i         ( sram_r_valid_i         ),
		.sram_r_ready_o         ( sram_r_ready_o         ),
		.sram_addr_w_addr_o     ( sram_addr_w_addr_o     ),
		.sram_addr_w_valid_o    ( sram_addr_w_valid_o    ),
		.sram_addr_w_ready_i    ( sram_addr_w_ready_i    ),
		.sram_w_data_o          ( sram_w_data_o          ),
		.sram_w_strb_o          ( sram_w_strb_o          ),
		.sram_w_valid_o         ( sram_w_valid_o         ),
		.sram_w_ready_i         ( sram_w_ready_i         ),
		.sram_bkwd_resp_i       ( sram_bkwd_resp_i       ),
		.sram_bkwd_valid_i      ( sram_bkwd_valid_i      ),
		.sram_bkwd_ready_o      ( sram_bkwd_ready_o      ),
		.clint_addr_r_addr_i    ( clint_addr_r_addr_o    ),
		.clint_addr_r_valid_i   ( clint_addr_r_valid_o   ),
		.clint_addr_r_ready_o   ( clint_addr_r_ready_i   ),
		.clint_r_data_o         ( clint_r_data_i         ),
		.clint_r_resp_o         ( clint_r_resp_i         ),
		.clint_r_valid_o        ( clint_r_valid_i        ),
		.clint_r_ready_i        ( clint_r_ready_o        )
	);

	ysyx_22041211_AXI_SRAM#(
		.ADDR_LEN       ( 32 ),
		.DATA_LEN       ( 32 )
	)u_ysyx_22041211_AXI_SRAM(
		.rstn            ( ~rst            ),
		.clk            ( clk            ),
		.addr_r_addr_i  ( sram_addr_r_addr_o  ),
		.addr_r_valid_i ( sram_addr_r_valid_o ),
		.addr_r_ready_o ( sram_addr_r_ready_i      ),
		.r_data_o  		( sram_r_data_i  ),
		.r_resp_o 		( sram_r_resp_i ),
		.r_valid_o      ( sram_r_valid_i      ),
		.r_ready_i      ( sram_r_ready_o      ),

		.addr_w_addr_i  ( sram_addr_w_addr_o ),
		.addr_w_valid_i ( sram_addr_w_valid_o ),
		.addr_w_ready_o ( sram_addr_w_ready_i ),
		.w_data_i       ( sram_w_data_o ),
		.w_strb_i       ( sram_w_strb_o ),
		.w_valid_i      ( sram_w_valid_o ),
		.w_ready_o      ( sram_w_ready_i ),
		.bkwd_resp_o    ( sram_bkwd_resp_i ),
		.bkwd_valid_o   ( sram_bkwd_valid_i ),
		.bkwd_ready_i   ( sram_bkwd_ready_o )
	);

	ysyx_22041211_UART#(
		.DATA_LEN       ( 32 )
	)u_ysyx_22041211_AXI_UART(
		.rstn           ( ~rst           ),
		.clk            ( clk            ),
		.addr_w_valid_i ( uart_addr_w_valid_i ),
		.addr_w_ready_o ( uart_addr_w_ready_o ),
		.w_data_i       ( uart_w_data_i       ),
		.w_valid_i      ( uart_w_valid_i      ),
		.w_ready_o      ( uart_w_ready_o      ),
		.bkwd_resp_o    ( uart_bkwd_resp_o    ),
		.bkwd_valid_o   ( uart_bkwd_valid_o   ),
		.bkwd_ready_i   ( uart_bkwd_ready_i   )
	);

	ysyx_22041211_CLINT#(
		.ADDR_LEN       ( 32 ),
		.DATA_LEN 	    ( 32 )
	)u_ysyx_22041211_CLINT(
		.rstn           ( ~rst           ),
		.clk            ( clk            ),
		.addr_r_addr_i  ( clint_addr_r_addr_o  ),
		.addr_r_valid_i ( clint_addr_r_valid_o ),
		.addr_r_ready_o ( clint_addr_r_ready_i ),
		.r_data_o       ( clint_r_data_i       ),
		.r_resp_o       ( clint_r_resp_i       ),
		.r_valid_o      ( clint_r_valid_i      ),
		.r_ready_i      ( clint_r_ready_o      )
	);



	// ysyx_22041211_AXI_SRAM#(
	// 	.ADDR_LEN       ( 32 ),
	// 	.DATA_LEN       ( 32 )
	// )u_ysyx_22041211_inst_AXI_SRAM(
	// 	.rstn            ( ~rst            ),
	// 	.clk            ( clk            ),
	// 	.addr_r_addr_i  ( inst_addr_r_addr_o  ),
	// 	.addr_r_valid_i ( inst_addr_r_valid_o ),
	// 	.addr_r_ready_o ( inst_addr_r_ready_i      ),
	// 	.r_data_o  		( inst  ),
	// 	.r_resp_o 		( inst_r_resp_i ),
	// 	.r_valid_o      ( inst_r_valid_i      ),
	// 	.r_ready_i      ( inst_r_ready_o      ),

	// 	.addr_w_addr_i  ( 		0		 ),
	// 	.addr_w_valid_i ( 		0		 ),
	// 	.addr_w_ready_o ( 				 ),
	// 	.w_data_i       ( 		0		 ),
	// 	.w_strb_i       ( 		0		 ),
	// 	.w_valid_i      ( 		0		 ),
	// 	.w_ready_o      ( 				 ),
	// 	.bkwd_resp_o   ( 				 ),
	// 	.bkwd_valid_o   ( 				 ),
	// 	.bkwd_ready_i   ( 		0		 )
	// );



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

	// ysyx_22041211_AXI_SRAM#(
	// 	.ADDR_LEN       ( 32 ),
	// 	.DATA_LEN       ( 32 )
	// )u_ysyx_22041211_data_AXI_SRAM(
	// 	.rstn            ( ~rst            ),
	// 	.clk            ( clk            ),
	// 	.addr_r_addr_i  ( data_addr_r_addr_o  ),
	// 	.addr_r_valid_i ( data_addr_r_valid_o ),
	// 	.addr_r_ready_o ( data_addr_r_ready_i      ),
	// 	.r_data_o  		( data_r_data_i  ),
	// 	.r_resp_o 		( data_r_resp_i ),
	// 	.r_valid_o      ( data_r_valid_i      ),
	// 	.r_ready_i      ( data_r_ready_o      ),

	// 	.addr_w_addr_i  ( data_addr_w_addr_o ),
	// 	.addr_w_valid_i ( data_addr_w_valid_o ),
	// 	.addr_w_ready_o ( data_addr_w_ready_i ),
	// 	.w_data_i       ( data_w_data_o ),
	// 	.w_strb_i       ( data_w_strb_o ),
	// 	.w_valid_i      ( data_w_valid_o ),
	// 	.w_ready_o      ( data_w_ready_i ),
	// 	.bkwd_resp_o   ( data_bkwd_resp_i ),
	// 	.bkwd_valid_o   ( data_bkwd_valid_i ),
	// 	.bkwd_ready_i   ( data_bkwd_ready_o )
	// );

endmodule
