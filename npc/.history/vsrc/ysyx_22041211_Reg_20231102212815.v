/*************************************************************************
	> File Name: ysyx_22041211_Reg.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 17时28分28秒
 ************************************************************************/

module ysyx_22041211_Reg #(parameter WIDTH = 1, RESET_VAL = 0)(
	input							clk	,
	input							rst	,
	input			[WIDTH - 1:0]	din	,
	input							wen	,
	output	reg		[WIDTH - 1:0]	dout
);
	always @(posedge clk or posedge rst) begin
		if (rst) dout <= RESET_VAL;
		else if (wen) dout <= din;
	end
	
endmodule


