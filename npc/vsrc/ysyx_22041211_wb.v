module ysyx_22041211_wb #(parameter DATA_LEN = 32)(
    input		                		wd_i		,
    input		[4:0]		            wreg_i		,
    input		[DATA_LEN - 1:0]		alu_result_i,
    input		                		mem_wen_i	,
	input		[DATA_LEN - 1:0]		mem_wdata_i	,
    output		                		wd_o		,
    output		[4:0]		            wreg_o		,
    output		[DATA_LEN - 1:0]		wdata_o
);
    wire [31:0] mem_waddr;
    assign wd_o = wd_i;
    assign wreg_o = wreg_i;
    assign wdata_o = alu_result_i;
    assign mem_waddr = alu_result_i;
//访存指令
	// always @(*) begin
  	// 	if (mem_wen_i) // 有读写请求时
   	// 		ReadData = pmem_read_task(ALUResult, wmask);
  	// 	else
    // 		ReadData = 0;
	// end
	import "DPI-C" function void pmem_write_task(input int waddr, input int wdata);
	always @(*) begin
  		if (mem_wen_i) // 有写请求时
      		pmem_write_task(mem_waddr, mem_wdata_i);
	end
endmodule
