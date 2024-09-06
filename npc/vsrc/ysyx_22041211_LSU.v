/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clk rst waddr wdata wen wmask
`include "./ysyx_22041211_define.v"
module ysyx_22041211_LSU #(parameter DATA_LEN = 32)(
	input								rst			,
    input		                		wd_i		,
    input		                		clk			,
    input		[4:0]		            wreg_i		,
    input		[DATA_LEN - 1:0]		alu_result_i,
    input		                		mem_wen_i	,
	input		[DATA_LEN - 1:0]		mem_wdata_i	,
    input       [2:0]                   load_type_i , 
    input       [1:0]                   store_type_i , 
    input       [DATA_LEN - 1:0]        csr_wdata_i	,
    input                               exu_valid   , 
    input                               wb_ready_o  ,
    output                              lsu_ready_o ,
    output  reg                         lsu_valid_o ,
    output	reg	                		wd_o		,
    output	reg	[4:0]		            wreg_o		,
    output  reg [DATA_LEN - 1:0]        csr_wdata_o	,
    output	reg	[DATA_LEN - 1:0]		wdata_o
);	
	wire [31:0] mem_waddr;
    wire [31:0] mem_raddr;
    wire [31:0] mem_rdata;
    reg  [31:0] mem_rdata_rare;
    reg  [7:0]  mem_rmask;
    wire [7:0]  mem_wmask;
    reg        mem_to_reg;
    reg        mem_wen;

    // 写寄存器的信息
    wire		[DATA_LEN - 1:0]		    wdata       ;
    assign wdata = (mem_to_reg == 1'b1) ? mem_rdata : alu_result_i;

    
    assign lsu_ready_o = 1'b1;

    reg			[1:0]			        	con_state	;
	reg			[1:0]			        	next_state	;
    parameter [1:0] LSU_WAIT_EXU_VALID = 2'b00, LSU_WAIT_LSU_VALID = 2'b01, LSU_WAIT_WB_READY = 2'b10;

	always @(posedge clk ) begin
		if(next_state == LSU_WAIT_LSU_VALID)
			lsu_valid_o <= 1'b1;
		else 
			lsu_valid_o <= 1'b0;
	end

	// state trans
	always @(posedge clk ) begin
		if(rst)
			con_state <= LSU_WAIT_EXU_VALID;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
            // 等待ifu取指，下一个时钟周期开始译码
			LSU_WAIT_EXU_VALID: begin
				if (exu_valid == 1'b0) begin
					next_state = LSU_WAIT_EXU_VALID;
				end else begin 
					next_state = LSU_WAIT_LSU_VALID;
				end
			end
            // 等待idu完成译码
			LSU_WAIT_LSU_VALID: begin 
				if (lsu_valid_o == 1'b0) begin
					next_state = LSU_WAIT_LSU_VALID;
				end else begin 
					next_state = LSU_WAIT_WB_READY;
				end
			end
            // 等待exu空闲，下个时钟周期传递信息
            LSU_WAIT_WB_READY: begin 
				if (wb_ready_o == 1'b0) begin
					next_state = LSU_WAIT_WB_READY;
				end else begin 
					next_state = LSU_WAIT_EXU_VALID;
				end
			end
            default: begin 
				next_state = 2'b11;
			end
		endcase
	end

    always @(*) begin
        if(con_state == LSU_WAIT_LSU_VALID) begin				
            mem_to_reg = |load_type_i;
            mem_wen = mem_wen_i;
        end else begin
            mem_to_reg =    0;
            mem_wen = 0;
        end
	end

    ysyx_22041211_data_SRAM#(
        .ADDR_LEN     ( 32 ),
        .DATA_LEN     ( 32 )
    )u_ysyx_22041211_data_SRAM(
        .rst          ( rst          ),
        .clk          ( clk          ),
        .ren          ( mem_to_reg   ),
        .mem_wen_i    ( mem_wen_i    ),
        .mem_wdata_i  ( mem_wdata_i  ),
        .mem_waddr_i  ( mem_waddr    ),
        .mem_raddr_i  ( mem_raddr  ),
        .mem_wmask    ( mem_wmask    ),
        .mem_rmask    ( mem_rmask    ),
        .mem_rdata_usigned_o( mem_rdata_rare)
    );
    assign mem_waddr = alu_result_i;
    assign mem_raddr = alu_result_i;
    // store
	assign mem_wmask = (store_type_i == `STORE_SB_8)  ? `MEM_MASK_8 : 
                       (store_type_i == `STORE_SH_16) ? `MEM_MASK_16 :
                       (store_type_i == `STORE_SW_32) ? `MEM_MASK_32 : 
                       0;
    // load
    assign mem_rmask = (load_type_i == `LOAD_LB_8 || load_type_i == `LOAD_LBU_8)   ? `MEM_MASK_8 : 
                       (load_type_i == `LOAD_LH_16 || load_type_i == `LOAD_LHU_16) ? `MEM_MASK_16 :
                       (load_type_i == `LOAD_LW_32)                                ? `MEM_MASK_32 : 
                       0;
    assign mem_rdata = (load_type_i == `LOAD_LB_8)  ? {{24{mem_rdata_rare[7]}}, mem_rdata_rare[7:0]} : 
                       (load_type_i == `LOAD_LH_16) ? {{16{mem_rdata_rare[15]}}, mem_rdata_rare[15:0]}: 
                       mem_rdata_rare;

    always @(posedge clk) begin
        if(con_state == LSU_WAIT_LSU_VALID && next_state == LSU_WAIT_WB_READY) begin				
            wd_o	         <=     wd_i; 
            wreg_o	         <=     wreg_i;  	
            csr_wdata_o	     <=     csr_wdata_i;  
            wdata_o          <=     wdata; 
        end
	end
endmodule
