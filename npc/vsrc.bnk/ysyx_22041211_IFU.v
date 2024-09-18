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
	// input 		[DATA_WIDTH - 1:0]			inst_i		,

    // get instruction
	output									ce		,
    input		[DATA_WIDTH - 1:0]			inst_i	,
    output reg								inst_invalid_o	,
    output reg	[DATA_WIDTH - 1:0]			id_inst_i	,
	output reg	[ADDR_WIDTH - 1:0]			pc			,

	// IFU-AXI
	// Addr Read
	output		[ADDR_WIDTH - 1:0]		addr_r_addr_o,
	output		                		addr_r_valid_o,
	input		                		addr_r_ready_i,

	// Read data
	input		[DATA_WIDTH - 1:0]		r_data_i	,
	input		[1:0]					r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		r_valid_i	,
	output		                		r_ready_o	,

	// Addr Write
	output		[DATA_WIDTH - 1:0]		addr_w_addr_o,	// 写地址
	output		                		addr_w_valid_o,	// 主设备给出的地址和相关控制信号有效
	input		                		addr_w_ready_i, // 从设备已准备好接收地址和相关的控制信号

	// Write data
	output		[DATA_WIDTH - 1:0]		w_data_o	,	// 写出的数据
	output		[3:0]					w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					bkwd_resp_i,	// 写回复信号，写操作是否成功
	input		                		bkwd_valid_i,	// 从设备给出的写回复信号是否有效
	output		                		bkwd_ready_o
);
	wire		[ADDR_WIDTH - 1:0]	        pc_plus_4	;
	reg			[1:0]				        con_state	;
	reg			[1:0]			        	next_state	;
	wire									r_inst_en	;
	wire									inst_invalid;
	assign ce = r_inst_en;
	assign r_inst_en = (con_state == IFU_IDLE && next_state == IFU_WAIT_READY);

	assign addr_r_addr_o = pc;
	assign addr_r_valid_o = ;

	assign inst_invalid = ~((id_inst_i[6:0] == `TYPE_U_LUI_OPCODE) | (id_inst_i[6:0] == `TYPE_U_AUIPC_OPCODE) | //U-auipc lui
					 (id_inst_i[6:0] == `TYPE_J_JAL_OPCODE) | 	 					     //jal
				     ({id_inst_i[14:12], id_inst_i[6:0]} == {`TYPE_I_JALR_FUNC3, `TYPE_I_JALR_OPCODE}) |			 //I-jalr
					 ({id_inst_i[6:0]} == `TYPE_B_OPCODE) |			 //B-beq
					 ((id_inst_i[6:0] == `TYPE_I_LOAD_OPCODE) & (id_inst_i[14:12] == `TYPE_I_LB_FUNC3 | id_inst_i[14:12] == `TYPE_I_LH_FUNC3 | id_inst_i[14:12] == `TYPE_I_LW_FUNC3 | id_inst_i[14:12] == `TYPE_I_LBU_FUNC3 | id_inst_i[14:12] == `TYPE_I_LHU_FUNC3)) |	 //I-lb lh lw lbu lhu
					 ((id_inst_i[6:0] == `TYPE_I_CSR_OPCODE) & (id_inst_i[14:12] == `TYPE_I_CSRRW_FUNC3 | id_inst_i[14:12] == `TYPE_I_CSRRS_FUNC3)) |	 //I-csrrw csrrs
					 ((id_inst_i[6:0] == `TYPE_S_OPCODE) & (id_inst_i[14:12] == `TYPE_S_SB_FUNC3 | id_inst_i[14:12] == `TYPE_S_SH_FUNC3 | id_inst_i[14:12] == `TYPE_S_SW_FUNC3))	|		//S-sb sh sw
					 ((id_inst_i[6:0] == `TYPE_I_BASE_OPCODE) & (id_inst_i[14:12] == `TYPE_I_SLTI_FUNC3 || id_inst_i[14:12] == `TYPE_I_SLTIU_FUNC3 || id_inst_i[14:12] == `TYPE_I_ADDI_FUNC3 || id_inst_i[14:12] == `TYPE_I_XORI_FUNC3 || id_inst_i[14:12] == `TYPE_I_ORI_FUNC3 || id_inst_i[14:12] == `TYPE_I_ANDI_FUNC3 || {id_inst_i[14:12], id_inst_i[31:25]} == `TYPE_I_SLLI_FUNC3_IMM || {id_inst_i[14:12], id_inst_i[31:25]} == `TYPE_I_SRLI_FUNC3_IMM || {id_inst_i[14:12], id_inst_i[31:25]} == `TYPE_I_SRAI_FUNC3_IMM)) |	 //I-addi slli srli srai xori ori andi
					 (id_inst_i[6:0] == `TYPE_R_OPCODE) | //R
					 (id_inst_i == `TYPE_I_ECALL) | 
					 (id_inst_i == `TYPE_I_MRET)  | 
					 (id_inst_i == `TYPE_I_EBREAK));

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


	always @(posedge clk) begin
        if(r_inst_en) begin
            id_inst_i   <=   inst_i;
        end
	end

	assign inst_invalid_o = inst_invalid;

endmodule
