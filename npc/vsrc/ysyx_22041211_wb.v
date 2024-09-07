`include "./ysyx_22041211_define.v"
module ysyx_22041211_wb #(parameter DATA_LEN = 32)(
    input								rst,
    input		                		wd_i		,
    input		                		clk		    ,
    input		[4:0]		            wreg_i		,
    input       [DATA_LEN - 1:0]        csr_wdata_i	,
    input       [DATA_LEN - 1:0]        reg_wdata_i	,
    input                               ifu_valid   ,
    input                               lsu_valid   ,
    input                               memory_inst_i ,
    // output                              wb_ready_o  ,
    output  reg                         finish      ,
    output	reg	                		wd_o		,
    output	reg	[4:0]		            wreg_o		,
    output  reg [DATA_LEN - 1:0]        csr_wdata_o	,
    output	reg	[DATA_LEN - 1:0]		wdata_o
);
    // assign wb_ready_o = 1'b1;

    reg		[1:0]					        	con_state	;
	reg		[1:0]					        	next_state	;
    
	parameter [1:0] WB_WAIT_IFU_VALID = 0, WB_WAIT_CTRL_VALID = 2'b01, WB_WAIT_MEM = 2'b10, WB_WAIT_REG_VALID = 2'b11;

    always @(posedge clk ) begin
        if(next_state == WB_WAIT_REG_VALID)
			finish <= 1'b1;
		else 
			finish <= 1'b0;
	end

	// state trans
	always @(posedge clk ) begin
		if(rst)
			con_state <= WB_WAIT_IFU_VALID;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
			WB_WAIT_IFU_VALID: begin
				if (ifu_valid == 1'b0) begin
					next_state = WB_WAIT_IFU_VALID;
				end else begin 
					next_state = WB_WAIT_CTRL_VALID;
				end
			end
			WB_WAIT_CTRL_VALID: begin 
				if (memory_inst_i == 1'b1) begin
					next_state = WB_WAIT_MEM;
				end else begin 
					next_state = WB_WAIT_REG_VALID;
				end
			end
			WB_WAIT_MEM: begin
				if (lsu_valid == 1'b0) begin
					next_state = WB_WAIT_MEM;
				end else begin 
					next_state = WB_WAIT_REG_VALID;
				end
			end
			WB_WAIT_REG_VALID: begin 
					next_state = WB_WAIT_IFU_VALID;
			end
		endcase
	end

	// always @(*) begin
	// 	$display("ifu_valid: [%d] lsu_valid: [%d]  con_state: [%d] next_state: [%d]",ifu_valid, lsu_valid, con_state, next_state);
	// end

    always @(*) begin
        if(next_state == WB_WAIT_REG_VALID) begin
            wd_o	         =     wd_i; 
            wreg_o	         =     wreg_i;  	
            csr_wdata_o	     =     csr_wdata_i;  
            wdata_o          =     reg_wdata_i;  
        end else begin 
            wd_o	         =     0; 
            wreg_o	         =     0;  	
            csr_wdata_o	     =     0;  
            wdata_o          =     0; 
        end
	end
    
endmodule
