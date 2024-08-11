`include "./ysyx_22041211_define.v"
module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32) (
	// input								clk ,
	// input								rst	,
	// // input			[DATA_LEN - 1:0]	id_inst_i,
	// output			[ADDR_LEN - 1:0]	pc			,
	// output								invalid
	input								clk ,
	input								rst ,
	// input			[DATA_LEN - 1:0]	id_inst_i,
	output			[ADDR_LEN - 1:0]	pc			,
	output								invalid
);

	// //registerFile
	// wire								reg_re1_i		;
	wire			[4:0]				reg_raddr1_i	;
	// wire								reg_re2_i		;
	wire			[4:0]				reg_raddr2_i	;
	wire								reg_wen_i		;
	wire			[4:0]				reg_waddr_i		;
	wire	        [DATA_LEN - 1:0]    reg_wdata_i		;

	//my_counter
	wire			[ADDR_LEN - 1:0]	if_pc_plus_4	;
	wire			[ADDR_LEN - 1:0]	if_branch_target_i;
	wire			[2:0]				if_branch_type_i;
	wire								if_branch_request_i;	
	wire			[ADDR_LEN - 1:0]	if_jmp_target_i;
	wire								if_jmp_flag_i;
	// wire			[ADDR_LEN - 1:0]	pcPlus		;
	// wire			[ADDR_LEN - 1:0]	pcBranch	;
	// wire			[1:0]				pcSrc		;

	//my_ifetch
	reg  			[DATA_LEN - 1:0]    if_inst			;

	//my_decoder
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
	// wire			[DATA_LEN - 1:0]	ex_inst_i		;
	wire			[DATA_LEN - 1:0]	ex_pc_i			;
	wire								ex_wd_i			;
	wire			[4:0]				ex_wreg_i		;
	wire			[1:0]				ex_store_type_i	;
	wire			[2:0]				ex_load_type_i	;

	// wb Unit
	wire			[DATA_LEN - 1:0]	wb_mem_wdata_i	;
	wire								wb_mem_wen_i	;
	wire								wb_wd_i			;
	wire			[4:0]				wb_wreg_i		;
	wire			[DATA_LEN - 1:0]	wb_alu_result_i	;
	wire			[2:0]				wb_load_type_i	;
	
	assign pc = id_pc_i;
	assign invalid = ~((id_inst_i[6:0] == `TYPE_U_LUI_OPCODE) | (id_inst_i[6:0] == `TYPE_U_AUIPC_OPCODE) | //U-auipc lui
					 (id_inst_i[6:0] == `TYPE_J_JAL_OPCODE) | 	 					     //jal
				     ({id_inst_i[14:12], id_inst_i[6:0]} == {`TYPE_I_JALR_FUNC3, `TYPE_I_JALR_OPCODE}) |			 //I-jalr
					 ({id_inst_i[6:0]} == `TYPE_B_OPCODE) |			 //B-beq
					 ((id_inst_i[6:0] == `TYPE_I_LOAD_OPCODE) & (id_inst_i[14:12] == `TYPE_I_LB_FUNC3 | id_inst_i[14:12] == `TYPE_I_LH_FUNC3 | id_inst_i[14:12] == `TYPE_I_LW_FUNC3 | id_inst_i[14:12] == `TYPE_I_LBU_FUNC3 | id_inst_i[14:12] == `TYPE_I_LHU_FUNC3)) |	 //I-lb lh lw lbu lhu
					 ((id_inst_i[6:0] == `TYPE_S_OPCODE) & (id_inst_i[14:12] == `TYPE_S_SB_FUNC3 | id_inst_i[14:12] == `TYPE_S_SH_FUNC3 | id_inst_i[14:12] == `TYPE_S_SW_FUNC3))	|		//S-sb sh sw
					 ((id_inst_i[6:0] == `TYPE_I_BASE_OPCODE) & (id_inst_i[14:12] == `TYPE_I_ADDI_FUNC3)) |	 //I-addi slti sltiu xori ori andi
					//  ((id_inst_i[6:0] == 7'b0010011) & ((id_inst_i[14:12] == 3'b001 && id_inst_i[31:26] == 6'b000000) | (id_inst_i[14:12] == 3'b101 && (id_inst_i[31:26] == 6'b000000 || id_inst_i[31:26] == 6'b010000)) )) |	 //I-slli srli srai
					 (id_inst_i[6:0] == `TYPE_R_OPCODE) | //R
					 (id_inst_i == `TYPE_I_EBREAK));
	// 检测到ebreak
    import "DPI-C" function void ifebreak_func(int id_inst_i);
    always @(*)
        ifebreak_func(id_inst_i);

	import "DPI-C" function int pmem_read_task(input int raddr, input byte wmask);
	import "DPI-C" function void pmem_write_task(input int waddr, input int wdata);
	
	//取指令
	always @(*) begin
		if(~rst) begin
        	if_inst = pmem_read_task(id_pc_i, 8'b00001111);
		end
		else  begin
			if_inst = 32'b0;
		end
	end

	always @(*) begin
			$display("pc: [%h] inst: [%b] invalid: [%h]",id_pc_i, if_inst, invalid);
	end
	assign id_inst_i = if_inst;

	ysyx_22041211_counter#(
		.ADDR_LEN         ( 32 )
	)u_ysyx_22041211_counter(
		.clk              ( clk              ),
		.rst              ( rst              ),
		.branch_request_i ( if_branch_request_i ),
		.branch_target_i  ( if_branch_target_i  ),
		.branch_flag_i    ( |if_branch_type_i    ),
		.jmp_flag_i  	  ( if_jmp_flag_i  ),
		.jmp_target_i     ( if_jmp_target_i    ),
		.pc_plus_4        ( if_pc_plus_4     ),
		.pc               ( id_pc_i               )
	);


	ysyx_22041211_pcPlus#(
		.DATA_LEN ( 32 )
	)u_ysyx_22041211_pcPlus(
		.pc_old ( id_pc_i ),
		.rst    ( rst    ),
		.pc_new  ( if_pc_plus_4  )
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
		.inst_i							(id_inst_i),
		.reg1_data_i					(id_reg1_data_i),
		.reg2_data_i					(id_reg2_data_i),
		.pc_i       					(id_pc_i),	
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
		// .inst_o     					(ex_inst_i),
		.imm_o      					(ex_imm_i)
	);

	ysyx_22041211_EXE my_execute(
		.reg1_i				(ex_reg1_i),
		.reg2_i				(ex_reg2_i),
		.pc_i				(ex_pc_i),
		// .inst			(ex_inst_i),
		.alu_control		(ex_aluop_i),
		.alu_sel			(ex_alusel_i),		
		.imm_i				(ex_imm_i),
		.wd_i				(ex_wd_i),	
		.wreg_i				(ex_wreg_i),
		.branch_type_i		(if_branch_type_i),	
		.store_type_i		(ex_store_type_i),	
		.load_type_i		(ex_load_type_i),
		.branch_request_o	(if_branch_request_i),
		.wd_o				(wb_wd_i),	
		.wreg_o				(wb_wreg_i),
		.mem_wen_o			(wb_mem_wen_i),	
		.mem_wdata_o		(wb_mem_wdata_i),	
		.load_type_o		(wb_load_type_i),
		.alu_result_o		(wb_alu_result_i)
	);

	ysyx_22041211_wb #(
		.DATA_LEN ( 32 )
	)u_ysyx_22041211_wb(
		.wd_i     		( wb_wd_i     ),
		.wreg_i   		( wb_wreg_i   ),
		.alu_result_i   ( wb_alu_result_i  	),
		.mem_wen_i     	( wb_mem_wen_i   ),
		.mem_wdata_i   	( wb_mem_wdata_i ),
		.load_type_i	(wb_load_type_i),
		.wd_o     		( reg_wen_i   ),
		.wreg_o   		( reg_waddr_i ),
		.wdata_o  		( reg_wdata_i )
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
