`include "./ysyx_22041211_define.v"
module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32) (
	input								clk ,
	input								rst ,
	output			[ADDR_LEN - 1:0]	pc			,
	output								invalid
);
	wire			[ADDR_LEN - 1:0]	inst			; // 正在执行的指令
	// //registerFile
	// wire								reg_re1_i		;
	wire			[4:0]				reg_raddr1_i	;
	// wire								reg_re2_i		;
	wire			[4:0]				reg_raddr2_i	;
	wire								reg_wen_i		;
	wire			[4:0]				reg_waddr_i		;
	wire	        [DATA_LEN - 1:0]    reg_wdata_i		;

	//my_IFU
	wire								idu_ready_o	;
	wire								if_last_finish_i;
	wire			[ADDR_LEN - 1:0]	if_branch_target_i;
	wire			[2:0]				if_branch_type_i;
	wire								if_branch_request_i;	
	wire			[ADDR_LEN - 1:0]	if_jmp_target_i;
	wire								if_jmp_flag_i;	
	wire			[ADDR_LEN - 1:0]	if_csr_pc_i;
	// wire			[ADDR_LEN - 1:0]	pcPlus		;
	// wire			[ADDR_LEN - 1:0]	pcBranch	;
	// wire			[1:0]				pcSrc		;

	//my_decoder
	wire								ifu_valid_o		;
	wire								exu_ready_o		;
	wire								idu_valid_o		;
	wire			[ADDR_LEN - 1:0]	id_pc_i			;
	wire			[ADDR_LEN - 1:0]	id_inst_i		;
	wire			[ADDR_LEN - 1:0]	id_reg1_data_i	;
	wire			[ADDR_LEN - 1:0]	id_reg2_data_i	;


	// execute
	wire			[3:0]				ex_aluop_i		;
	wire			[3:0]				ex_alusel_i		;
	wire			[DATA_LEN - 1:0]	ex_reg1_i		;
	wire			[DATA_LEN - 1:0]	ex_reg2_i		;
	wire			[DATA_LEN - 1:0]	ex_imm_i		;
	wire			[DATA_LEN - 1:0]	ex_inst_i		;
	wire			[DATA_LEN - 1:0]	ex_pc_i			;
	wire								ex_wd_i			;
	wire			[4:0]				ex_wreg_i		;
	wire			[1:0]				ex_store_type_i	;
	wire			[2:0]				ex_load_type_i	;
	wire			[2:0]				ex_csr_flag_i	;
	wire			[31:0]				ex_csr_rdata_i	;

	// csr Unit
	wire			[11:0]				csr_addr_i	;
	wire			[DATA_LEN - 1:0]	csr_wdata_i		;
	wire	        [DATA_LEN - 1:0]    csr_mepc_i		;
	wire	        [DATA_LEN - 1:0]    csr_mcause_i	;

	// wb Unit
	wire			[DATA_LEN - 1:0]	wb_mem_wdata_i	;
	wire			[DATA_LEN - 1:0]	wb_csr_wdata_i	;
	wire								wb_mem_wen_i	;
	wire								wb_wd_i			;
	wire								wb_ready_o		;
	wire								exu_valid_o		;
	wire			[4:0]				wb_wreg_i		;
	wire			[DATA_LEN - 1:0]	wb_alu_result_i	;
	wire			[2:0]				wb_load_type_i	;
	wire			[1:0]				wb_store_type_i	;
	
	assign pc = id_pc_i;
	
	// 检测到ebreak
    import "DPI-C" function void ifebreak_func(int inst);
    always @(*)
        ifebreak_func(inst);

	// 为ITRACE提供指令
    import "DPI-C" context function void inst_get(int inst);
    always @(*)
        inst_get(inst);

	always @(*) begin
		$display("pc: [%h] inst: [%b] invalid: [%h]",id_pc_i, inst, invalid);
	end

ysyx_22041211_IFU#(
    .ADDR_WIDTH       ( 32 ),
    .DATA_WIDTH       ( 32 )
)u_ysyx_22041211_IFU(
    .clk              ( clk              ),
    .rst              ( rst              ),
    .valid            ( ifu_valid_o           ),
    .last_finish      ( if_last_finish_i    ),
    .ready            ( idu_ready_o 			),
    .branch_request_i ( if_branch_request_i ),
	.branch_target_i  ( if_branch_target_i  ),
	.branch_flag_i    ( |if_branch_type_i    ),
	.jmp_flag_i  	  ( if_jmp_flag_i  ),
	.jmp_target_i     ( if_jmp_target_i    ),
	.csr_jmp_i     	  ( ex_csr_flag_i[2]  ),
	.csr_pc_i         ( if_csr_pc_i      ),
    .inst_o           ( inst           ),
	.id_inst_i        ( id_inst_i           ),
    .invalid          ( invalid           ),
    .pc               ( id_pc_i             )
);


	ysyx_22041211_RegisterFile my_RegisterFile(
		.clk		(clk),
		.wdata		(reg_wdata_i),
		.rd			(reg_waddr_i),
		.rsc1		(reg_raddr1_i),
		.rsc2		(reg_raddr2_i),
		.rst		(rst)		 ,
		.regWrite	(reg_wen_i),
		.r_data1	(id_reg1_data_i),
		.r_data2	(id_reg2_data_i)
	);

	ysyx_22041211_decoder my_decoder(
		.clk              				( clk              ),
		.rst              				( rst              ),
		.inst_i							(id_inst_i),
		.reg1_data_i					(id_reg1_data_i),
		.reg2_data_i					(id_reg2_data_i),
		.pc_i       					(id_pc_i),	

		.ifu_valid    					(ifu_valid_o),	
		.exu_ready   					(exu_ready_o),
		.idu_ready_o       				(idu_ready_o),
		.idu_valid_o     				(idu_valid_o),

		.aluop_o    					(ex_aluop_i),	
		.alusel_o   					(ex_alusel_i),
		.pc_o       					(ex_pc_i),
		.reg1_o     					(ex_reg1_i),
		.reg2_o     					(ex_reg2_i),
		.wd_o       					(ex_wd_i),
		.wreg_o     					(ex_wreg_i),
		.reg1_addr_o					(reg_raddr1_i),
		.reg2_addr_o					(reg_raddr2_i),
		.branch_type_o					(if_branch_type_i),
		.branch_target_o				(if_branch_target_i),
		.jmp_flag_o						(if_jmp_flag_i),
		.jmp_target_o					(if_jmp_target_i),
		.store_type_o					(ex_store_type_i),
		.load_type_o					(ex_load_type_i),
		.csr_addr_o						(csr_addr_i),
		.csr_flag_o						(ex_csr_flag_i),  
		// .inst_o     					(ex_inst_i),
		.imm_o      					(ex_imm_i)
	);

	ysyx_22041211_EXE my_execute(
		.clk              	( clk     ),
		.rst              	( rst     ),
		.reg1_i				(ex_reg1_i),
		.reg2_i				(ex_reg2_i),
		.pc_i				(ex_pc_i),
		// .inst			(ex_inst_i),
		.alu_control		(ex_aluop_i),
		.alu_sel			(ex_alusel_i),		
		.imm_i				(ex_imm_i),
		.csr_rdata_i		(ex_csr_rdata_i),
		.csr_flag_i			(ex_csr_flag_i),
		.wd_i				(ex_wd_i),	
		.wreg_i				(ex_wreg_i),
		.branch_type_i		(if_branch_type_i),	

		.idu_valid			(idu_valid_o),
		.wb_ready			(wb_ready_o),	
		.exu_ready_o		(exu_ready_o),
		.exu_valid_o		(exu_valid_o),

		.store_type_i		(ex_store_type_i),	
		.load_type_i		(ex_load_type_i),
		.branch_request_o	(if_branch_request_i),
		.wd_o				(wb_wd_i),	
		.wreg_o				(wb_wreg_i),
		.mem_wen_o			(wb_mem_wen_i),	
		.mem_wdata_o		(wb_mem_wdata_i),	
		.csr_wdata_o		(wb_csr_wdata_i),
		.csr_mcause_o		(csr_mcause_i),
		.pc_o				(csr_mepc_i),
		.load_type_o		(wb_load_type_i),
		.store_type_o		(wb_store_type_i),
		.alu_result_o		(wb_alu_result_i)
	);

	ysyx_22041211_wb #(
		.DATA_LEN ( 32 )
	)u_ysyx_22041211_wb(
		.clk			(clk),
		.rst            ( rst     ),
		.wd_i     		( wb_wd_i     ),
		.wreg_i   		( wb_wreg_i   ),
		.alu_result_i   ( wb_alu_result_i  	),
		.csr_wdata_i	( wb_csr_wdata_i),
		.mem_wen_i     	( wb_mem_wen_i   ),
		.mem_wdata_i   	( wb_mem_wdata_i ),
		.load_type_i	(wb_load_type_i),
		.store_type_i	(wb_store_type_i),
		.csr_wdata_o	( csr_wdata_i),

		.exu_valid     	( exu_valid_o   ),
		.wb_ready_o   	( wb_ready_o ),
		.finish  		( if_last_finish_i ),

		.wd_o     		( reg_wen_i   ),
		.wreg_o   		( reg_waddr_i ),
		.wdata_o  		( reg_wdata_i )
	);

	ysyx_22041211_CSR#(
		.DATA_WIDTH    ( 32 )
	)u_ysyx_22041211_CSR(
		.clk           ( clk           ),
		.rst           ( rst           ),
		.csr_addr      ( csr_addr_i      ),
		.wdata         ( csr_wdata_i         ),
		.csr_type_i    ( ex_csr_flag_i    ),
		.csr_mepc_i    ( csr_mepc_i    ),
		.csr_mcause_i  ( csr_mcause_i  ),
		.csr_pc_o      ( if_csr_pc_i      ),
		.r_data        ( ex_csr_rdata_i     )
	);


	// ysyx_22041211_pcPlusBranch my_pcPlusBranch (
	// 	.offset		(imm),
	// 	.pc_old		(id_pc_i),
	// 	.pcBranch	(pcBranch)
	// );

	// ysyx_22041211_branchJmp my_branchJmp(
	// 	.zero		(zero),
	// 	.SF			(ALUResult[0]),
	// 	.branch		(branch),
	// 	.invalid	(invalid),
	// 	.jmp		(jmp),
	// 	.PCSrc		(pcSrc)
	// );
	

	// ysyx_22041211_MuxKey #(2,1,32) ALUSrc_choosing (srcB,ALUSrc, {
	// 	1'b1, imm,
	// 	1'b0, reg_data2
	// });

	// ysyx_22041211_MuxKey #(4,2,32) memToReg_choosing (WriteData, memToReg_tmp, {
	// 	2'b00, ALUResult,
	// 	2'b01, ReadData_tmp	,
	// 	2'b10, pcPlus	,
	// 	2'b11, pcBranch
	// });



endmodule
