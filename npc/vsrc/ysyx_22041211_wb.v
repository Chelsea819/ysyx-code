`include "./ysyx_22041211_define.v"
module ysyx_22041211_wb #(parameter DATA_LEN = 32)(
    input								rst,
    input		                		wd_i		,
    input		                		clk		    ,
    input		[4:0]		            wreg_i		,
    input       [DATA_LEN - 1:0]        csr_wdata_i	,
    input       [DATA_LEN - 1:0]        reg_wdata_i	,
    input                               lsu_valid   ,
    output                              wb_ready_o  ,
    output  reg                         finish      ,
    output	reg	                		wd_o		,
    output	reg	[4:0]		            wreg_o		,
    output  reg [DATA_LEN - 1:0]        csr_wdata_o	,
    output	reg	[DATA_LEN - 1:0]		wdata_o
);
    // assign wb_ready_o = 1'b1;

    reg							        	con_state	;
	reg							        	next_state	;
    
	parameter WB_BUSY = 1, WB_WAIT_VALID = 0;

    always @(posedge clk ) begin
        if(con_state == WB_BUSY)
			finish <= 1'b1;
		else 
			finish <= 1'b0;
	end

	// state trans
	always @(posedge clk ) begin
		if(rst)
			con_state <= WB_WAIT_VALID;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
			WB_WAIT_VALID: begin
				if (lsu_valid == 1'b0) begin
					next_state = WB_WAIT_VALID;
				end else begin 
					next_state = WB_BUSY;
				end
			end
			WB_BUSY: begin 
				// if (wb_ready_o == 1'b0) begin
				// 	next_state = WB_BUSY;
				// end else begin 
					next_state = WB_WAIT_VALID;
				// end
			end
		endcase
	end

    always @(*) begin
        if(con_state == WB_BUSY) begin
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
