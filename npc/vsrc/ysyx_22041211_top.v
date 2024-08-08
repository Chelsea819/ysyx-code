module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	// input			[DATA_LEN - 1:0]	inst,
	output			[ADDR_LEN - 1:0]	pc			,
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
	reg			[DATA_LEN - 1:0]	inst		;
	wire			[1:0]				memToReg_tmp;	
	wire			[2:0]				branch		;
	wire			[3:0]				ALUcontrol	;
	wire								ALUSrc		;
	wire								regWrite	;
	wire			[1:0]				jmp			;
	wire								memWrite	;						
	wire			[1:0]				memToReg	;

	//immGet
	wire			[DATA_LEN - 1:0]	imm			;

	//ALU
	wire			[DATA_LEN - 1:0]	srcB		;
	wire								zero	 	;
	wire			[DATA_LEN - 1:0]	ALUResult	;
	// wire								SF			;

	//ALUSrc
	wire								ALUSrc		;

	//dataMem
	wire			[DATA_LEN - 1:0]	ReadData_tmp	;
	wire 			[2:0]				DataLen			;
	wire								DataSign		;
	reg			[DATA_LEN - 1:0]		ReadData		;

	
	assign pc = pc_tmp;
	assign memToReg = memToReg_tmp;
	
	// 做位拓展 ReadData_tmp是处理好的最终读取到的数据
	assign ReadData_tmp = (DataLen == 3'b100) ? ReadData : 
						  (DataSign == 1'b0 && DataLen == 3'b001) ? {{24{1'b0}}, ReadData[7:0]} : 
						  (DataSign == 1'b0 && DataLen == 3'b010) ? {{16{1'b0}}, ReadData[15:0]} : 
						  (DataLen == 3'b001) ? {{24{ReadData[7]}}, ReadData[7:0]}:				//0--1 8bits
						  (DataLen == 3'b010) ? {{16{ReadData[15]}}, ReadData[15:0]}: 32'b0;    //1--2 16bits

	assign invalid = ~((inst[6:0] == 7'b0010111) | (inst[6:0] == 7'b0110111) | //U-auipc lui
					 (inst[6:0] == 7'b1101111) | 	 					     //jal
				     ({inst[14:12],inst[6:0]} == 10'b0001100111) |			 //I-jalr
					 ({inst[6:0]} == 7'b0001100011) |			 //B-beq
					 ((inst[6:0] == 7'b0000011) & (inst[14:12] == 3'b000 | inst[14:12] == 3'b001 | inst[14:12] == 3'b010 | inst[14:12] == 3'b100 | inst[14:12] == 3'b101)) |	 //I-lb lh lw lbu lhu
					 ((inst[6:0] == 7'b0100011) & (inst[14:12] == 3'b000 | inst[14:12] == 3'b001 | inst[14:12] == 3'b010))	|		//S-sb sh sw
					 ((inst[6:0] == 7'b0010011) & (inst[14:12] == 3'b000 | inst[14:12] == 3'b010 | inst[14:12] == 3'b011 | inst[14:12] == 3'b100 | inst[14:12] == 3'b110 | inst[14:12] == 3'b111)) |	 //I-addi slti sltiu xori ori andi
					 ((inst[6:0] == 7'b0010011) & ((inst[14:12] == 3'b001 && inst[31:26] == 6'b000000) | (inst[14:12] == 3'b101 && (inst[31:26] == 6'b000000 || inst[31:26] == 6'b010000)) )) |	 //I-slli srli srai
					 (inst[6:0] == 7'b0110011) | //R
					 (inst == 32'b00000000000100000000000001110011));
	// 检测到ebreak
    import "DPI-C" context function void ifebreak_func(int inst);
    always @(*)
        ifebreak_func(inst);

	import "DPI-C" function int pmem_read_task(input int raddr, input byte wmask);
	import "DPI-C" function void pmem_write_task(input int waddr, input int wdata, input byte wmask);

	//取指令
	always @(posedge clk) begin
        inst <= pmem_read_task(pc_next, 8'b00001111);
		$display("pc: [%h] inst: [%h]",pc_next, inst);
	end

	// wire	[31:0]	inst_pc;
	// assign inst_pc = ((pc_tmp < 32'h80000000) ? 32'h80000000 : pc_next);

	//访存指令
	wire [7:0]	wmask;
	assign wmask = ((DataLen == 3'b001)? 8'b00000001: 
				   (DataLen == 3'b010)? 8'b00000011:
				   (DataLen == 3'b100)? 8'b00001111: 8'b11111111);
	always @(*) begin
  		if ((memToReg[0] & ~memToReg[1])) // 有读写请求时
   			ReadData = pmem_read_task(ALUResult, wmask);
  		else
    		ReadData = 0;
	end

	always @(*) begin
  		if (memWrite) // 有写请求时
      		pmem_write_task(ALUResult, reg_data2, wmask);
	end

	// 为ITRACE提供指令
    import "DPI-C" context function void inst_get(int inst);
    always @(*)
        inst_get(inst);
	
	ysyx_22041211_MuxKeyWithDefault #(3,2,32) PCSrc_choosing (pc_next ,pcSrc ,32'h80000000,{
		2'b01, pcBranch,
		2'b00, pcPlus,
		2'b10, ALUResult & ~1
	});

	// assign pc_next = ((pc_tmp < 32'h80000000) ? 32'h80000000 :
	// 				 (pcSrc == 2'b01) ? pcBranch :
	// 				 (pcSrc == 2'b00) ? pcPlus :
	// 				 (pcSrc == 2'b10) ? ALUResult & ~1 : 32'h80000000);

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
		.shamt_F	(inst[31:26]),
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
		.SF			(ALUResult[0]),
		.branch		(branch),
		.invalid	(invalid),
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
