/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/

module ysyx_22041211_IFU #(parameter ADDR_WIDTH = 32, DATA_WIDTH = 32)(
	input									clk				,
	input									rst				,

    // hand signal
	input									ready			,
	input									last_finish		,
    output		reg							valid	        ,

    // refresh pc
	input									branch_request_i,	
	input		[ADDR_WIDTH - 1:0]			branch_target_i	,
	input									branch_flag_i	,
	input                                   jmp_flag_i      ,
    input       [31:0]                   	jmp_target_i    ,
	input									csr_jmp_i	    ,
	input		[ADDR_WIDTH - 1:0]			csr_pc_i	    ,

    // get instruction
	// output	[ADDR_LEN - 1:0]			ce		,
    // input		[DATA_WIDTH - 1:0]			inst_i	,
    output reg	[DATA_WIDTH - 1:0]			inst_o	,
	output reg	[ADDR_WIDTH - 1:0]			pc
);
	wire		[ADDR_WIDTH - 1:0]	        pc_plus_4	;
	reg							        	con_state	;
	reg							        	next_state	;
	reg			[DATA_WIDTH - 1:0]      	inst	;

	parameter IFU_IDLE = 0, IFU_WAIT_READY = 1;

	// state trans
	always @(posedge clk ) begin
		if(rst)
			con_state <= IFU_IDLE;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
			IFU_IDLE: begin
				if (valid == 1'b0) begin
					next_state = IFU_IDLE;
				end else begin 
					next_state = IFU_WAIT_READY;
				end
			end
			IFU_WAIT_READY: begin 
				if (ready == 1'b0 || last_finish == 1'b0) begin
					next_state = IFU_WAIT_READY;
				end else begin 
					next_state = IFU_IDLE;
				end
			end
		endcase
	end

	always @(posedge clk) begin
        if(con_state == IFU_WAIT_READY && next_state == IFU_IDLE) begin
            inst_o         <=     inst;
        end
	end

    // get new pc
    ysyx_22041211_counter#(
        .ADDR_LEN         ( 32 )
    )u_ysyx_22041211_counter(
        .clk              ( clk              ),
        .rst              ( rst              ),
        .branch_request_i ( branch_request_i ),
        .branch_target_i  ( branch_target_i  ),
        .branch_flag_i    ( branch_flag_i    ),
        .pc_plus_4        ( pc_plus_4        ),
        .jmp_flag_i       ( jmp_flag_i       ),
        .jmp_target_i     ( jmp_target_i     ),
        .csr_jmp_i        ( csr_jmp_i        ),
        .csr_pc_i         ( csr_pc_i         ),
		.con_state        ( con_state        ),
        .ready         	  ( ready         ),
        .pc               ( pc               )
    );
	import "DPI-C" context function int pmem_read_task(input int raddr, input byte wmask);

	ysyx_22041211_pcPlus#(
		.DATA_LEN ( 32 )
	)u_ysyx_22041211_pcPlus(
		.pc_old ( pc ),
		.rst    ( rst    ),
		.pc_new ( pc_plus_4  )
	);

    //fetch instruction
	always @(*) begin
		if(~rst) begin
			$display("rst = %d, pc = %x con_state = %d ",rst, pc, con_state);
        	inst = pmem_read_task(pc, 8'b00001111);
			valid = 1'b1;
		end else begin  
			inst = 32'b0;
			valid = 1'b0;
		end
	end

endmodule
