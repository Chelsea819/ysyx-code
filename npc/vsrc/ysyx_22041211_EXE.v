/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/
`include "ysyx_22041211_define.v"
module ysyx_22041211_EXE #(parameter DATA_LEN = 32)(
	input									      clock				        ,
    input									      reset				        ,
	input		[DATA_LEN - 1:0]		reg1_i		,
	input		[DATA_LEN - 1:0]		reg2_i		,
	input		[DATA_LEN - 1:0]		pc_i		,
    // input		[DATA_LEN - 1:0]		inst		,
	input 		[3:0]					alu_control	,
    input 		[3:0]					alu_sel		, // choose source number
	input       [DATA_LEN - 1:0]        imm_i		,
	input       [DATA_LEN - 1:0]        csr_rdata_i	,
	input 		[2:0]					csr_flag_i	,
	input		                		wd_i		,
    input		[4:0]		            wreg_i		,
	input		[1:0]					store_type_i,
	input       [2:0]                   load_type_i ,
	input		[2:0]					branch_type_i,
	input                               ifu_valid    ,
    // input                                         isu_ready                 ,
    // output                                        exu_ready_o                 ,
    // output  reg                                   exu_valid_o                 ,
	output  reg    [2:0]                load_type_o 	,
	output  reg    [1:0]				store_type_o	,
	output	reg							branch_request_o,	
    output	reg	                		mem_wen_o		,
	output	reg	[DATA_LEN - 1:0]		mem_wdata_o		,
    output	reg	                		wd_o			,
    output	reg	[4:0]		            wreg_o			,
	output  reg    [DATA_LEN - 1:0]     csr_wdata_o		,
	output  reg    [2:0]			    csr_type_o		,
	output  reg    [DATA_LEN - 1:0]     csr_mcause_o	,
	output	reg	[DATA_LEN - 1:0]		pc_o			,
    output	reg	[DATA_LEN - 1:0]		alu_result_o
);
	// assign exu_ready_o = 1'b1;
	// assign exu_valid_o = 1'b1;
	reg                                   exu_valid_o    ;
	wire [31:0] src1;
	wire [31:0] src2;
	wire 		alu_zero;
	wire 		alu_less;
	assign wd_o  = wd_i;
	assign wreg_o  = wreg_i;
	assign mem_wen_o  = |store_type_i;
	assign mem_wdata_o  = reg2_i;
	assign load_type_o  = load_type_i;
	assign store_type_o  = store_type_i;
	assign pc_o  = pc_i;
	assign csr_mcause_o  = 32'hb;


	// always @(*) begin
	// 	$display("mem_wen_o = [%h] mem_wen_o = [%b]",mem_wen_o,mem_wen_o);
	// end

	reg			[1:0]			        	con_state	;
	reg			[1:0]			        	next_state	;
    parameter [1:0] EXU_WAIT_IDU_VALID = 2'b00, EXU_WAIT_EXU_VALID = 2'b01, EXU_WAIT_WB_READY = 2'b10;

	always @(posedge clock ) begin
		if(next_state == EXU_WAIT_EXU_VALID)
			exu_valid_o <= 1'b1;
		else 
			exu_valid_o <= 1'b0;
	end

	// state trans
	always @(posedge clock ) begin
		if(reset)
			con_state <= EXU_WAIT_IDU_VALID;
		else 
			con_state <= next_state;
	end

	// next_state
	always @(*) begin
		case(con_state) 
            // 等待ifu取指，下一个时钟周期开始译码
			EXU_WAIT_IDU_VALID: begin
				if (ifu_valid == 1'b0) begin
					next_state = EXU_WAIT_IDU_VALID;
				end else begin 
					next_state = EXU_WAIT_EXU_VALID;
				end
			end
            // 等待idu完成译码
			EXU_WAIT_EXU_VALID: begin 
				if (exu_valid_o == 1'b0) begin
					next_state = EXU_WAIT_EXU_VALID;
				end else begin 
					next_state = EXU_WAIT_WB_READY;
				end
			end
            // 等待exu空闲，下个时钟周期传递信息
            EXU_WAIT_WB_READY: begin 
				// if (isu_ready == 1'b0) begin
				// 	next_state = EXU_WAIT_WB_READY;
				// end else begin 
					next_state = EXU_WAIT_IDU_VALID;
				// end
			end
            default: begin 
				next_state = 2'b11;
			end
		endcase
	end

	ysyx_22041211_MuxKeyWithDefault #(6,3,1) branch_request_mux (branch_request_o , branch_type_i, 1'b0, {
		`BRANCH_BEQ, alu_zero,
		`BRANCH_BNE, ~alu_zero,
		`BRANCH_BLT, alu_less,
		`BRANCH_BGE, ~alu_less,
		`BRANCH_BLTU, alu_less,
		`BRANCH_BGEU, ~alu_less
	});

	ysyx_22041211_MuxKeyWithDefault #(2,3,32) csr_wdata_choose (csr_wdata_o , csr_flag_i, 32'b0, {
		`CSR_CSRRW, reg1_i,
		`CSR_CSRRS, reg1_i | csr_rdata_i
	});

	assign csr_type_o = csr_flag_i;

	ysyx_22041211_ALU my_alu(
		.src1				(src1),
		.src2				(src2),
		.alu_control		(alu_control),
		.result				(alu_result_o ),
		.alu_less_o 		(alu_less),
		.alu_zero_o  		(alu_zero)
	);

	ysyx_22041211_MuxKeyWithDefault #(4,2,32) src1_choose (src1, alu_sel[1:0], 32'b0, {
		`ALU_SEL1_ZERO, 32'b0,
		`ALU_SEL1_REG1, reg1_i,
		`ALU_SEL1_PC,   pc_i,
		`ALU_SEL1_CSR,  csr_rdata_i
	});

	ysyx_22041211_MuxKeyWithDefault #(4,2,32) src2_choose (src2, alu_sel[3:2], 32'b0, {
		`ALU_SEL1_ZERO, 32'b0,
		`ALU_SEL2_REG2, reg2_i,
		`ALU_SEL2_IMM, 	imm_i,
		`ALU_SEL2_4, 	32'b100
	});


endmodule

