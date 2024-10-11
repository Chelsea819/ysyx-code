/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clock rstn waddr wdata wen wmask
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"
module ysyx_22041211_LSU #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								rstn			,
    input		                		wd_i		,
    input		                		clock			,
    input		[4:0]		            wreg_i		,
    input		[DATA_LEN - 1:0]		alu_result_i,
    input		                		mem_wen_i	,
	input		[DATA_LEN - 1:0]		mem_wdata_i	,
    input       [2:0]                   load_type_i , 
    input       [1:0]                   store_type_i, 
    input       [DATA_LEN - 1:0]        csr_wdata_i	,
    input       [2:0]                   csr_type_i	,
    input                               ifu_valid   , 
    // input                               wb_ready_o  ,
    // output                              lsu_ready_o ,
    output                              memory_inst_o ,
    output                              lsu_valid_o ,
    output	   	                		wd_o		,
    output	   	[4:0]		            wreg_o		,
    output      [DATA_LEN - 1:0]        csr_wdata_o	,
    output      [2:0]                   csr_type_o	,

    input	        [DATA_LEN - 1:0]    mem_rdata_rare_i	,
    // output		                		mem_ren_o	,
    // output	reg                		    mem_wen_o	,

    // AXI
    //Addr Read
	output	reg	[ADDR_LEN - 1:0]		addr_r_addr_o,
	output		                		addr_r_valid_o,
	input		                		addr_r_ready_i,
	output		[2:0]                	addr_r_size_o,

	// Read data
	input		[DATA_LEN - 1:0]		r_data_i	,
	input		[1:0]					r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		r_valid_i	,
	output		                		r_ready_o	,

	// Addr Write
	output	reg	[ADDR_LEN - 1:0]		addr_w_addr_o,	// 写地址
	output		                		addr_w_valid_o,	// 主设备给出的地址和相关控制信号有效
	output		[2:0]                	addr_w_size_o,	// 主设备给出的地址和相关控制信号有效
	input		                		addr_w_ready_i, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	output	reg	[DATA_LEN - 1:0]		w_data_o	,	// 写出的数据
	output		[3:0]					w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					bkwd_resp_i,	// 写回复信号，写操作是否成功
	input		                		bkwd_valid_i,	// 从设备给出的写回复信号是否有效
	output		                		bkwd_ready_o,	// 主设备已准备好接收写回复信号
    
    output     [DATA_LEN - 1:0]		    wdata_o
);	
    wire [31:0] mem_rdata;
    // reg  [7:0]  mem_rmask;
    reg        mem_to_reg;
	reg	[3:0]					w_strb	;	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit

