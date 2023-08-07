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

	output		[DATA_LEN - 1:0]		result
);
	assign	result = {DATA_LEN{~rst}} & ( data1 + data2 ) & {DATA_LEN{en}};
	


endmodule

