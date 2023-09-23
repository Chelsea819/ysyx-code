
module ysyx_22041211_choose #(parameter DATA_LEN = 32)(
	input		                		key,
	input		[DATA_LEN - 1:0]		data1,
    input		[DATA_LEN - 1:0]		data0,

	output		[DATA_LEN - 1:0]		src
);

	// ysyx_22041211_MuxKeyWithDefault #(2, 1, 32) choose_src2 (src, key, {DATA_LEN{1'b0}},{
    //     1'b1 , data1,
    //     1'b0 , data0 
    // });

    assign src = key == 1'b1 ? data1 : data0;



endmodule

