/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/
`include "ysyx_22041211_define.v"
`include "ysyx_22041211_define_delay.v"

module ysyx_22041211_IFU #(parameter ADDR_WIDTH = 32, DATA_WIDTH = 32)(
	input									clock				,
	input									reset				,

    // hand signal
	// input									ready			,
	input									last_finish		,
	output	reg								valid	        ,

    // refresh pc
	input									branch_request_i,	
	input		[ADDR_WIDTH - 1:0]			branch_target_i	,
	input									branch_flag_i	,
	input                                   jmp_flag_i      ,
    input       [31:0]                   	jmp_target_i    ,
	input									csr_jmp_i	    ,
	input		[ADDR_WIDTH - 1:0]			csr_pc_i	    ,
	// input 		[DATA_WIDTH - 1:0]			inst_i		,

    // get instruction
	// output									ce		,
    input		[DATA_WIDTH - 1:0]			inst_i	,
    // output reg								inst_invalid_o	,
    output reg	[DATA_WIDTH - 1:0]			id_inst_i	,
	output reg	[ADDR_WIDTH - 1:0]			pc			,

	// IFU-AXI
	// Addr Read
	output	reg	[ADDR_WIDTH - 1:0]		addr_r_addr_o,
	output		                		addr_r_valid_o,
	input		                		addr_r_ready_i,

	// Read data
	// input		[DATA_WIDTH - 1:0]		r_data_i	,
	input		[1:0]					r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		r_valid_i	,
	output		                		r_ready_o	
);
	wire		[ADDR_WIDTH - 1:0]	        pc_plus_4	;
	reg			[1:0]				        con_state	;
	reg			[1:0]			        	next_state	;
	wire									r_inst_en	;
	wire									inst_invalid;

	// delay test
