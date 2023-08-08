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
	wire			[DATA_LEN - 1:0]	scr1	;
	wire			[DATA_LEN - 1:0]	scr2	;	
	wire                              	add_en	;
    wire                              	reg_wen	;
	wire			[DATA_LEN - 1:0]	result	;
	wire      		[2:0]               key		;

	assign pc = pc_temp;
	assign result1 = scr2;

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
		.clk	(clk),
		.wdata	(result),
		.rd		(rd),
		.rsc1	(rsc1),
		.rsc2	(rsc2),
		.wen	(reg_wen),
		.rst	(rst),
		.r_data1(scr1),
		.r_data2(scr2)
	);
	

	ysyx_22041211_controller my_controller(
		.inst	({inst[14:12],inst[6:0]}),
		.key	(key),
		.add_en	(add_en),
		.reg_wen(reg_wen)
	);

	ysyx_22041211_add my_add(
		.data1		(scr1),
		.data2		(imm),
		.en			(add_en),
		.rst		(rst),
		.result		(result)
	);


endmodule
