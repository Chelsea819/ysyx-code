module ysyx_22041211_inst_axi(
	//Addr Read
	output		[ADDR_LEN - 1:0]		addr_r_addr_i,
	output		                		addr_r_valid_i,
	input		                		addr_r_ready_o,

	// Read data
	input		[DATA_LEN - 1:0]		r_data_o	,
	input		[1:0]					r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		r_valid_o	,
	output		                		r_ready_i	,

	// Addr Write
	output		[ADDR_LEN - 1:0]		addr_w_addr_i,	// 写地址
	output		                		addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	input		                		addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	output		[DATA_LEN - 1:0]		w_data_o	,	// 写出的数据
	output		[3:0]					w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					bkwd_resp__o,	// 写回复信号，写操作是否成功
	input		                		bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	output		                		bkwd_ready_i	// 主设备已准备好接收写回复信号
);