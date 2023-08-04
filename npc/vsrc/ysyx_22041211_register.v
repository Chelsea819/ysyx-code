/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/

module ysyx_22041211_register (
	input								clk	,
	input								rst	,
	input		[DATA_LEN - 1:0]		din	,
	input								wen ,
	output	reg [DATA_LEN - 1:0]		dout
);


