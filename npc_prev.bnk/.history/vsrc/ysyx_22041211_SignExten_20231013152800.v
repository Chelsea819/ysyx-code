/*************************************************************************
	> File Name: ysyx_22041211_SignExten.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 17时28分28秒
 ************************************************************************/

module ysyx_22041211_SignExten #(parameter WIDTH = 32, RESET_VAL = 0)(
	input			[15:0]			data	,
	output	 		[WIDTH - 1:0]	SignImm
);
	assign SignImm = {{16{data[15]}},data};
endmodule


