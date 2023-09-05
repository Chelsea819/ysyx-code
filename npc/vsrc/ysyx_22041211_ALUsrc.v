
module ysyx_22041211_ALUsrc #(parameter DATA_LEN = 32)(
	input		                		alu_src,
	input		[DATA_LEN - 1:0]		imm,
    input		[DATA_LEN - 1:0]		reg_data,

	output		[DATA_LEN - 1:0]		src2
);

	ysyx_22041211_MuxKeyWithDefault #(2, 1, 32) choose_src2 (src2, alu_src, {DATA_LEN{1'b0}},{
        1'b1 , imm,
        1'b0 , reg_data 
    });


endmodule

