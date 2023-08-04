module ysyx_22041211_RAM(
	input										clk		,
	input										w_en	,
	input		[ADDR_LEN - 1:0]				addr	,
	input		[DATA_LEN - 1:0]				data	,
	output reg  [DATA_LEN - 1:0]				dout	
);
	ysyx_22041211_Reg #()()
