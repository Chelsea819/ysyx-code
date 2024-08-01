module ysyx_22041211_RAM #(parameter ADD_LEN = 32,DATA_LEN = 32)(
	input										w_en	,
	input		[ADDR_LEN - 1:0]				addr	,
	input		[DATA_LEN - 1:0]				data	,
	output	    [DATA_LEN - 1:0]				dout	
);
	wire			[DATA_LEN - 1:0]		ram_data;
	assign ram_data[addr] = data & {DATA_LEN{w_en}};
	assign dout = ram_data[addr];

endmodule
