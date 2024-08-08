module ysyx_22041211_wb #(parameter DATA_LEN = 32)(
    input		                		wd_i		,
    input		[4:0]		            wreg_i		,
    input		[DATA_LEN - 1:0]		wdata_i     ,
    output		                		wd_o		,
    output		[4:0]		            wreg_o		,
    output		[DATA_LEN - 1:0]		wdata_o
);
    assign wd_o = wd_i;
    assign wreg_o = wreg_i;
    assign wdata_o = wdata_i;

endmodule
