/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/

module ysyx_22041211_add #(parameter DATA_LEN = 32)(
	input		[DATA_LEN - 1:0]		data1,
	input		[DATA_LEN - 1:0]		data2,
	input								en,
	input								rst,
	input								clk,


	output		[DATA_LEN - 1:0]		result
);
	ysyx_22041211_Reg #(DATA_LEN) my_Reg (clk,rst,data1 + data2,en,result);
	//ysyx_22041211_Reg #(WIDTH = 1)(clk,1'b0,(data1[DATA_LEN - 1] == data2[DATA_LEN - 1] && result[DATA_LEN - 1] != data1[DATA_LEN - 1]),en,overflow);


endmodule

