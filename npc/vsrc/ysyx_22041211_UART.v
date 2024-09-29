/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clock reset waddr wdata wen wmask
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"
module ysyx_22041211_UART #(parameter DATA_LEN = 32)(
	input								rstn		,
    input		                		clock			,

	// Addr Write
	input		                		addr_w_valid_i,	// 主设备给出的地址和相关控制信号有效
	output		                		addr_w_ready_o, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	input		[DATA_LEN - 1:0]		w_data_i	,	// 写出的数据
	input		                		w_valid_i	,	// 主设备给出的数据和字节选通信号有效
	output		                		w_ready_o	,	// 从设备已准备好接收数据选通信号

	// Backward
	output		[1:0]					bkwd_resp_o,	// 写回复信号，写操作是否成功
	output		                		bkwd_valid_o,	// 从设备给出的写回复信号是否有效
	input		                		bkwd_ready_i	// 主设备已准备好接收写回复信号

);	
	parameter  WAIT_ADDR = 1'b0, WAIT_DATA_WRITE = 1'b1;
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
			else if((con_state == WAIT_ADDR && next_state == WAIT_DATA_WRITE))
				RANDOM_DELAY <= delay_num;
		end
	// fixed var delay
	`elsif VAR_DELAY
		// 当 RAN_DELAY 未定义，但 VAR_DELAY 被定义时，编译这段代码
		wire				[3:0]		        	RANDOM_DELAY;
		assign RANDOM_DELAY = `VAR_DELAY;
	`endif
	reg			[3:0]		bkwd_valid_delay;
	assign bkwd_valid_o = (con_state == WAIT_DATA_WRITE) && rstn && (bkwd_valid_delay == RANDOM_DELAY);


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
	assign bkwd_valid_o = (con_state == WAIT_DATA_WRITE) && rstn;
`endif	

	reg							        con_state	;
	reg						        	next_state	;
	reg					        		mem_wen	;
	

	assign addr_w_ready_o = (con_state == WAIT_ADDR) && rstn;
	assign w_ready_o = (con_state == WAIT_ADDR) && rstn;
	assign bkwd_resp_o = {2{~((con_state == WAIT_DATA_WRITE) & rstn)}};


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
				if(addr_w_valid_i & addr_w_ready_o & w_valid_i & w_ready_o) begin 
					next_state = WAIT_DATA_WRITE;
				end else begin 
					next_state = WAIT_ADDR;
				end
			end
			WAIT_DATA_WRITE: begin
				if (bkwd_valid_o & ~|bkwd_resp_o & bkwd_ready_i) begin
					next_state = WAIT_ADDR;
				end else begin 
					next_state = WAIT_DATA_WRITE;
				end
			end
		endcase
	end


	always @(posedge clock) begin
  		if (mem_wen) begin // 有写请求时
			$write("%c",w_data_i[7:0]);
		end
	end


endmodule
