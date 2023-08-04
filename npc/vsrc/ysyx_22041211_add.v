/*************************************************************************
	> File Name: ysyx_22041211_add.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时24分15秒
 ************************************************************************/

module ysyx_22041211_add (
	input		[DATA_LEN - 1:0]		data1,
	input		[DATA_LEN - 1:0]		data2,
	input								carry_in,

	output		[DATA_LEN - 1:0]		result,
	output								overflow,
	output								carry_out
);
	assign {carry_out,result} = data1 + data2 + carry_in;
	assign overflow = (data1[DATA_LEN - 1] == data2[DATA_LEN - 1] && result[DATA_LEN - 1] != data1[DATA_LEN - 1])


