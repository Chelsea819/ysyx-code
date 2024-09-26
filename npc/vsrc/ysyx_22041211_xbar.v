/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clk rst waddr wdata wen wmask
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"
module ysyx_22041211_xbar #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input								rstn		,
    input		                		clk			,

	//Addr Read
	input		[ADDR_LEN - 1:0]		axi_ctl_addr_r_addr_i,
	input		                		axi_ctl_addr_r_valid_i,
	output		                		axi_ctl_addr_r_ready_o,

	// Read data
	output	reg	[DATA_LEN - 1:0]		axi_ctl_r_data_o	,
	output		[1:0]					axi_ctl_r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	output		                		axi_ctl_r_valid_o	,
	input		                		axi_ctl_r_ready_i	,

	// Addr Write
	input		[ADDR_LEN - 1:0]		axi_ctl_addr_w_addr_i,	// 写地址
	input		                		axi_ctl_addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	output		                		axi_ctl_addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	input		[DATA_LEN - 1:0]		axi_ctl_w_data_i	,	// 写出的数据
	input		[3:0]					axi_ctl_w_strb_i	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	input		                		axi_ctl_w_valid_i	,	// 主设备给出的数据和字节选通信号有效
	output		                		axi_ctl_w_ready_o	,	// 从设备已准备好接收数据选通信号

	// Backward
	output		[1:0]					axi_ctl_bkwd_resp_o,	// 写回复信号，写操作是否成功
	output		                		axi_ctl_bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	input		                		axi_ctl_bkwd_ready_i,	// 主设备已准备好接收写回复信号

	// UART
	// Addr Write
	output		                		uart_addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	input		                		uart_addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	output		[DATA_LEN - 1:0]		uart_w_data_i	,	// 写出的数据
	output		                		uart_w_valid_i	,	// 主设备给出的数据和字节选通信号有效
	input		                		uart_w_ready_o	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					uart_bkwd_resp_o,	// 写回复信号，写操作是否成功
	input		                		uart_bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	output		                		uart_bkwd_ready_i,	// 主设备已准备好接收写回复信号

	// AXI-sram
    //Addr Read
	output	reg	[ADDR_LEN - 1:0]		sram_addr_r_addr_o,
	output		                		sram_addr_r_valid_o,
	input		                		sram_addr_r_ready_i,

	// Read data
	input		[DATA_LEN - 1:0]		sram_r_data_i	,
	input		[1:0]					sram_r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		sram_r_valid_i	,
	output		                		sram_r_ready_o	,

	// Addr Write
	output	reg	[ADDR_LEN - 1:0]		sram_addr_w_addr_o,	// 写地址
	output		                		sram_addr_w_valid_o,	// 主设备给出的地址和相关控制信号有效
	input		                		sram_addr_w_ready_i, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	output	reg	[DATA_LEN - 1:0]		sram_w_data_o	,	// 写出的数据
	output	reg	[3:0]					sram_w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		sram_w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		sram_w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					sram_bkwd_resp_i,	// 写回复信号，写操作是否成功
	input		                		sram_bkwd_valid_i,	// 从设备给出的写回复信号是否有效
	output		                		sram_bkwd_ready_o

);	

	wire						axi_device;

	assign axi_device = (axi_ctl_addr_w_addr_i == `SERIAL_PORT) ? `AXI_XBAR_UART : `AXI_XBAR_SRAM;

	assign uart_w_data_i = axi_ctl_w_data_i;
	assign sram_addr_r_addr_o = axi_ctl_addr_r_addr_i;
	assign sram_addr_w_addr_o = axi_ctl_addr_w_addr_i;
	assign sram_w_data_o = axi_ctl_w_data_i;
	assign sram_w_strb_o = axi_ctl_w_strb_i;

	assign {axi_ctl_addr_r_ready_o, axi_ctl_r_data_o, axi_ctl_r_resp_o, axi_ctl_r_valid_o, 
			axi_ctl_addr_w_ready_o, axi_ctl_w_ready_o, 
			axi_ctl_bkwd_resp_o, axi_ctl_bkwd_valid_o } = (axi_device == `AXI_XBAR_UART) ? {1'b0, 32'b0, 2'b0, 1'b0, 
																							uart_addr_w_ready_o, uart_w_ready_o,
																							uart_bkwd_resp_o, uart_bkwd_valid_o} :
															(axi_device == `AXI_XBAR_SRAM) ? {sram_addr_r_ready_i, sram_r_data_i, sram_r_resp_i, sram_r_valid_i, 
																							sram_addr_w_ready_i, sram_w_ready_i,
																							sram_bkwd_resp_i, sram_bkwd_valid_i} :
																							0;

	assign {uart_addr_w_valid_i, uart_w_valid_i, uart_bkwd_ready_i } = (axi_device == `AXI_XBAR_UART) ? 
																			{axi_ctl_addr_w_valid_i, axi_ctl_w_valid_i, axi_ctl_bkwd_ready_i} : 0;
	
	assign {sram_addr_r_valid_o, sram_r_ready_o, 
			sram_addr_w_valid_o, sram_w_valid_o, sram_bkwd_ready_o} = (axi_device == `AXI_XBAR_SRAM) ? 
																			{axi_ctl_addr_r_valid_i, axi_ctl_r_ready_i, 
																			axi_ctl_addr_w_valid_i, axi_ctl_w_valid_i, axi_ctl_bkwd_ready_i} : 0;
	
	

endmodule
