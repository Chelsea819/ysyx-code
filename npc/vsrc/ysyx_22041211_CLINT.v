/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clock reset waddr wdata wen wmask
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"
module ysyx_22041211_CLINT #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
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
	input		                		r_ready_i	

);	
	// addr 
	parameter [1:0] WAIT_ADDR = 2'b00, WAIT_DATA_GET = 2'b01;
	reg				[1:0]			        con_state	;
	reg				[1:0]		        	next_state	;
	wire						        	mem_ren	;
	wire			[DATA_LEN - 1:0]	    r_data  ;
	
	reg 					[63:0] 				mtime;
	wire										bit_sel; // 0 低位； 1 高位
	
	assign addr_r_ready_o = (con_state == WAIT_ADDR) && rstn;
	assign r_resp_o = {2{~(con_state == WAIT_DATA_GET) | ~rstn}};
	assign mem_ren = (con_state == WAIT_DATA_GET) && rstn;

	assign bit_sel = ~(addr_r_addr_i - `DEVICE_CLINT_ADDR_L == 0);
	assign r_data = (bit_sel == 0) ?  mtime[DATA_LEN - 1:0] : mtime[63:DATA_LEN];



	always @(posedge clock ) begin
		if(rstn)
			mtime <= mtime + 64'b1;
		else
			mtime <= 0;
	end

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

	assign r_valid_o = (con_state == WAIT_DATA_GET) && rstn && (r_valid_delay == RANDOM_DELAY);

  // r addr delay
	always @(posedge clock ) begin
		if (next_state == WAIT_DATA_GET && (r_valid_delay != RANDOM_DELAY || r_valid_delay == 0))
			r_valid_delay <= r_valid_delay + 1;
		else if(next_state == WAIT_DATA_GET && r_valid_delay == RANDOM_DELAY)
			r_valid_delay <= r_valid_delay;
		else
			r_valid_delay <= 4'b0;
	end

// no delay
`else
	assign r_valid_o = (con_state == WAIT_DATA_GET) && rstn;
`endif	


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
			default:
				next_state = 2'b11;
		endcase
	end

	always @(*) begin
		if(~rstn) begin
			r_data_o = 0;
		end else if(con_state == WAIT_DATA_GET && next_state == WAIT_ADDR) begin
			r_data_o = r_data;
		end else 
			r_data_o = 0;
	end

	
endmodule
