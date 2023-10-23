/*************************************************************************
	> File Name: ysyx_22041211_SignExten.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 17时28分28秒
 ************************************************************************/

module ysyx_22041211_SignExten #(parameter WIDTH = 1, RESET_VAL = 0)(
	input							data	,
	output	 		[WIDTH - 1:0]	SignImm
);
	always @(posedge clk) begin
		if (rst) dout <= RESET_VAL;
		else if (wen) dout <= din;
	end
endmodule


