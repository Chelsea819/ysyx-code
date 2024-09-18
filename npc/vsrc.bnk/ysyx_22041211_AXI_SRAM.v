/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clk rst waddr wdata wen wmask
`include "./ysyx_22041211_define.v"
module ysyx_22041211_AXI_SRAM #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input								rst			,
    input		                		clk			,
    input		                		ren			,

	//Addr Read
	input		[ADDR_LEN - 1:0]		addr_r_addr_i,
	input		                		addr_r_valid_i,
	output		                		addr_r_ready_o,

	// Read data
	output		[DATA_LEN - 1:0]		r_data_o	,
	output		[1:0]					r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	output		                		r_valid_o	,
	input		                		r_ready_i	,

	// Addr Write
	input		[ADDR_LEN - 1:0]		addr_w_addr_i,	// 写地址
	input		                		addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	output		                		addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	input		[DATA_LEN - 1:0]		w_data_o	,	// 写出的数据
	input		[3:0]					w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	input		                		w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	output		                		w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	output		[1:0]					bkwd_resp__o,	// 写回复信号，写操作是否成功
	output		                		bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	input		                		bkwd_ready_i	// 主设备已准备好接收写回复信号

);	
	ysyx_22041211_SRAM#(
		.ADDR_LEN     ( 32 ),
		.DATA_LEN     ( 32 )
	)u_ysyx_22041211_SRAM(
		.rst          ( rst         ),
		.clk          ( clk         ),
		.ren          (           	),
		.mem_wen_i    (     		),
		.mem_wdata_i  (   			),
		.mem_waddr_i  (   			),
		.mem_raddr_i  (   			),
		.mem_wmask    (     		),
		.mem_rmask    (     		),
		.mem_rdata_usigned_o (   	)
	);

	

endmodule
