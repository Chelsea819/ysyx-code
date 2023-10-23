
module ysyx_22041211_dataMemory #(parameter DATA_LEN = 32, ADDR_LEN = 32,RESET_VAL = 32'h80000000)(
	input                               clk         ,
    input                               rst         ,
    input                               memToReg    , 
    input	[DATA_LEN - 1:0]			ALUResult	,
    input	[DATA_LEN - 1:0]            WriteData	,
    output  [DATA_LEN - 1:0]			ReadData    	
);
    

    
	
endmodule
