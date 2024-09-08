`include "./ysyx_22041211_define.v"
module ysyx_22041211_cpu #(parameter DATA_LEN = 32,ADDR_LEN = 32) (
	input								clk 		,
	input								rst 		,
	input	        [DATA_LEN - 1:0]    inst_i		,
	input	        [DATA_LEN - 1:0]    mem_rdata_i	,
    output		                		mem_ren_o	,
    output		                		mem_wen_o	,
	output		    [DATA_LEN - 1:0]	mem_wdata_o	,
	output		    [ADDR_LEN - 1:0]	mem_waddr_o	,
	output		    [ADDR_LEN - 1:0]	mem_raddr_o	,
	output		    [7:0]  				mem_wmask_o	,
	output		    [7:0]  				mem_rmask_o	,
	output			[ADDR_LEN - 1:0]	pc			,
	output								inst_ren	,
	output								invalid		,
	output								finish
);
	// //registerFile
	// wire								reg_re1_i		;
	wire			[4:0]				reg_raddr1_i	;
	// wire								reg_re2_i		;
	wire			[4:0]				reg_raddr2_i	;
	wire								reg_wen_i		;
	wire			[4:0]				reg_waddr_i		;
	wire	        [DATA_LEN - 1:0]    reg_wdata_i		;

	//my_IFU
	// wire								idu_ready_o	;
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
	// wire								exu_ready_o		;
	// wire								idu_valid_o		;
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
	wire			[2:0]				csr_type_i		;
	wire	        [DATA_LEN - 1:0]    csr_mepc_i		;
	wire	        [DATA_LEN - 1:0]    csr_mcause_i	;

	// lsu
	wire			[DATA_LEN - 1:0]	lsu_mem_wdata_i	;
	wire			[DATA_LEN - 1:0]	lsu_csr_wdata_i	;
	wire			[2:0]				lsu_csr_type_i	;
	wire								lsu_mem_wen_i	;
	wire								lsu_wd_i		;
	// wire								exu_valid_o		;
	wire								lsu_ready_o		;
	wire								lsu_valid_o		;
	wire			[4:0]				lsu_wreg_i		;
	wire			[DATA_LEN - 1:0]	lsu_alu_result_i	;
	wire			[2:0]				lsu_load_type_i	;
	wire			[1:0]				lsu_store_type_i	;

	// wb Unit
	wire			[DATA_LEN - 1:0]	wb_reg_wdata_i	;
	wire			[DATA_LEN - 1:0]	wb_csr_wdata_i	;
	wire			[2:0]				wb_csr_type_i	;
	wire								wb_reg_wen_i	;
	wire			[4:0]				wb_wreg_i		;
	wire								wb_memory_inst_i;
	wire								wb_ready_o		;
	
	assign pc = id_pc_i;
	assign finish = if_last_finish_i;
	
	// always @(*) begin
	// 	$display("pc: [%h] inst: [%b] invalid: [%h]",id_pc_i, inst, invalid);
	// end

ysyx_22041211_IFU#(
    .ADDR_WIDTH       ( 32 ),
    .DATA_WIDTH       ( 32 )
)u_ysyx_22041211_IFU(
    .clk              ( clk              ),
    .rst              ( rst              ),
    .valid            ( ifu_valid_o           ),
    .last_finish      ( if_last_finish_i    ),
    // .ready            ( idu_ready_o 			),
    .branch_request_i ( if_branch_request_i ),
	.branch_target_i  ( if_branch_target_i  ),
	.branch_flag_i    ( |if_branch_type_i    ),
	.jmp_flag_i  	  ( if_jmp_flag_i  ),
	.jmp_target_i     ( if_jmp_target_i    ),
	.csr_jmp_i     	  ( ex_csr_flag_i[2]  ),
	.csr_pc_i         ( if_csr_pc_i      ),
	.inst_i           ( inst_i           ),
	.id_inst_i        ( id_inst_i           ),
    .inst_invalid_o   ( invalid           ),
	.ce				  (	inst_ren			),
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
		// .exu_ready   					(exu_ready_o),
		// .idu_ready_o       				(idu_ready_o),
		// .idu_valid_o     				(idu_valid_o),

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
		.imm_o      					(ex_imm_i)
	);

	ysyx_22041211_EXE my_execute(
		.clk              	( clk     ),
		.rst              	( rst     ),
		.reg1_i				(ex_reg1_i),
		.reg2_i				(ex_reg2_i),
		.pc_i				(ex_pc_i),
		.alu_control		(ex_aluop_i),
		.alu_sel			(ex_alusel_i),		
		.imm_i				(ex_imm_i),
		.csr_rdata_i		(ex_csr_rdata_i),
		.csr_flag_i			(ex_csr_flag_i),
		.wd_i				(ex_wd_i),	
		.wreg_i				(ex_wreg_i),
		.branch_type_i		(if_branch_type_i),	
		.ifu_valid			(ifu_valid_o),
		// .isu_ready		(lsu_ready_o),	
		// .exu_ready_o		(exu_ready_o),
		// .exu_valid_o		(exu_valid_o),
		.store_type_i		(ex_store_type_i),	
		.load_type_i		(ex_load_type_i),
		.branch_request_o	(if_branch_request_i),
		.wd_o				(lsu_wd_i),	
		.wreg_o				(lsu_wreg_i),
		.mem_wen_o			(lsu_mem_wen_i),	
		.mem_wdata_o		(lsu_mem_wdata_i),	
		.csr_wdata_o		(lsu_csr_wdata_i),
		.csr_type_o			(lsu_csr_type_i),
		.csr_mcause_o		(csr_mcause_i),
		.pc_o				(csr_mepc_i),
		.load_type_o		(lsu_load_type_i),
		.store_type_o		(lsu_store_type_i),
		.alu_result_o		(lsu_alu_result_i)
	);

	ysyx_22041211_LSU#(
		.DATA_LEN      ( 32 )
	)u_ysyx_22041211_LSU(
		.rst           ( rst           ),
		.wd_i          ( lsu_wd_i          ),
		.clk           ( clk           		),
		.wreg_i   		( lsu_wreg_i   		),
		.alu_result_i   ( lsu_alu_result_i  	),
		.mem_wen_i     	( lsu_mem_wen_i   	),
		.mem_wdata_i   	( lsu_mem_wdata_i 	),
		.memory_inst_o  ( wb_memory_inst_i ),
		.load_type_i	( lsu_load_type_i	),
		.store_type_i	( lsu_store_type_i	),
		.csr_wdata_i	( lsu_csr_wdata_i	),
		.csr_type_i		( lsu_csr_type_i	),

		.mem_rdata_rare_i( mem_rdata_i   	),
		.mem_ren_o   	( mem_ren_o 		),
		.mem_wen_o   	( mem_wen_o   	),
		.mem_wdata_o    ( mem_wdata_o   	),
		.mem_waddr_o    ( mem_waddr_o   		),
		.mem_raddr_o    ( mem_raddr_o 		),
		.mem_wmask_o  	( mem_wmask_o 	),
		.mem_rmask_o	( mem_rmask_o	),

		.ifu_valid     	( ifu_valid_o   	),
		// .wb_ready_o   	( wb_ready_o 		),
		// .lsu_ready_o    ( lsu_ready_o   	),
		.lsu_valid_o    ( lsu_valid_o   	),
		.wd_o     		( wb_reg_wen_i   		),
		.wreg_o   		( wb_wreg_i 		),
		.wdata_o  		( wb_reg_wdata_i 	),
		.csr_type_o		( wb_csr_type_i	),
		.csr_wdata_o    ( wb_csr_wdata_i   	)
	);

	ysyx_22041211_wb#(
		.DATA_LEN     ( 32 )
	)u_ysyx_22041211_wb(
		.rst          ( rst          ),
		.wd_i         ( wb_reg_wen_i ),
		.clk          ( clk          ),
		.wreg_i       ( wb_wreg_i       ),
		.csr_wdata_i  ( wb_csr_wdata_i  ),
		.csr_type_i   ( wb_csr_type_i  ),
		.reg_wdata_i  ( wb_reg_wdata_i  ),
		.memory_inst_i( wb_memory_inst_i ),
		.ifu_valid    ( ifu_valid_o    ),
		.lsu_valid    ( lsu_valid_o    ),
		// .wb_ready_o   ( wb_ready_o   ),
		.finish       ( if_last_finish_i ),
		.wd_o     	  ( reg_wen_i   ),
		.wreg_o   	  ( reg_waddr_i ),
		.wdata_o  	  ( reg_wdata_i ),
		.csr_type_o   ( csr_type_i  ),
		.csr_wdata_o  ( csr_wdata_i  )
	);


	ysyx_22041211_CSR#(
		.DATA_WIDTH    ( 32 )
	)u_ysyx_22041211_CSR(
		.clk           ( clk           ),
		.rst           ( rst           ),
		.csr_addr      ( csr_addr_i      ),
		.wdata         ( csr_wdata_i         ),
		.csr_type_i    ( csr_type_i    ),
		.csr_mepc_i    ( csr_mepc_i    ),  
		.csr_mcause_i  ( csr_mcause_i  ),
		.csr_pc_o      ( if_csr_pc_i      ),
		.r_data        ( ex_csr_rdata_i     )
	);

endmodule
