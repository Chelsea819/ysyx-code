/*************************************************************************
	> File Name: ysyx_22041211_top.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月06日 星期日 16时00分47秒
 ************************************************************************/

/* verilator lint_off UNUSEDSIGNAL */

module ysyx_22041211_top #(parameter DATA_LEN = 32,ADDR_LEN = 32)(
	input								clk ,
	input								rst	,
	input	reg		[DATA_LEN - 1:0]	inst,
	output			[ADDR_LEN - 1:0]	pc	
	
		
);
	//wire			[ADDR_LEN - 1:0]	snpc    ;
	wire	    	[DATA_LEN - 1:0]	imm		;	
   	wire      		[4:0]               rd		;
    wire      		[4:0]               rsc1	;
    wire      		[4:0]               rsc2	;
	wire			[DATA_LEN - 1:0]	src1	;
	wire			[DATA_LEN - 1:0]	src2	;
	wire								en		;	
	//wire            [2:0]               alu_control	;
    wire                              	regWrite	;
	wire								branch;
	//wire			[DATA_LEN - 1:0]	result	;
	wire      		[2:0]               key		;
	wire								alu_srcA;
	wire			[1:0]				alu_srcB;
	wire			[DATA_LEN - 1:0]	reg_data1;
	wire			[DATA_LEN - 1:0]	reg_data2;
	// wire			                	mem_toReg;
	// wire			                	mem_write;
	// wire			[DATA_LEN - 1:0]	data_mem;
	wire			[DATA_LEN - 1:0]	alu_result;
	wire 			[DATA_LEN - 1:0]	pc_src2;
	wire			[DATA_LEN - 1:0]	pc_before;
	wire			[DATA_LEN - 1:0]	pc_after;
	wire			[DATA_LEN - 1:0]	pc_src1;
	wire								pc_choose1;	
	// wire			[DATA_LEN - 1:0]	w_reg_data;

	assign pc = pc_before;


	ysyx_22041211_counter my_counter(
		.clk	(clk),
		.rst	(rst),
		.pc_new	(pc_after),
		.pc	(pc_before)

	);

	ysyx_22041211_MuxKey #(2,1,32) PC_chosing (pc_src2, branch, {
		1'b0, 32'b100,
		1'b1, imm
	});
	

	ysyx_22041211_pcPlus my_pcPlus(
		.pc_old	(pc_src1),
		.src	(pc_src2),
		.key	(pc_choose1),
		.pc_new	(pc_after)
	);

	ysyx_22041211_MuxKey #(2,1,32) pcSrcChosing (pc_src1, pc_choose1, {
		1'b0, reg_data1,
		1'b1, pc_before
	});

	// import "DPI-C" context function void get_inst();
    // always @(posedge clk)
    //     dpi_inst();

    // task dpi_inst();  // 在任务中使用 input reg 类型
    //     /* verilator no_inline_task */
    //     get_inst();
    // endtask


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
		.r_data1	(reg_data1),
		.r_data2	(reg_data2)
	);


	ysyx_22041211_MuxKey #(2,1,32) my_srcAchosing (src1, alu_srcA, {
		1'b0, reg_data1,
		1'b1, pc
	});


	ysyx_22041211_MuxKey #(3,2,32) my_srcBchosing (src2, alu_srcB, {
		2'b00, reg_data2,
		2'b01, imm,
		2'b10, 32'b100
	});


	ysyx_22041211_controller my_controller(
		//.inst			({inst[14:12],inst[6:0]}),
		.inst			(inst),
		.key			(key),
		//.alu_control	(alu_control),
		.add_en			(en),
		.regWrite		(regWrite),
		.pc_choose1		(pc_choose1),
		// .mem_toReg		(mem_toReg),
		// .mem_write		(mem_write),
		.branch			(branch),
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

/* verilator lint_on UNUSEDSIGNAL */
