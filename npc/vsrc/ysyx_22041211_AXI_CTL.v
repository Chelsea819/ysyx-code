/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clk rst waddr wdata wen wmask
/* verilator lint_off UNOPTFLAT */
`include "ysyx_22041211_define.v"
module ysyx_22041211_AXI_CTL #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input								rst		,
    input		                		clk		,

	// IFU--inst-AXI
	//Addr Read
	input		[ADDR_LEN - 1:0]		inst_addr_r_addr_i,
	input		                		inst_addr_r_valid_i,
	output		                		inst_addr_r_ready_o,

	// Read data
	output	reg	[DATA_LEN - 1:0]		inst_r_data_o	,
	output		[1:0]					inst_r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	output		                		inst_r_valid_o	,
	input		                		inst_r_ready_i	,

	// data-AXI
	//Addr Read
	input		[ADDR_LEN - 1:0]		data_addr_r_addr_i,
	input		                		data_addr_r_valid_i,
	output		                		data_addr_r_ready_o,

	// Read data
	output	reg	[DATA_LEN - 1:0]		data_r_data_o	,
	output		[1:0]					data_r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	output		                		data_r_valid_o	,
	input		                		data_r_ready_i	,

	// Addr Write
	input		[ADDR_LEN - 1:0]		data_addr_w_addr_i,	// 写地址
	input		                		data_addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	output		                		data_addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	input		[DATA_LEN - 1:0]		data_w_data_i	,	// 写出的数据
	input		[3:0]					data_w_strb_i	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	input		                		data_w_valid_i	,	// 主设备给出的数据和字节选通信号有效
	output		                		data_w_ready_o	,	// 从设备已准备好接收数据选通信号

	// Backward
	output		[1:0]					data_bkwd_resp_o,	// 写回复信号，写操作是否成功
	output		                		data_bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	input		                		data_bkwd_ready_i,	// 主设备已准备好接收写回复信号

    // Xbar
	output	reg [1:0]						xbar_device,
    //Addr Read
	output	reg	[ADDR_LEN - 1:0]		xbar_addr_r_addr_o,
	output		                		xbar_addr_r_valid_o,
	input		                		xbar_addr_r_ready_i,

	// Read data
	input		[DATA_LEN - 1:0]		xbar_r_data_i	,
	input		[1:0]					xbar_r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		xbar_r_valid_i	,
	output		                		xbar_r_ready_o	,

	// Addr Write
	output	reg	[ADDR_LEN - 1:0]		xbar_addr_w_addr_o,	// 写地址
	output		                		xbar_addr_w_valid_o,	// 主设备给出的地址和相关控制信号有效
	input		                		xbar_addr_w_ready_i, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	output	reg	[DATA_LEN - 1:0]		xbar_w_data_o	,	// 写出的数据
	output	reg	[3:0]					xbar_w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		xbar_w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		xbar_w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					xbar_bkwd_resp_i,	// 写回复信号，写操作是否成功
	input		                		xbar_bkwd_valid_i,	// 从设备给出的写回复信号是否有效
	output		                		xbar_bkwd_ready_o	// 主设备已准备好接收写回复信号
);	
	parameter [1:0] AXI_CTL_IDLE = 2'b00, AXI_CTL_BUSY_DATA = 2'b01, AXI_CTL_BUSY_INST = 2'b10;

	reg				[1:0]			        con_state	;
	reg				[1:0]		        	next_state	;
	reg 			[1:0] 					xbar_device_tmp;

	// wire			[31:0]						xbar_waddr;
	
	// assign xbar_device = (data_addr_w_addr_i == `SERIAL_PORT && con_state == AXI_CTL_IDLE) ? `AXI_XBAR_UART : `AXI_XBAR_SRAM;

	always @(*) begin
		if (con_state == AXI_CTL_IDLE) begin
			if (data_addr_w_addr_i == `SERIAL_PORT) begin
				xbar_device = `AXI_XBAR_UART;
			end else begin
				xbar_device = `AXI_XBAR_SRAM;
			end
		end else begin
			xbar_device = xbar_device_tmp;
		end
	end


	always @(posedge clk ) begin
		if (rst) begin
			xbar_device_tmp <= 0;
		end else begin
			xbar_device_tmp <= xbar_device;
		end
	end


	// always @(*) begin
	// 	$display("con_state: [%b] next_state: [%b] xbar_addr_r_ready_i: [%b] inst_addr_r_valid_i: [%b] data_addr_r_valid_i[%b]",con_state, next_state, xbar_addr_r_ready_i, inst_addr_r_valid_i, data_addr_r_valid_i);
	// end
	// state trans
	always @(posedge clk ) begin
		if(rst)
			con_state <= AXI_CTL_IDLE;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
			AXI_CTL_IDLE: begin
				// sram busy
				if (~xbar_addr_r_ready_i & ~xbar_addr_w_ready_i) begin
					next_state = AXI_CTL_IDLE;
				// data write
				end else if(xbar_addr_w_ready_i & data_addr_w_valid_i & data_w_valid_i & xbar_w_ready_i) begin 
					next_state = AXI_CTL_BUSY_DATA;
				// data read
				end else if(xbar_addr_r_ready_i & data_addr_r_valid_i) begin
					next_state = AXI_CTL_BUSY_DATA;
				// inst read
				end else if(xbar_addr_r_ready_i & inst_addr_r_valid_i) begin
					next_state = AXI_CTL_BUSY_INST;
				end else begin 
					next_state = AXI_CTL_IDLE;
				end
			end
			AXI_CTL_BUSY_DATA: begin
				// finish write
				if (xbar_bkwd_valid_i & ~|xbar_bkwd_resp_i & data_bkwd_ready_i) begin
					next_state = AXI_CTL_IDLE;			
				// finish read
				end else if (xbar_r_valid_i & ~|xbar_r_resp_i & data_r_ready_i) begin 
					next_state = AXI_CTL_IDLE;
				end else begin 
					next_state = AXI_CTL_BUSY_DATA;
				end
			end
			AXI_CTL_BUSY_INST: begin				
				// finish inst read
				if (xbar_r_valid_i & ~|xbar_r_resp_i & inst_r_ready_i) begin 
					next_state = AXI_CTL_IDLE;
				end else begin 
					next_state = AXI_CTL_BUSY_INST;
				end
			end
			default:
				next_state = 2'b11;
		endcase
	end
	// NEXT: AXI-CTL-IDLE
	// finish data read or write
	assign {data_r_resp_o, data_r_valid_o, 
			data_bkwd_resp_o, data_bkwd_valid_o} 
			= (con_state == AXI_CTL_BUSY_DATA && next_state == AXI_CTL_IDLE) ? {xbar_r_resp_i, xbar_r_valid_i,
																				xbar_bkwd_resp_i, xbar_bkwd_valid_i} : 
																				0;
	// finish inst read
	assign {inst_r_resp_o, inst_r_valid_o} 
			= (con_state == AXI_CTL_BUSY_INST && next_state == AXI_CTL_IDLE) ? {xbar_r_resp_i, xbar_r_valid_i} : 
																				0;
	// finish data read or write OR finish inst read
	assign {xbar_r_ready_o, xbar_bkwd_ready_o} 
			= (con_state == AXI_CTL_BUSY_DATA && next_state == AXI_CTL_IDLE) ? {data_r_ready_i, data_bkwd_ready_i} : 
				(con_state == AXI_CTL_BUSY_INST && next_state == AXI_CTL_IDLE) ? {inst_r_ready_i, 1'b0} : 0;
	
	// NEXT: AXI_CTL_BUSY_DATA
	assign {data_addr_r_ready_o, data_addr_w_ready_o, data_w_ready_o} 
			= (next_state == AXI_CTL_BUSY_DATA) ? {xbar_addr_r_ready_i, xbar_addr_w_ready_i, xbar_w_ready_i} : 0;
	
	// NEXT: AXI_CTL_BUSY_INST
	assign {inst_addr_r_ready_o} 
			= (next_state == AXI_CTL_BUSY_INST) ? xbar_addr_r_ready_i : 0;

	// NEXT: SRAM AXI_CTL_BUSY_DATA or AXI_CTL_BUSY_INST
	assign {xbar_addr_r_valid_o,
			xbar_addr_w_valid_o,
			xbar_w_valid_o} 
			= (next_state == AXI_CTL_BUSY_DATA) ? { data_addr_r_valid_i,
													data_addr_w_valid_i, 
													data_w_valid_i} : 
				(next_state == AXI_CTL_BUSY_INST) ? {inst_addr_r_valid_i,
													1'b0,
													1'b0} : 
													0;

	always @(posedge clk) begin
		case(next_state)
			AXI_CTL_IDLE: begin
				if(con_state == AXI_CTL_BUSY_DATA) begin
					data_r_data_o <= xbar_r_data_i;		
				end else if(con_state == AXI_CTL_BUSY_INST) begin
					inst_r_data_o <= xbar_r_data_i;
				end
			end
			AXI_CTL_BUSY_INST: begin
				if(con_state == AXI_CTL_IDLE) begin
					xbar_addr_r_addr_o	<= inst_addr_r_addr_i;
					xbar_addr_w_addr_o	<= 0;
					xbar_w_data_o		<= 0;
					xbar_w_strb_o		<= 0;
				end
			end

			AXI_CTL_BUSY_DATA: begin
				if(con_state == AXI_CTL_IDLE) begin
					xbar_addr_r_addr_o	<= data_addr_r_addr_i;
					xbar_addr_w_addr_o	<= data_addr_w_addr_i;
					xbar_w_data_o		<= data_w_data_i;
					xbar_w_strb_o		<= data_w_strb_i;
				end
			end
			default: begin 
			end
		endcase
	end
endmodule
/* verilator lint_on UNOPTFLAT */
