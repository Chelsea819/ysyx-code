/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clock reset waddr wdata wen wmask
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"
module ysyx_22041211_xbar #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input								rstn		,
    input		                		clock			,

	//Addr Read
	input		[ADDR_LEN - 1:0]		axi_ctl_addr_r_addr_i,
	input		                		axi_ctl_addr_r_valid_i,
	output		                		axi_ctl_addr_r_ready_o,
	input		[3:0]                	axi_ctl_addr_r_id_i,	// 谁发出的读请求
	input		[7:0]                	axi_ctl_addr_r_len_i,	// 突发长度 用于一个burst中有多少个beat
	input		[2:0]                	axi_ctl_addr_r_size_i,	// 突发长度 设置1个beat的宽度，不可超过数据总线的宽度
	input		[1:0]                	axi_ctl_addr_r_burst_i,	// 突发类型

	input								axi_device,


	// Read data
	output	reg	[DATA_LEN - 1:0]		axi_ctl_r_data_o	,
	output		[1:0]					axi_ctl_r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	output		                		axi_ctl_r_valid_o	,
	input		                		axi_ctl_r_ready_i	,
	output		                		axi_ctl_r_last_o	,
	output		[3:0]           		axi_ctl_r_id_o		,	// 读到的数据给谁？

	// Addr Write
	input		[ADDR_LEN - 1:0]		axi_ctl_addr_w_addr_i,	// 写地址
	input		                		axi_ctl_addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	output		                		axi_ctl_addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号
	input		[3:0]                	axi_ctl_addr_w_id_i,	// 谁发出的写请求
	input		[7:0]                	axi_ctl_addr_w_len_i,	// 突发长度，这个字段标识每次突发传输的传输次数
	input		[2:0]                	axi_ctl_addr_w_size_i,	// 突发大小，这个字段表示每次突发传输的大小
	input		[1:0]                	axi_ctl_addr_w_burst_i,	// 突发类型，包括突发类型和突发大小信息，该字段决定了每次突发传输时地址的计算方法

	// Write data
	input		[DATA_LEN - 1:0]		axi_ctl_w_data_i	,	// 写出的数据
	input		[3:0]					axi_ctl_w_strb_i	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	input		                		axi_ctl_w_valid_i	,	// 主设备给出的数据和字节选通信号有效
	output		                		axi_ctl_w_ready_o	,	// 从设备已准备好接收数据选通信号
	input		                		axi_ctl_w_last_i	,	// 该信号用于标识当前传输是否为突发传输中的最后一次传输


	// Backward
	output		[1:0]					axi_ctl_bkwd_resp_o,	// 写回复信号，写操作是否成功
	output		                		axi_ctl_bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	input		                		axi_ctl_bkwd_ready_i,	// 主设备已准备好接收写回复信号
	output		[3:0]                	axi_ctl_bkwd_id_o,		// 写响应ID，该信号用于标识写响应传输

	// AXI-sram
    //Addr Read
	output	reg	[ADDR_LEN - 1:0]		axi_addr_r_addr_o,
	output		                		axi_addr_r_valid_o,
	input		                		axi_addr_r_ready_i,
	output		[3:0]                	axi_addr_r_id_o,	// 谁发出的读请求
	output		[7:0]                	axi_addr_r_len_o,	// 突发长度 用于一个burst中有多少个beat
	output		[2:0]                	axi_addr_r_size_o,	// 突发长度 设置1个beat的宽度，不可超过数据总线的宽度
	output		[1:0]                	axi_addr_r_burst_o,	// 突发类型

	// Read data
	input		[DATA_LEN - 1:0]		axi_r_data_i	,
	input		[1:0]					axi_r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		axi_r_valid_i	,
	output		                		axi_r_ready_o	,
	input		                		axi_r_last_i	,
	input		[3:0]           		axi_r_id_i		,	// 读到的数据给谁？

	// Addr Write
	output	reg	[ADDR_LEN - 1:0]		axi_addr_w_addr_o,	// 写地址
	output		                		axi_addr_w_valid_o,	// 主设备给出的地址和相关控制信号有效
	input		                		axi_addr_w_ready_i, // 从设备已准备好接收地址和相关的控制信号
	output		[3:0]                	axi_addr_w_id_o,	// 谁发出的写请求
	output		[7:0]                	axi_addr_w_len_o,	// 突发长度，这个字段标识每次突发传输的传输次数
	output		[2:0]                	axi_addr_w_size_o,	// 突发大小，这个字段表示每次突发传输的大小
	output		[1:0]                	axi_addr_w_burst_o,	// 突发类型，包括突发类型和突发大小信息，该字段决定了每次突发传输时地址的计算方法

	// Write data
	output	reg	[DATA_LEN - 1:0]		axi_w_data_o	,	// 写出的数据
	output	reg	[3:0]					axi_w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		axi_w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		axi_w_ready_i	,	// 从设备已准备好接收数据选通信号
	output		                		axi_w_last_o	,	// 该信号用于标识当前传输是否为突发传输中的最后一次传输
	
	// Backward
	input		[1:0]					axi_bkwd_resp_i,	// 写回复信号，写操作是否成功
	input		                		axi_bkwd_valid_i,	// 从设备给出的写回复信号是否有效
	output		                		axi_bkwd_ready_o,
	input		[3:0]                	axi_bkwd_id_i,		// 写响应ID，该信号用于标识写响应传输

	// CLINT
	//Addr Read
	output		[ADDR_LEN - 1:0]		clint_addr_r_addr_i,
	output		                		clint_addr_r_valid_i,
	input		                		clint_addr_r_ready_o,

	// Read data
	input	reg	[DATA_LEN - 1:0]		clint_r_data_o	,
	input		[1:0]					clint_r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		clint_r_valid_o	,
	output		                		clint_r_ready_i	

);	
	assign axi_addr_r_addr_o = axi_ctl_addr_r_addr_i;
	assign axi_addr_w_addr_o = axi_ctl_addr_w_addr_i;
	assign axi_w_data_o = axi_ctl_w_data_i;
	assign axi_w_strb_o = axi_ctl_w_strb_i;
	assign clint_addr_r_addr_i = axi_ctl_addr_r_addr_i;

	assign {axi_ctl_addr_r_ready_o, 
			axi_ctl_r_data_o, axi_ctl_r_resp_o, axi_ctl_r_valid_o, axi_ctl_r_last_o, axi_ctl_r_id_o, 
			axi_ctl_addr_w_ready_o, 
			axi_ctl_w_ready_o, 
			axi_ctl_bkwd_resp_o, axi_ctl_bkwd_valid_o, axi_ctl_bkwd_id_o } = (axi_device == `AXI_XBAR_CLINT) ? {clint_addr_r_ready_o, clint_r_data_o, clint_r_resp_o, clint_r_valid_o, 1'b0, 4'b0,
																							1'b0, 1'b0,
																							2'b0, 1'b0, 4'b0} :
																							{axi_addr_r_ready_i, 
																							axi_r_data_i, axi_r_resp_i, axi_r_valid_i, axi_r_last_i, axi_r_id_i, 
																							axi_addr_w_ready_i, 
																							axi_w_ready_i, 
																							axi_bkwd_resp_i, axi_bkwd_valid_i, axi_bkwd_id_i};

	
	assign {axi_addr_r_valid_o, axi_addr_r_id_o, axi_addr_r_len_o, axi_addr_r_size_o, axi_addr_r_burst_o, 
			axi_r_ready_o, 
			axi_addr_w_valid_o, axi_addr_w_id_o, axi_addr_w_len_o, axi_addr_w_size_o,  axi_addr_w_burst_o,
			axi_w_valid_o, axi_w_last_o, 
			axi_bkwd_ready_o											} = {axi_ctl_addr_r_valid_i, axi_ctl_addr_r_id_i, axi_ctl_addr_r_len_i, axi_ctl_addr_r_size_i, axi_ctl_addr_r_burst_i, 
																			axi_ctl_r_ready_i, 
																			axi_ctl_addr_w_valid_i, axi_ctl_addr_w_id_i, axi_ctl_addr_w_len_i, axi_ctl_addr_w_size_i, axi_ctl_addr_w_burst_i, 
																			axi_ctl_w_valid_i, axi_ctl_w_last_i, 
																			axi_ctl_bkwd_ready_i};
	
	assign {clint_addr_r_valid_i, clint_r_ready_i} = (axi_device == `AXI_XBAR_CLINT) ? 
																			{axi_ctl_addr_r_valid_i, axi_ctl_r_ready_i} : 0;
	
	

endmodule
