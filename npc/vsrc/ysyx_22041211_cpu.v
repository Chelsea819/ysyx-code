`include "ysyx_22041211_define.v"
module ysyx_22041211_cpu #(parameter DATA_LEN = 32,ADDR_LEN = 32) (
	input								clock 		,
	input								reset 		,

	// IFU-AXI
	// Addr Read
	output		[ADDR_LEN - 1:0]		inst_addr_r_addr_o,
	output		                		inst_addr_r_valid_o,
	input		                		inst_addr_r_ready_i,

	// Read data
	// input		[DATA_LEN - 1:0]		inst_r_data_i	,
	input		[1:0]					inst_r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		inst_r_valid_i	,
	output		                		inst_r_ready_o	,

	// data AXI
	 //Addr Read
	output		[ADDR_LEN - 1:0]		data_addr_r_addr_o,
	output		                		data_addr_r_valid_o,
	input		                		data_addr_r_ready_i,
	output		[2:0]                	data_addr_r_size_o,



	// Read data
	input		[DATA_LEN - 1:0]		data_r_data_i	,
	input		[1:0]					data_r_resp_i	,	// 读操作是否成功，存储器处理读写事物时可能会发生错误
	input		                		data_r_valid_i	,
	output		                		data_r_ready_o	,

	// Addr Write
	output		[ADDR_LEN - 1:0]		data_addr_w_addr_o,	// 写地址
	output		                		data_addr_w_valid_o,	// 主设备给出的地址和相关控制信号有效
	input		                		data_addr_w_ready_i, // 从设备已准备好接收地址和相关的控制信号
	output		[2:0]                	data_addr_w_size_o,

	// Write data
	output		[DATA_LEN - 1:0]		data_w_data_o	,	// 写出的数据
	output		[3:0]					data_w_strb_o	,	// wmask 	数据的字节选通，数据中每8bit对应这里的1bit
	output		                		data_w_valid_o	,	// 主设备给出的数据和字节选通信号有效
	input		                		data_w_ready_i	,	// 从设备已准备好接收数据选通信号

	// Backward
	input		[1:0]					data_bkwd_resp_i,	// 写回复信号，写操作是否成功
	input		                		data_bkwd_valid_i,	// 从设备给出的写回复信号是否有效
	output		                		data_bkwd_ready_o,	// 主设备已准备好接收写回复信号

	input	        [DATA_LEN - 1:0]    inst_i		
	// output			[ADDR_LEN - 1:0]	pc			,
	// output								invalid		,
	// output								finish
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
	
	// assign pc = id_pc_i;
	// assign finish = if_last_finish_i;
	
	// always @(*) begin
	// 	$display("pc: [%h] inst: [%b] invalid: [%h] reset: [%b] clock[%b]",pc, id_inst_i, invalid, reset, clock);
	// end

ysyx_22041211_IFU#(
    .ADDR_WIDTH       ( 32 ),
    .DATA_WIDTH       ( 32 )
)ysyx_22041211_IFU(
    .clock              ( clock              ),
    .reset              ( reset              ),
	.addr_r_addr_o    ( inst_addr_r_addr_o              ),		
    .addr_r_valid_o   ( inst_addr_r_valid_o              ),
    .addr_r_ready_i   ( inst_addr_r_ready_i              ),
    // .r_data_i         ( inst_i              ),
    .r_resp_i         ( inst_r_resp_i              ),
    .r_valid_i        ( inst_r_valid_i              ),
    .r_ready_o        ( inst_r_ready_o              ),
	

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
    // .inst_invalid_o   ( invalid           ),
    .pc               ( id_pc_i             )
);


	ysyx_22041211_RegisterFile ysyx_22041211_RegisterFile(
		.clock		(clock),
		.wdata		(reg_wdata_i),
		.rd			(reg_waddr_i),
		.rsc1		(reg_raddr1_i),
		.rsc2		(reg_raddr2_i),
		.reset		(reset)		 ,
		.regWrite	(reg_wen_i),
		.r_data1	(id_reg1_data_i),
		.r_data2	(id_reg2_data_i)
	);

	ysyx_22041211_decoder ysyx_22041211_decoder(
		.clock              				( clock              ),
		.reset              				( reset              ),
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

	ysyx_22041211_EXE ysyx_22041211_EXE(
		.clock              	( clock     ),
		.reset              	( reset     ),
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
		.DATA_LEN          ( 32 ),
		.ADDR_LEN          ( 32 )
	)ysyx_22041211_LSU(
		.rstn           ( ~reset           ),
		.wd_i          ( lsu_wd_i          ),
		.clock           ( clock           		),
		.wreg_i   		( lsu_wreg_i   		),
		.alu_result_i   ( lsu_alu_result_i  	),
		.mem_wen_i     	( lsu_mem_wen_i   	),
		.mem_wdata_i   	( lsu_mem_wdata_i 	),

		.memory_inst_o  ( wb_memory_inst_i ),
		.load_type_i	( lsu_load_type_i	),
		.store_type_i	( lsu_store_type_i	),
		.csr_wdata_i	( lsu_csr_wdata_i	),
		.csr_type_i		( lsu_csr_type_i	),

		.ifu_valid     	( ifu_valid_o   	),
		// .wb_ready_o   	( wb_ready_o 		),
		// .lsu_ready_o    ( lsu_ready_o   	),
		.lsu_valid_o    ( lsu_valid_o   	),
		.wd_o     		( wb_reg_wen_i   		),
		.wreg_o   		( wb_wreg_i 		),
		.wdata_o  		( wb_reg_wdata_i 	),
		.csr_type_o		( wb_csr_type_i	),
		.csr_wdata_o    ( wb_csr_wdata_i   	),

		.mem_rdata_rare_i  ( data_r_data_i   	),

		.addr_r_addr_o     ( data_addr_r_addr_o     ),
		.addr_r_valid_o    ( data_addr_r_valid_o    ),
		.addr_r_ready_i    ( data_addr_r_ready_i    ),
		.addr_r_size_o	   ( data_addr_r_size_o		),
		.r_data_i          ( data_r_data_i          ),
		.r_resp_i          ( data_r_resp_i          ),
		.r_valid_i         ( data_r_valid_i         ),
		.r_ready_o         ( data_r_ready_o         ),
		.addr_w_addr_o     ( data_addr_w_addr_o     ),
		.addr_w_valid_o    ( data_addr_w_valid_o    ),
		.addr_w_ready_i    ( data_addr_w_ready_i    ),
		.addr_w_size_o	   ( data_addr_w_size_o		),
		.w_data_o          ( data_w_data_o          ),
		.w_strb_o          ( data_w_strb_o          ),
		.w_valid_o         ( data_w_valid_o         ),
		.w_ready_i         ( data_w_ready_i         ),
		.bkwd_resp_i       ( data_bkwd_resp_i       ),
		.bkwd_valid_i      ( data_bkwd_valid_i      ),
		.bkwd_ready_o      ( data_bkwd_ready_o      )
	);


	// ysyx_22041211_LSU#(
	// 	.DATA_LEN      ( 32 )
	// )u_ysyx_22041211_LSU(
	// 	.reset           ( reset           ),
	// 	.wd_i          ( lsu_wd_i          ),
	// 	.clock           ( clock           		),
	// 	.wreg_i   		( lsu_wreg_i   		),
	// 	.alu_result_i   ( lsu_alu_result_i  	),
	// 	.mem_wen_i     	( lsu_mem_wen_i   	),
	// 	.mem_wdata_i   	( lsu_mem_wdata_i 	),

	// 	.memory_inst_o  ( wb_memory_inst_i ),
	// 	.load_type_i	( lsu_load_type_i	),
	// 	.store_type_i	( lsu_store_type_i	),
	// 	.csr_wdata_i	( lsu_csr_wdata_i	),
	// 	.csr_type_i		( lsu_csr_type_i	),

	// 	.mem_rdata_rare_i( data_r_data_i   	),
	// 	.mem_ren_o   	( mem_ren_o 		),
	// 	.mem_wen_o   	( mem_wen_o   	),
	// 	.mem_wdata_o    ( mem_wdata_o   	),
	// 	.mem_waddr_o    ( mem_waddr_o   		),
	// 	.mem_raddr_o    ( mem_raddr_o 		),
	// 	.mem_wmask_o  	( mem_wmask_o 	),
	// 	.mem_rmask_o	( mem_rmask_o	),

	// 	.ifu_valid     	( ifu_valid_o   	),
	// 	// .wb_ready_o   	( wb_ready_o 		),
	// 	// .lsu_ready_o    ( lsu_ready_o   	),
	// 	.lsu_valid_o    ( lsu_valid_o   	),
	// 	.wd_o     		( wb_reg_wen_i   		),
	// 	.wreg_o   		( wb_wreg_i 		),
	// 	.wdata_o  		( wb_reg_wdata_i 	),
	// 	.csr_type_o		( wb_csr_type_i	),
	// 	.csr_wdata_o    ( wb_csr_wdata_i   	)
	// );

	ysyx_22041211_wb#(
		.DATA_LEN     ( 32 )
	)ysyx_22041211_wb(
		.reset          ( reset          ),
		.wd_i         ( wb_reg_wen_i ),
		.clock          ( clock          ),
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
	)ysyx_22041211_CSR(
		.clock           ( clock           ),
		.reset           ( reset           ),
		.csr_addr      ( csr_addr_i      ),
		.wdata         ( csr_wdata_i         ),
		.csr_type_i    ( csr_type_i    ),
		.csr_mepc_i    ( csr_mepc_i    ),  
		.csr_mcause_i  ( csr_mcause_i  ),
		.csr_pc_o      ( if_csr_pc_i      ),
		.r_data        ( ex_csr_rdata_i     )
	);

endmodule
