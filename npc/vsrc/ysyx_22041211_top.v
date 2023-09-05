/*************************************************************************
	> File Name: ysyx_22041211_top.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月06日 星期日 16时00分47秒
 ************************************************************************/

module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	input			[DATA_LEN - 1:0]	inst,
	output			[ADDR_LEN - 1:0]	pc	,
	output			[DATA_LEN - 1:0]	result1
	
		
);
	wire 			[ADDR_LEN - 1:0]	pc_temp ;
	wire	    	[DATA_LEN - 1:0]	imm		;	
   	wire      		[4:0]               rd		;
    wire      		[4:0]               rsc1	;
    wire      		[4:0]               rsc2	;
	wire			[DATA_LEN - 1:0]	src1	;
	wire			[DATA_LEN - 1:0]	src2	;
	wire								en		;	
	//wire            [2:0]               alu_control	;
    wire                              	regWrite	;
	//wire			[DATA_LEN - 1:0]	result	;
	wire      		[2:0]               key		;
	wire								alu_srcA;
	wire								alu_srcB;
	wire			[DATA_LEN - 1:0]	reg_data1;
	wire			[DATA_LEN - 1:0]	reg_data2;
	// wire			                	mem_toReg;
	// wire			                	mem_write;
	// wire			[DATA_LEN - 1:0]	data_mem;
	wire			[DATA_LEN - 1:0]	alu_result;
	// wire			[DATA_LEN - 1:0]	w_reg_data;

	assign pc = pc_temp;
	assign result1 = src2;

	ysyx_22041211_counter my_counter(
		.clk	(clk),
		.rst	(rst),
		.pc_old	(pc_temp),
		.pc_new	(pc_temp)

	);

	ysyx_22041211_Decode my_Decode(
		.inst	(inst),
		.imm	(imm),
		.rd		(rd),
		.clk	(clk),
		.rsc1	(rsc1),
		.key	(key),
		.rsc2	(rsc2)
	);

	ysyx_22041211_RegisterFile my_RegisterFile(
		.clk		(clk),
		//.wdata		(w_reg_data),
		.wdata		(alu_result),
		.rd			(rd),
		.rsc1		(rsc1),
		.rsc2		(rsc2),
		.regWrite	(regWrite),
		.rst		(rst),
		.r_data1	(reg_data1),
		.r_data2	(reg_data2)
	);

	ysyx_22041211_ALUsrc my_srcA_chosing(
		.key		(alu_srcA),
		.data1		(pc),
		.data0		(reg_data1),
		.src		(src1)
	);

	ysyx_22041211_ALUsrc my_srcB_chosing(
		.key		(alu_srcB),
		.data1		(imm),
		.data0		(reg_data2),
		.src		(src2)
	);
	

	ysyx_22041211_controller my_controller(
		.inst			({inst[14:12],inst[6:0]}),
		.key			(key),
		//.alu_control	(alu_control),
		.add_en			(en),
		.regWrite		(regWrite),
		// .mem_toReg		(mem_toReg),
		// .mem_write		(mem_write),
		.alu_srcA		(alu_srcA),
		.alu_srcB		(alu_srcB)
	);

	ysyx_22041211_ALU my_add(
		.src1		(src1),
		.src2		(src2),
		.en			(en),
		.rst		(rst),
		.result		(alu_result)
		//.pc			(pc)
	);

	// ysyx_22041211_mem_toReg mem_toReg(
	// 	.mem_toReg	(mem_toReg),
	// 	.data_mem	(data_mem),
	// 	.alu_result	(alu_result),
	// 	.w_reg_data	(w_reg_data)
	// );

	// ysyx_22041211_dataMemory dataMemory(
	// 	.alu_result		(alu_result),
	// 	.mem_write		(mem_write),
	// 	.data_mem		()
	// )


endmodule
