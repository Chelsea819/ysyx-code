/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clock reset waddr wdata wen wmask
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"
module ysyx_22041211_AXI_SRAM #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input								rstn		,
    input		                		clock			,

	//Addr Read
	input		[ADDR_LEN - 1:0]		addr_r_addr_i,
	input		                		addr_r_valid_i,
	output		                		addr_r_ready_o,

	// Read data
	output	reg	[DATA_LEN - 1:0]		r_data_o	,
	output		[1:0]					r_resp_o	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	output		                		r_valid_o	,
	input		                		r_ready_i	,

	// Addr Write
	input		[ADDR_LEN - 1:0]		addr_w_addr_i,	// 写地址
	input		                		addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	output		                		addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	input		[DATA_LEN - 1:0]		w_data_i	,	// 写出的数据
	input		[3:0]					w_strb_i	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	input		                		w_valid_i	,	// 主设备给出的数据和字节选通信号有效
	output		                		w_ready_o	,	// 从设备已准备好接收数据选通信号

	// Backward
	output		[1:0]					bkwd_resp_o,	// 写回复信号，写操作是否成功
	output		                		bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	input		                		bkwd_ready_i	// 主设备已准备好接收写回复信号

);	
	parameter [1:0] WAIT_ADDR = 2'b00, WAIT_DATA_GET = 2'b01, WAIT_DATA_WRITE = 2'b10;
// delay test
`ifdef DELAY_TEST
	// random delay
	`ifdef RAN_DELAY
		reg				[3:0]		        	RANDOM_DELAY;
		wire			[3:0]		        	delay_num;

		ysyx_22041211_LFSR u_LFSR(
			.clock          ( clock          ),
			.rstn         ( rstn         ),
			.initial_var  ( 4'b1  		 ),
			.result       ( delay_num    )
		);
		
		always @(posedge clock ) begin
			if (~rstn) 
				RANDOM_DELAY <= 4'b1;
			else if((con_state == WAIT_ADDR && next_state == WAIT_DATA_GET) || (con_state == WAIT_ADDR && next_state == WAIT_DATA_WRITE))
				RANDOM_DELAY <= delay_num;
		end
	// fixed var delay
	`elsif VAR_DELAY
		// 当 RAN_DELAY 未定义，但 VAR_DELAY 被定义时，编译这段代码
		wire				[3:0]		        	RANDOM_DELAY;
		assign RANDOM_DELAY = `VAR_DELAY;
	`endif

	reg			[3:0]		r_valid_delay;
	reg			[3:0]		bkwd_valid_delay;

	assign r_valid_o = (con_state == WAIT_DATA_GET) && rstn && (r_valid_delay == RANDOM_DELAY);
	assign bkwd_valid_o = (con_state == WAIT_DATA_WRITE) && rstn && (bkwd_valid_delay == RANDOM_DELAY);

  // r addr delay
	always @(posedge clock ) begin
		if (next_state == WAIT_DATA_GET && (r_valid_delay != RANDOM_DELAY || r_valid_delay == 0))
			r_valid_delay <= r_valid_delay + 1;
		else if(next_state == WAIT_DATA_GET && r_valid_delay == RANDOM_DELAY)
			r_valid_delay <= r_valid_delay;
		else
			r_valid_delay <= 4'b0;
	end

	always @(posedge clock ) begin
		if (next_state == WAIT_DATA_WRITE && (bkwd_valid_delay != RANDOM_DELAY || bkwd_valid_delay == 0)) 
			bkwd_valid_delay <= bkwd_valid_delay + 1;
		else if(next_state == WAIT_DATA_WRITE && bkwd_valid_delay == RANDOM_DELAY)
			bkwd_valid_delay <= bkwd_valid_delay;
		else
			bkwd_valid_delay <= 4'b0;
	end
// no delay
`else
	assign r_valid_o = (con_state == WAIT_DATA_GET) && rstn;
	assign bkwd_valid_o = (con_state == WAIT_DATA_WRITE) && rstn;
`endif	

	reg				[1:0]			        con_state	;
	reg				[1:0]		        	next_state	;
	wire						        	mem_ren	;
	reg						        		mem_wen	;
	wire			[DATA_LEN - 1:0]	    r_data  ;
	

	assign addr_r_ready_o = (con_state == WAIT_ADDR) && rstn;
	assign r_resp_o = {2{~(con_state == WAIT_DATA_GET) | ~rstn}};
	assign addr_w_ready_o = (con_state == WAIT_ADDR) && rstn;
	assign w_ready_o = (con_state == WAIT_ADDR) && rstn;
	assign bkwd_resp_o = {2{~((con_state == WAIT_DATA_WRITE) & rstn)}};


	assign mem_ren = (con_state == WAIT_DATA_GET) && rstn;
	// assign mem_wen = (con_state == WAIT_DATA_WRITE) && rstn;

	// mem_wen
	always @(posedge clock ) begin
		if(rstn & (con_state == WAIT_ADDR && next_state == WAIT_DATA_WRITE))
			mem_wen <= 1'b1;
		else 
			mem_wen <= 1'b0;
	end

	// state trans
	always @(posedge clock ) begin
		if(rstn)
			con_state <= next_state;
		else 
			con_state <= WAIT_ADDR;
	end

	// next_state
	always @(*) begin
		case(con_state) 
			WAIT_ADDR: begin
				if (addr_r_ready_o & addr_r_valid_i) begin
					next_state = WAIT_DATA_GET;
				end else if(addr_w_valid_i & addr_w_ready_o & w_valid_i & w_ready_o) begin 
					next_state = WAIT_DATA_WRITE;
				end else begin 
					next_state = WAIT_ADDR;
				end
			end
			WAIT_DATA_GET: begin
				if (r_ready_i & r_valid_o & ~|r_resp_o) begin
					next_state = WAIT_ADDR;
				end else begin 
					next_state = WAIT_DATA_GET;
				end
			end
			WAIT_DATA_WRITE: begin
				if (bkwd_valid_o & ~|bkwd_resp_o & bkwd_ready_i) begin
					next_state = WAIT_ADDR;
				end else begin 
					next_state = WAIT_DATA_WRITE;
				end
			end
			default:
				next_state = 2'b11;
		endcase
	end


	ysyx_22041211_SRAM#(
		.ADDR_LEN     ( 32 ),
		.DATA_LEN     ( 32 )
	)u_ysyx_22041211_SRAM(
		.rstn          ( rstn         ),
		.clock          ( clock         ),
		.ren          ( mem_ren     ),
		.mem_wen_i    ( mem_wen		),
		.mem_wdata_i  ( w_data_i	),
		.mem_waddr_i  ( addr_w_addr_i),
		.mem_raddr_i  ( addr_r_addr_i),
		.mem_wmask    ( {4'b0, w_strb_i}),
		.mem_rdata_usigned_o (  r_data	)
	);

	always @(*) begin
		if(~rstn) begin
			r_data_o = 0;
		end else if(con_state == WAIT_DATA_GET && next_state == WAIT_ADDR) begin
			r_data_o = r_data;
		end else 
			r_data_o = 0;
	end

	

endmodule
