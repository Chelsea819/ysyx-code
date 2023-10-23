module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	input	reg		[DATA_LEN - 1:0]	inst
	
);
	wire			[DATA_LEN - 1:0]	pc_next		;
	wire			[DATA_LEN - 1:0]	pc			;
	wire			[DATA_LEN - 1:0]	reg_data1	;
	wire			[DATA_LEN - 1:0]	imm			;
	wire			[DATA_LEN - 1:0]	ReadData	;
	wire								zero	 	;	
	wire			[DATA_LEN - 1:0]	ALUResult	;
	wire			[DATA_LEN - 1:0]	ALUController;


	ysyx_22041211_counter my_counter(
		.clk		(clk),
		.rst		(rst),
		.pc_next	(pc_next),
		.pc			(pc)
	);

	












endmodule