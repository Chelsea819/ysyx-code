
module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	input			[DATA_LEN - 1:0]	inst,
	// input  			[DATA_LEN - 1:0]	ReadData	, //no
	output			[ADDR_LEN - 1:0]	pc			,
	// output			[DATA_LEN - 1:0]	ALUResult	,
	// output    		[DATA_LEN - 1:0]	storeData	,
	// output 			[1:0]				DataLen 	,  // 0 1 3
	output								memWrite	,						
	output			[1:0]				memToReg	,
	output								invalid
);
	//my_counter
	wire			[ADDR_LEN - 1:0]	pc_tmp		;
	wire			[ADDR_LEN - 1:0]	pc_next		;
	wire			[ADDR_LEN - 1:0]	pcPlus		;
	wire			[ADDR_LEN - 1:0]	pcBranch	;
	wire			[1:0]				pcSrc		;

	//registerFile
	wire			[DATA_LEN - 1:0]	reg_data1	;
	wire			[DATA_LEN - 1:0]	reg_data2	;
	wire	        [DATA_LEN - 1:0]    WriteData	;

	//control
	wire			[1:0]				memToReg_tmp;	
	wire								branch		;
	wire			[3:0]				ALUcontrol	;
	wire								ALUSrc		;
	wire								regWrite	;
	wire			[1:0]				jmp			;

	//immGet
	wire			[DATA_LEN - 1:0]	imm			;

	//ALU
	wire			[DATA_LEN - 1:0]	srcB		;
	wire								zero	 	;
	wire			[DATA_LEN - 1:0]	ALUResult	;

	//ALUSrc
	wire								ALUSrc		;

	//dataMem
	wire			[DATA_LEN - 1:0]	ReadData_tmp	;
	wire 			[7:0]				DataLen			;
	wire								DataSign		;
	reg			[DATA_LEN - 1:0]		ReadData		;
	// wire			[1:0]				DataLen			;	

	
	assign pc = pc_tmp;
	// assign storeData = reg_data2;
	assign memToReg = memToReg_tmp;
	
	// // 做位拓展 ReadData_tmp是处理好的最终读取到的数据
	assign ReadData_tmp = (DataSign == 1'b0 || DataLen == 8'b00000100) ? ReadData : 
						  (DataLen == 8'b00000001) ? {{24{ReadData[7]}}, ReadData[7:0]}:				//0--1 8bits
						  (DataLen == 8'b00000010) ? {{16{ReadData[15]}}, ReadData[15:0]}: 32'b0;    //1--2 16bits

	assign invalid = ~((inst[6:0] == 7'b0010111) | (inst[6:0] == 7'b0110111) | //U-auipc lui
					 (inst[6:0] == 7'b1101111) | 	 					     //jal
				     ({inst[14:12],inst[6:0]} == 10'b0001100111) |			 //I-jalr
					 ({inst[14:12],inst[6:0]} == 10'b0001100011) |			 //B-beq
					 ((inst[6:0] == 7'b0000011) & (inst[14:12] == 3'b000 | inst[14:12] == 3'b001 | inst[14:12] == 3'b010 | inst[14:12] == 3'b100 | inst[14:12] == 3'b101)) |	 //I-lb lh lw lbu lhu
					 ((inst[6:0] == 7'b0100011) & (inst[14:12] == 3'b000 | inst[14:12] == 3'b001 | inst[14:12] == 3'b010))	|		//S-sb sh sw
					 ((inst[6:0] == 7'b0010011) & (inst[14:12] == 3'b000 | inst[14:12] == 3'b010 | inst[14:12] == 3'b011 | inst[14:12] == 3'b100 | inst[14:12] == 3'b110 | inst[14:12] == 3'b111)) |	 //I-addi slti sltiu xori ori andi
					 (inst[6:0] == 7'b0110011) | //R
					 (inst == 32'b00000000000100000000000001110011));

						  

	// 检测到ebreak
    import "DPI-C" context function void ifebreak_func(int inst);
    always @(*)
        dpi_inst(inst);

    task dpi_inst(input [31:0] inst_bnk);  // 在任务中使用 input reg 类型
        /* verilator no_inline_task */
        ifebreak_func(inst_bnk);
    endtask

	//访存指令
	import "DPI-C" function void vaddr_read(input int raddr,input byte DataLen, output int ReadData);
	import "DPI-C" function void vaddr_write(input int waddr, input int wdata, input byte wmask);

	// always @(*) begin
  	// 	if (valid) begin // 有读写请求时
    // 		pmem_read(ALUResult, ReadData);
    // 		if (wen) begin // 有写请求时
    //  			pmem_write(waddr, wdata, DataLen);
    // 		end
  	// 	end
  	// 	else begin
    // 		ReadData = 0;
  	// 	end
	// end

	always @(*) begin
  		if (memToReg[0])  // 有读请求时
    		vaddr_read(ALUResult, DataLen, ReadData);	
  		else 
    		ReadData = 32'b0;
	end

	always @(*) begin
		if (memWrite) begin // 有写请求时
     		vaddr_write(ALUResult, reg_data2, DataLen);
    	end
	end

	// 为ITRACE提供指令
    import "DPI-C" context function void inst_get(int inst);
    always @(*)
        dpi_instGet(inst);

    task dpi_instGet(input [31:0] inst_bnk);  // 在任务中使用 input reg 类型
        /* verilator no_inline_task */
        inst_get(inst_bnk);
    endtask


	
	ysyx_22041211_MuxKey #(3,2,32) PCSrc_choosing (pc_next ,pcSrc ,{
		2'b01, pcBranch,
		2'b00, pcPlus,
		2'b10, ALUResult & ~1
	});

	ysyx_22041211_counter my_counter(
		.clk		(clk),
		.rst		(rst),
		.pc_next	(pc_next),
		.pc			(pc_tmp)
	);

	ysyx_22041211_pcPlus my_pcPlus(
		.pc_old	(pc_tmp),
		.pc_new	(pcPlus)
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
		.func3		(inst[14:12]),
		.func7		(inst[31:25]),
		.memToReg	(memToReg_tmp),
		.memWrite	(memWrite),
		.branch		(branch),
		.jmp		(jmp),
		.ALUcontrol	(ALUcontrol),
		.regWrite	(regWrite),
		.DataLen	(DataLen),
		.DataSign	(DataSign),
		.ALUSrc		(ALUSrc)
	);

	ysyx_22041211_pcPlusBranch my_pcPlusBranch (
		.offset		(imm),
		.pc_old		(pc_tmp),
		.pcBranch	(pcBranch)
	);

	ysyx_22041211_branchJmp my_branchJmp(
		.zero		(zero),
		.branch		(branch),
		.jmp		(jmp),
		.PCSrc		(pcSrc)
	);
	

	ysyx_22041211_ALU my_ALU(
		.src1		(reg_data1),
		.src2		(srcB),
		.alu_control(ALUcontrol),
		.result		(ALUResult),
		.zero		(zero)
	);

	ysyx_22041211_MuxKey #(2,1,32) ALUSrc_choosing (srcB,ALUSrc, {
		1'b1, imm,
		1'b0, reg_data2
	});

	ysyx_22041211_MuxKey #(4,2,32) memToReg_choosing (WriteData, memToReg_tmp, {
		2'b00, ALUResult,
		2'b01, ReadData_tmp	,
		2'b10, pcPlus	,
		2'b11, pcBranch
	});



endmodule
