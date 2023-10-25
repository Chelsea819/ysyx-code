module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	input	reg		[DATA_LEN - 1:0]	inst,
	input	        [DATA_LEN - 1:0]    WriteData	,
	output			[DATA_LEN - 1:0]	pc			,
	output			[DATA_LEN - 1:0]	ALUResult	,
    output  		[DATA_LEN - 1:0]	ReadData	,    
	output 			[1:0]				DataLen 	,
	output								DataSign	,						
	output								memToReg	
);
	//my_counter
	wire			[DATA_LEN - 1:0]	pc_next		;
	wire			[DATA_LEN - 1:0]	pcPlus		;
	wire			[DATA_LEN - 1:0]	pcBranch	;
	wire								pcSrc		;

	//registerFile
	wire			[DATA_LEN - 1:0]	reg_data1	;
	wire			[DATA_LEN - 1:0]	reg_data2	;

	//control
	//wire								memToReg	;
	wire								memWrite	;
	wire								branch		;
	wire			[3:0]				ALUcontrol	;
	wire								ALUSrc		;
	wire								regWrite	;

	//immGet
	wire			[DATA_LEN - 1:0]	imm			;

	//ALU
	//wire			[DATA_LEN - 1:0]	srcA		;
	wire			[DATA_LEN - 1:0]	srcB		;
	wire								zero	 	;

	//ALUSrc
	wire								ALUSrc		;

	//dataMem


	assign pcSrc = branch & zero;		

	// 检测到ebreak
    import "DPI-C" context function void ifebreak_func(int inst);
    always @(posedge clk)
        dpi_inst(inst);

    task dpi_inst(input reg [31:0] inst);  // 在任务中使用 input reg 类型
        /* verilator no_inline_task */
        ifebreak_func(inst);
    endtask
	
	ysyx_22041211_MuxKey #(2,1,32) PCSrc_choosing (pc_next ,pcSrc ,{
		1'b1, pcBranch,
		1'b0, pcPlus
	});

	ysyx_22041211_counter my_counter(
		.clk		(clk),
		.rst		(rst),
		.pc_next	(pc_next),
		.pc			(pc)
	);

	ysyx_22041211_pcPlus my_pcPlus(
		.pc_old	(pc),
		.pc_new	(pc_next)
	);

	ysyx_22041211_immGet my_immGet(
		.inst		(inst),
		.imm		(imm)
	);

	ysyx_22041211_RegisterFile my_RegisterFile(
		.clk		(clk),
		.wdata		(WriteData),
		.rd			(inst[11:7]),
		.rsc1		(inst[19:15]),
		.rsc2		(inst[24:20]),
		.rst		(rst)		 ,
		.regWrite	(regWrite),
		.r_data1	(reg_data1),
		.r_data2	(reg_data2)
	);

	ysyx_22041211_controller my_controller(
		.opcode		(inst[6:0]),
		.funct3		(inst[14:12]),
		.funct7		(inst[31:25]),
		.memToReg	(memToReg),
		.memWrite	(memWrite),
		.branch		(branch),
		.ALUcontrol	(ALUcontrol),
		.regWrite	(regWrite),
		.ALUSrc		(ALUSrc)
	);

	ysyx_22041211_pcPlusBranch my_pcPlusBranch (
		.offset		(imm),
		.pc_old		(pc),
		.pcBranch	(pcBranch)
	);
	
	//assign srcA = reg_data1;

	ysyx_22041211_ALU my_ALU(
		.src1		(reg_data1),
		.src2		(srcB),
		.alu_control(ALUControl),
		.result		(ALUResult),
		.zero		(zero)
	);

	ysyx_22041211_MuxKey #(2,1,32) ALUSrc_choosing (srcB,ALUSrc, {
		1'b1, imm,
		1'b0, reg_data2
	});

	ysyx_22041211_MuxKey #(2,1,32) memToReg_choosing (WriteData, memToReg, {
		1'b0, ALUResult,
		1'b1, ReadData
	});



endmodule