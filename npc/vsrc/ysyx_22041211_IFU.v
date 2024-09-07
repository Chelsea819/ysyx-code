/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/
`include "./ysyx_22041211_define.v"
module ysyx_22041211_IFU #(parameter ADDR_WIDTH = 32, DATA_WIDTH = 32)(
	input									clk				,
	input									rst				,

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

    // get instruction
	// output	[ADDR_LEN - 1:0]			ce		,
    // input		[DATA_WIDTH - 1:0]			inst_i	,
    output									invalid	,
    output 		[DATA_WIDTH - 1:0]			id_inst_i	,
	output reg	[DATA_WIDTH - 1:0]			inst_o		,	// 外部api 当前周期正在执行的指令
	output reg	[ADDR_WIDTH - 1:0]			pc
);
	wire		[ADDR_WIDTH - 1:0]	        pc_plus_4	;
	reg			[1:0]				        con_state	;
	reg			[1:0]			        	next_state	;
	wire									r_inst_en	;

	assign r_inst_en = (con_state == IFU_IDLE && next_state == IFU_WAIT_READY);

	assign invalid = ~((inst_o[6:0] == `TYPE_U_LUI_OPCODE) | (inst_o[6:0] == `TYPE_U_AUIPC_OPCODE) | //U-auipc lui
					 (inst_o[6:0] == `TYPE_J_JAL_OPCODE) | 	 					     //jal
				     ({inst_o[14:12], inst_o[6:0]} == {`TYPE_I_JALR_FUNC3, `TYPE_I_JALR_OPCODE}) |			 //I-jalr
					 ({inst_o[6:0]} == `TYPE_B_OPCODE) |			 //B-beq
					 ((inst_o[6:0] == `TYPE_I_LOAD_OPCODE) & (inst_o[14:12] == `TYPE_I_LB_FUNC3 | inst_o[14:12] == `TYPE_I_LH_FUNC3 | inst_o[14:12] == `TYPE_I_LW_FUNC3 | inst_o[14:12] == `TYPE_I_LBU_FUNC3 | inst_o[14:12] == `TYPE_I_LHU_FUNC3)) |	 //I-lb lh lw lbu lhu
					 ((inst_o[6:0] == `TYPE_I_CSR_OPCODE) & (inst_o[14:12] == `TYPE_I_CSRRW_FUNC3 | inst_o[14:12] == `TYPE_I_CSRRS_FUNC3)) |	 //I-csrrw csrrs
					 ((inst_o[6:0] == `TYPE_S_OPCODE) & (inst_o[14:12] == `TYPE_S_SB_FUNC3 | inst_o[14:12] == `TYPE_S_SH_FUNC3 | inst_o[14:12] == `TYPE_S_SW_FUNC3))	|		//S-sb sh sw
					 ((inst_o[6:0] == `TYPE_I_BASE_OPCODE) & (inst_o[14:12] == `TYPE_I_SLTI_FUNC3 || inst_o[14:12] == `TYPE_I_SLTIU_FUNC3 || inst_o[14:12] == `TYPE_I_ADDI_FUNC3 || inst_o[14:12] == `TYPE_I_XORI_FUNC3 || inst_o[14:12] == `TYPE_I_ORI_FUNC3 || inst_o[14:12] == `TYPE_I_ANDI_FUNC3 || {inst_o[14:12], inst_o[31:25]} == `TYPE_I_SLLI_FUNC3_IMM || {inst_o[14:12], inst_o[31:25]} == `TYPE_I_SRLI_FUNC3_IMM || {inst_o[14:12], inst_o[31:25]} == `TYPE_I_SRAI_FUNC3_IMM)) |	 //I-addi slli srli srai xori ori andi
					 (inst_o[6:0] == `TYPE_R_OPCODE) | //R
					 (inst_o == `TYPE_I_ECALL) | 
					 (inst_o == `TYPE_I_MRET)  | 
					 (inst_o == `TYPE_I_EBREAK));

	parameter [1:0] IFU_IDLE = 2'b00, IFU_WAIT_READY = 2'b01, IFU_WAIT_FINISH = 2'b10;

	always @(posedge clk ) begin
		if(next_state == IFU_IDLE)
			valid <= 1'b1;
		else 
			valid <= 1'b0;
	end

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
				// if (ready == 1'b0) begin
				// 	next_state = IFU_WAIT_READY;
				// end else begin 
					next_state = IFU_WAIT_FINISH;
				// end
			end
			IFU_WAIT_FINISH: begin 
				if (last_finish == 1'b0 || last_finish == 1'b0) begin
					next_state = IFU_WAIT_FINISH;
				end else begin 
					next_state = IFU_IDLE;
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
        .last_finish      ( last_finish      ),
        .pc               ( pc               )
    );

	ysyx_22041211_pcPlus#(
		.DATA_LEN ( 32 )
	)u_ysyx_22041211_pcPlus(
		.pc_old ( pc ),
		.rst    ( rst    ),
		.pc_new ( pc_plus_4  )
	);

	// ysyx_22041211_inst_SRAM#(
	// 	.ADDR_LEN ( 32 ),
	// 	.DATA_LEN ( 32 )
	// )u_ysyx_22041211_inst_SRAM(
	// 	.clk      ( clk      ),
	// 	.rst      ( rst      ),
	// 	.ren      ( r_inst_en),
	// 	.pc       ( pc       ),
	// 	.inst     ( inst_o     ),
	// 	.id_inst_i( id_inst_i)
	// );

	ysyx_22041211_SRAM#(
		.ADDR_LEN     ( 32 ),
		.DATA_LEN     ( 32 )
	)u_ysyx_22041211_SRAM(
		.rst          ( rst          ),
		.clk          ( clk          ),
		.ren          ( r_inst_en    ),
		.mem_wen_i    ( 0    ),
		.mem_wdata_i  ( 0  ),
		.mem_waddr_i  ( 0  ),
		.mem_raddr_i  ( pc  ),
		.mem_wmask    ( 0    ),
		.mem_rmask    ( 8'b00001111 ),
		.mem_rdata_usigned_o  ( inst_o  )
	);

	always @(posedge clk) begin
        if(r_inst_en) begin
            id_inst_i   <=   inst_o;
        end
	end



endmodule