`ifdef DELAY_TEST
	// random delay
	`ifdef RAN_DELAY
		reg				[3:0]		        	RANDOM_DELAY;
		wire			[3:0]		        	delay_num;

		ysyx_22041211_LFSR u_LFSR(
			.clock          ( clock          ),
			.rstn         ( ~reset         ),
			.initial_var  ( 4'b1  		 ),
			.result       ( delay_num    )
		);
		
		always @(posedge clock ) begin
			if (reset) 
				RANDOM_DELAY <= 4'b1;
			else if((con_state == IFU_WAIT_FINISH && next_state == IFU_WAIT_ADDR_PASS) || (con_state == IFU_WAIT_ADDR_PASS && next_state == IFU_WAIT_INST_LOAD))
				RANDOM_DELAY <= delay_num;
		end

	// fixed var delay
	`elsif VAR_DELAY
		// 当 RAN_DELAY 未定义，但 VAR_DELAY 被定义时，编译这段代码
		wire				[3:0]		        	RANDOM_DELAY;
		assign RANDOM_DELAY = `VAR_DELAY;
	`endif

	reg			[3:0]		addr_r_valid_delay;
	reg			[3:0]		r_ready_delay;

	// assign addr_r_valid_o = (con_state == IFU_WAIT_ADDR_PASS) & ~reset & (addr_r_valid_delay == RANDOM_DELAY);
	assign addr_r_valid_o = (con_state == IFU_WAIT_ADDR_PASS) & ~reset;
	assign r_ready_o = (con_state == IFU_WAIT_INST_LOAD) & (r_ready_delay == RANDOM_DELAY);

	// r addr delay
	always @(posedge clock ) begin
		if (next_state == IFU_WAIT_ADDR_PASS && (addr_r_valid_delay != RANDOM_DELAY || addr_r_valid_delay == 0))
			addr_r_valid_delay <= addr_r_valid_delay + 1;
		else if(next_state == IFU_WAIT_ADDR_PASS && addr_r_valid_delay == RANDOM_DELAY)
			addr_r_valid_delay <= addr_r_valid_delay;
		else 
			addr_r_valid_delay <= 4'b0;
	end

	always @(posedge clock ) begin
		if (next_state == IFU_WAIT_INST_LOAD && ((r_ready_delay != RANDOM_DELAY || r_ready_delay == 0)))
			r_ready_delay <= r_ready_delay + 1;
		else if(next_state == IFU_WAIT_INST_LOAD && r_ready_delay == RANDOM_DELAY)
			r_ready_delay <= r_ready_delay;
		else  
			r_ready_delay <= 4'b0;
	end
// no delay
`else
	assign addr_r_valid_o = (con_state == IFU_WAIT_ADDR_PASS) & ~reset;
	assign r_ready_o = (con_state == IFU_WAIT_INST_LOAD);
`endif
	// assign ce = r_inst_en;
	assign r_inst_en = (con_state == IFU_WAIT_READY && next_state == IFU_WAIT_FINISH);
	assign valid = (con_state == IFU_WAIT_INST_LOAD && next_state == IFU_WAIT_READY);

	assign inst_invalid = ~((inst_i[6:0] == `TYPE_U_LUI_OPCODE) | (inst_i[6:0] == `TYPE_U_AUIPC_OPCODE) | //U-auipc lui
					 (inst_i[6:0] == `TYPE_J_JAL_OPCODE) | 	 					     //jal
				     ({inst_i[14:12], inst_i[6:0]} == {`TYPE_I_JALR_FUNC3, `TYPE_I_JALR_OPCODE}) |			 //I-jalr
					 ({inst_i[6:0]} == `TYPE_B_OPCODE) |			 //B-beq
					 ((inst_i[6:0] == `TYPE_I_LOAD_OPCODE) & (inst_i[14:12] == `TYPE_I_LB_FUNC3 | inst_i[14:12] == `TYPE_I_LH_FUNC3 | inst_i[14:12] == `TYPE_I_LW_FUNC3 | inst_i[14:12] == `TYPE_I_LBU_FUNC3 | inst_i[14:12] == `TYPE_I_LHU_FUNC3)) |	 //I-lb lh lw lbu lhu
					 ((inst_i[6:0] == `TYPE_I_CSR_OPCODE) & (inst_i[14:12] == `TYPE_I_CSRRW_FUNC3 | inst_i[14:12] == `TYPE_I_CSRRS_FUNC3)) |	 //I-csrrw csrrs
					 ((inst_i[6:0] == `TYPE_S_OPCODE) & (inst_i[14:12] == `TYPE_S_SB_FUNC3 | inst_i[14:12] == `TYPE_S_SH_FUNC3 | inst_i[14:12] == `TYPE_S_SW_FUNC3))	|		//S-sb sh sw
					 ((inst_i[6:0] == `TYPE_I_BASE_OPCODE) & (inst_i[14:12] == `TYPE_I_SLTI_FUNC3 || inst_i[14:12] == `TYPE_I_SLTIU_FUNC3 || inst_i[14:12] == `TYPE_I_ADDI_FUNC3 || inst_i[14:12] == `TYPE_I_XORI_FUNC3 || inst_i[14:12] == `TYPE_I_ORI_FUNC3 || inst_i[14:12] == `TYPE_I_ANDI_FUNC3 || {inst_i[14:12], inst_i[31:25]} == `TYPE_I_SLLI_FUNC3_IMM || {inst_i[14:12], inst_i[31:25]} == `TYPE_I_SRLI_FUNC3_IMM || {inst_i[14:12], inst_i[31:25]} == `TYPE_I_SRAI_FUNC3_IMM)) |	 //I-addi slli srli srai xori ori andi
					 (inst_i[6:0] == `TYPE_R_OPCODE) | //R
					 (inst_i == `TYPE_I_ECALL) | 
					 (inst_i == `TYPE_I_MRET)  | 
					 (inst_i == `TYPE_I_EBREAK));

	import "DPI-C" function void inst_invalid_get(byte invalid);
		always @(*) begin
			// $display("pc = %x dpc = %x\n",pc,pc_next);
			inst_invalid_get({7'b0, inst_invalid});
		end

	parameter [1:0] IFU_WAIT_ADDR_PASS = 2'b00, IFU_WAIT_READY = 2'b01, IFU_WAIT_FINISH = 2'b10, IFU_WAIT_INST_LOAD = 2'b11;

	// state trans
	always @(posedge clock ) begin
		if(reset)
			con_state <= IFU_WAIT_ADDR_PASS;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
			IFU_WAIT_ADDR_PASS: begin
				if (addr_r_ready_i == 1'b1 && addr_r_valid_o == 1'b1) begin
					next_state = IFU_WAIT_INST_LOAD;
				end else begin 
					next_state = IFU_WAIT_ADDR_PASS;
				end
			end
			IFU_WAIT_INST_LOAD: begin
				if (r_ready_o == 1'b1 && r_valid_i == 1'b1 && r_resp_i == 2'b00) begin
					next_state = IFU_WAIT_READY;
				end else begin 
					next_state = IFU_WAIT_INST_LOAD;
				end
			end
			IFU_WAIT_READY: begin 
				// if (ready == 1'b0) begin
				// 	next_state = IFU_WAIT_READY;
				// end else begin 
					next_state = IFU_WAIT_FINISH;
				// end
			end
			IFU_WAIT_FINISH: begin 
				if (last_finish == 1'b0) begin
					next_state = IFU_WAIT_FINISH;
				end else begin 
					next_state = IFU_WAIT_ADDR_PASS;
				end
			end
			default:
				next_state = 2'b0;
		endcase
	end

    // get new pc
    ysyx_22041211_counter#(
        .ADDR_LEN         ( 32 )
    )u_ysyx_22041211_counter(
        .clock              ( clock              ),
        .reset              ( reset              ),
        .branch_request_i ( branch_request_i ),
        .branch_target_i  ( branch_target_i  ),
        .branch_flag_i    ( branch_flag_i    ),
        .pc_plus_4        ( pc_plus_4        ),
        .jmp_flag_i       ( jmp_flag_i       ),
        .jmp_target_i     ( jmp_target_i     ),
        .csr_jmp_i        ( csr_jmp_i        ),
        .csr_pc_i         ( csr_pc_i         ),
		.con_state        ( con_state        ),
        .last_finish      ( last_finish      ),
        .pc               ( pc               )
    );

	ysyx_22041211_pcPlus#(
		.DATA_LEN ( 32 )
	)u_ysyx_22041211_pcPlus(
		.pc_old ( pc ),
		.reset    ( reset    ),
		.pc_new ( pc_plus_4  )
	);

	// always @(*) begin
	// 	if(reset)
	// 		addr_r_addr_o = 0;
	// 	else if(addr_r_valid_o)
	// 		addr_r_addr_o = pc;
	// 	else 
	// 		addr_r_addr_o = 0;
	// end
	assign addr_r_addr_o = pc;
	assign id_inst_i = inst_i;

	// assign inst_invalid_o = inst_invalid;

endmodule
