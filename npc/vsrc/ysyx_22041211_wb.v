module ysyx_22041211_wb #(parameter DATA_LEN = 32)(
    input		                		wd_i		,
    input		[4:0]		            wreg_i		,
    input		[DATA_LEN - 1:0]		alu_result_i     ,
    output		                		wd_o		,
    output		[4:0]		            wreg_o		,
    output		[DATA_LEN - 1:0]		wdata_o
);
    assign wd_o = wd_i;
    assign wreg_o = wreg_i;
    assign wdata_o = alu_result_i;
//访存指令
	// wire [7:0]	wmask;
	// assign wmask = ((DataLen == 3'b001)? 8'b00000001: 
	// 			   (DataLen == 3'b010)? 8'b00000011:
	// 			   (DataLen == 3'b100)? 8'b00001111: 8'b11111111);
	// always @(*) begin
  	// 	if ((memToReg[0] & ~memToReg[1])) // 有读写请求时
   	// 		ReadData = pmem_read_task(ALUResult, wmask);
  	// 	else
    // 		ReadData = 0;
	// end

	// always @(*) begin
  	// 	if (memWrite) // 有写请求时
    //   		pmem_write_task(ALUResult, reg_data2, wmask);
	// end
endmodule