// delay test
`ifdef DELAY_TEST
	// random delay
	`ifdef RAN_DELAY
		reg				[3:0]		        	RANDOM_DELAY;
        reg				[3:0]		        	RANDOM_W_DATA_DELAY;
        wire			[3:0]		        	delay_num;
        wire			[3:0]		        	delay_num_w_data;

        ysyx_22041211_LFSR u_LFSR(
            .clock          ( clock          ),
            .rstn         ( rstn         ),
            .initial_var  ( 4'b1  		 ),
            .result       ( delay_num    )
        );
        ysyx_22041211_LFSR u_LFSR_w_data(
            .clock          ( clock          ),
            .rstn         ( rstn         ),
            .initial_var  ( 4'b1  		 ),
            .result       ( delay_num_w_data    )
        );
		
		always @(posedge clock ) begin
            if (~rstn) 
                RANDOM_DELAY <= 4'b1;
            else if((con_state == LSU_WAIT_IFU_VALID && next_state == LSU_WAIT_ADDR_PASS) || (con_state == LSU_WAIT_ADDR_PASS && next_state == LSU_WAIT_LSU_VALID))
                RANDOM_DELAY <= delay_num;
        end

        always @(posedge clock ) begin
            if (~rstn) 
                RANDOM_W_DATA_DELAY <= 0;
            else if((con_state == LSU_WAIT_IFU_VALID && next_state == LSU_WAIT_ADDR_PASS))
                RANDOM_W_DATA_DELAY <= delay_num_w_data;
        end
	// fixed var delay
	`elsif VAR_DELAY
		// 当 RAN_DELAY 未定义，但 VAR_DELAY 被定义时，编译这段代码
        wire				[3:0]		        	RANDOM_DELAY;
        wire				[3:0]		        	RANDOM_W_DATA_DELAY;
		assign RANDOM_DELAY = `VAR_DELAY;
		assign RANDOM_W_DATA_DELAY = `VAR_W_DELAY;

	`endif

	reg			[3:0]		addr_r_valid_delay;
	reg			[3:0]		addr_w_valid_delay;
	reg			[3:0]		w_data_valid_delay;
	reg			[3:0]		r_ready_delay;
	reg			[3:0]		bkwd_ready_delay;

    assign addr_r_valid_o = (con_state == LSU_WAIT_ADDR_PASS) & mem_to_reg & rstn & (addr_r_valid_delay == RANDOM_DELAY); // addr valid and load inst
    assign r_ready_o = (con_state == LSU_WAIT_LSU_VALID) & (r_ready_delay == RANDOM_DELAY);
    assign addr_w_valid_o = (con_state == LSU_WAIT_ADDR_PASS) & mem_wen_i & rstn & (addr_w_valid_delay == RANDOM_DELAY);  // addr valid and store inst
    assign addr_w_size_o = `AXI_ADDR_SIZE_4;  // addr valid and store inst
	assign addr_r_size_o = `AXI_ADDR_SIZE_4;
    assign w_valid_o = (con_state == LSU_WAIT_ADDR_PASS) & mem_wen_i & rstn & (w_data_valid_delay == RANDOM_W_DATA_DELAY);
    assign bkwd_ready_o = (con_state == LSU_WAIT_LSU_VALID) & rstn & (bkwd_ready_delay == RANDOM_DELAY);

	// r addr delay
	always @(posedge clock ) begin
        if (next_state == LSU_WAIT_ADDR_PASS && (addr_r_valid_delay != RANDOM_DELAY || addr_r_valid_delay == 0))
			addr_r_valid_delay <= addr_r_valid_delay + 1;
		else if(next_state == LSU_WAIT_ADDR_PASS && addr_r_valid_delay == RANDOM_DELAY)
			addr_r_valid_delay <= addr_r_valid_delay;
		else 
			addr_r_valid_delay <= 4'b0;
	end
	always @(posedge clock ) begin
		if (next_state == LSU_WAIT_ADDR_PASS && (addr_w_valid_delay != RANDOM_DELAY || addr_w_valid_delay == 0))
			addr_w_valid_delay <= addr_w_valid_delay + 1;
        else if(next_state == LSU_WAIT_ADDR_PASS && addr_w_valid_delay == RANDOM_DELAY)
			addr_w_valid_delay <= addr_w_valid_delay;
		else  
			addr_w_valid_delay <= 4'b0;
	end
	always @(posedge clock ) begin
		if (next_state == LSU_WAIT_ADDR_PASS && (w_data_valid_delay != RANDOM_W_DATA_DELAY || w_data_valid_delay == 0))
			w_data_valid_delay <= w_data_valid_delay + 1;
        else if(next_state == LSU_WAIT_ADDR_PASS && w_data_valid_delay == RANDOM_W_DATA_DELAY)
			w_data_valid_delay <= w_data_valid_delay;
		else  
			w_data_valid_delay <= 4'b0;
	end

	always @(posedge clock ) begin
		if (next_state == LSU_WAIT_LSU_VALID && (r_ready_delay != RANDOM_DELAY || r_ready_delay == 0)) 
			r_ready_delay <= r_ready_delay + 1;
		else if(next_state == LSU_WAIT_LSU_VALID && r_ready_delay == RANDOM_DELAY)
			r_ready_delay <= r_ready_delay;
		else  
			r_ready_delay <= 4'b0;
	end
	always @(posedge clock ) begin
		if (next_state == LSU_WAIT_LSU_VALID && (bkwd_ready_delay != RANDOM_DELAY || bkwd_ready_delay == 0)) 
			bkwd_ready_delay <= bkwd_ready_delay + 1;
		else if(next_state == LSU_WAIT_LSU_VALID && bkwd_ready_delay == RANDOM_DELAY)
			bkwd_ready_delay <= bkwd_ready_delay;
		else 
			bkwd_ready_delay <= 4'b0;
	end
// no delay
`else
    assign addr_r_valid_o = (con_state == LSU_WAIT_ADDR_PASS) & mem_to_reg & rstn; // addr valid and load inst
    assign r_ready_o = (con_state == LSU_WAIT_LSU_VALID);
    assign addr_w_valid_o = (con_state == LSU_WAIT_ADDR_PASS) & mem_wen_i & rstn;  // addr valid and store inst
    assign w_valid_o = (con_state == LSU_WAIT_ADDR_PASS) & mem_wen_i & rstn;
    assign bkwd_ready_o = (con_state == LSU_WAIT_LSU_VALID) & rstn;

`endif

    // 写寄存器的信息
    wire		[DATA_LEN - 1:0]		    wdata       ;
    assign wdata = (mem_to_reg == 1'b1) ? mem_rdata : alu_result_i;
    assign memory_inst_o = mem_to_reg | mem_wen_i;  // load or store
    always @(*) begin
		if(~rstn) begin
			addr_r_addr_o = 0;
            addr_w_addr_o = 0;
            w_data_o = 0;
            w_strb = 0;
        // end else if(con_state == LSU_WAIT_ADDR_PASS && next_state == LSU_WAIT_LSU_VALID) begin
        end else if(con_state == LSU_WAIT_ADDR_PASS) begin
            addr_r_addr_o = alu_result_i;
            addr_w_addr_o = alu_result_i;
            w_data_o = mem_wdata_i;
            w_strb = (store_type_i == `STORE_SB_8)? 4'b1 :
                    (store_type_i == `STORE_SH_16) ? 4'b10 :
                    (store_type_i == `STORE_SW_32) ? 4'b100 : 
                    0;
		end else begin 
			addr_r_addr_o = 0;
            addr_w_addr_o = 0;
            w_data_o = 0;
            w_strb = 0;
		end
	end

	assign w_strb_o = (addr_w_addr_o[1:0] == 2'b00 ) ? w_strb :
					(addr_w_addr_o[1:0] == 2'b01 ) ? {w_strb[2:0], 1'b0} :
					(addr_w_addr_o[1:0] == 2'b10 ) ? {w_strb[1:0], 2'b0} :
					(addr_w_addr_o[1:0] == 2'b11 ) ? {w_strb[0], 3'b0} : 0;

    assign wdata_o = wdata;


    reg			[1:0]			        	con_state	;
	reg			[1:0]			        	next_state	;
    parameter [1:0] LSU_WAIT_IFU_VALID = 2'b00, LSU_WAIT_LSU_VALID = 2'b01, LSU_WAIT_WB_READY = 2'b10, LSU_WAIT_ADDR_PASS = 2'b11;

	// always @(*) begin
	// 	if(next_state == LSU_WAIT_LSU_VALID || next_state == LSU_WAIT_WB_READY)
	// 		lsu_valid_o = 1'b1;
	// 	else 
	// 		lsu_valid_o = 1'b0;
	// end

    assign lsu_valid_o = (con_state == LSU_WAIT_WB_READY);

	// state trans
	always @(posedge clock ) begin
		if(rstn)
            con_state <= next_state;
		else 
			con_state <= LSU_WAIT_IFU_VALID;
	end

	// next_state
	always @(*) begin
		case(con_state) 
            // 等待ifu取指，下一个时钟周期开始译码
			LSU_WAIT_IFU_VALID: begin
				if (ifu_valid == 1'b0) begin
					next_state = LSU_WAIT_IFU_VALID;
				end else begin 
					next_state = LSU_WAIT_ADDR_PASS;
				end
			end
            // 等待addr wdata握手
			LSU_WAIT_ADDR_PASS: begin 
                if(~(mem_to_reg | mem_wen_i)) begin 
                    next_state = LSU_WAIT_IFU_VALID; 
                end else if (addr_r_valid_o & addr_r_ready_i) begin // read
					next_state = LSU_WAIT_LSU_VALID;
                end else if (addr_w_valid_o & addr_w_ready_i & w_ready_i & w_valid_o) begin // read
					next_state = LSU_WAIT_LSU_VALID;
				end else begin 
					next_state = LSU_WAIT_ADDR_PASS;
				end
			end
            // 等待idu完成译码
			LSU_WAIT_LSU_VALID: begin 
				if ((r_valid_i & r_ready_o) | (~|bkwd_resp_i & bkwd_valid_i & bkwd_ready_o)) begin
					next_state = LSU_WAIT_WB_READY;
				end else begin 
					next_state = LSU_WAIT_LSU_VALID;
				end
			end
            // 等待exu空闲，下个时钟周期传递信息
            LSU_WAIT_WB_READY: begin 
				// if (wb_ready_o == 1'b0) begin
				// 	next_state = LSU_WAIT_WB_READY;
				// end else begin 
				next_state = LSU_WAIT_IFU_VALID;
				// end
			end
            default: begin 
				next_state = 2'b11;
			end
		endcase
	end

    // always @(*) begin
	// 	$display("load_type_i = [%b] mem_to_reg: [%d] rmask: [%d] mem_raddr: [%x] con_state: [%d] next_state: [%d]",load_type_i, mem_to_reg,mem_rmask, mem_raddr, con_state, next_state);
	// end

    // always @(posedge clock) begin
    //     if(next_state == LSU_WAIT_WB_READY) begin				
    //         mem_wen_o <= mem_wen_i;
    //     end else begin
    //         mem_wen_o <= 0;
    //     end
	// end

    always @(*) begin
        if(con_state != LSU_WAIT_IFU_VALID) begin				
            mem_to_reg = |load_type_i;
        end else begin
            mem_to_reg = 0;
        end
	end

    // assign mem_ren_o = mem_to_reg;
    
    // load
    assign mem_rdata = (load_type_i == `LOAD_LB_8)  ? {{24{mem_rdata_rare_i[7]}}, mem_rdata_rare_i[7:0]} : 
                    (load_type_i == `LOAD_LH_16) ? {{16{mem_rdata_rare_i[15]}}, mem_rdata_rare_i[15:0]}: 
                    (load_type_i == `LOAD_LBU_8) ? {{24{1'b0}}, mem_rdata_rare_i[7:0]}: 
                    (load_type_i == `LOAD_LHU_16) ? {{16{1'b0}}, mem_rdata_rare_i[15:0]}: 
                    mem_rdata_rare_i;

    assign wd_o = wd_i;
    assign wreg_o = wreg_i;
    assign csr_wdata_o	     =     csr_wdata_i;  
    assign csr_type_o	     =     csr_type_i;  

endmodule
