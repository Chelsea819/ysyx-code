module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	input	reg		[DATA_LEN - 1:0]	inst,
	input			[DATA_LEN - 1:0]	ALUResult	,
    input	        [DATA_LEN - 1:0]    WriteData	,
    output  		[DATA_LEN - 1:0]	ReadData    
	
);
	//my_counter
	wire			[DATA_LEN - 1:0]	pc_next		;
	wire			[DATA_LEN - 1:0]	pc			;


	//registerFile
	wire			[DATA_LEN - 1:0]	reg_data1	;
	wire			[DATA_LEN - 1:0]	reg_data2	;

	//sign
	wire			[DATA_LEN - 1:0]	imm			;

	//ALU
	wire			[DATA_LEN - 1:0]	srcA		;
	wire			[DATA_LEN - 1:0]	srcB		;
	wire								zero	 	;	
	wire			[3:0]				ALUController;
	wire			[DATA_LEN - 1:0]	ALUResult	;

	//ALUSrc
	wire								ALUSrc		;
	
	//dataMem
	wire 			[DATA_LEN - 1:0]	WriteData	;	
	wire								regWrite	;	
	

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

	ysyx_22041211_SignExten my_SignExt(
		.data		(inst[15:0]),
		.SignImm	(imm)
	);

	ysyx_22041211_RegisterFile my_RegisterFile(
		.clk		(clk),
		.wdata		(WriteData),
		.rd			(inst[11:7]),
		.rsc1		(inst[19:15]),
		.rsc2		(inst[24:20]),
		.regWrite	(regWrite),
		.r_data1	(reg_data1),
		.r_data2	(reg_data2)
	);
	
	ysyx_22041211_ALU my_ALU(
		.src1		(),
		.src2		(),
		.alu_control(),
		.result		()

	);

	ysyx_22041211_MuxKey #(2,1,32) ALUSrc_choosing (src2,ALUSrc,);





endmodule