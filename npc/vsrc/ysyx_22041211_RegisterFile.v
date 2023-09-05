/*************************************************************************
	> File Name: ysyx_22041211_register.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月04日 星期五 18时19分21秒
 ************************************************************************/

module ysyx_22041211_RegisterFile #(parameter ADDR_WIDTH = 32, DATA_WIDTH = 32)(
	input								clk		,
	input		[DATA_WIDTH - 1:0]		wdata	,
	input	    [4:0]					rsc1	,
	input	    [4:0]					rsc2	,
	input	    [4:0]					rd		,
	input								regWrite,
	input								rst		,
	output		[DATA_WIDTH - 1:0]		r_data1	,
	output		[DATA_WIDTH - 1:0]		r_data2	
);
	reg [DATA_WIDTH - 1:0] rf [ADDR_WIDTH - 1:0];
	always @(posedge clk) begin
		if (regWrite) rf[rd] <= wdata;
	end
	assign rf[0] = 0;

	//读取操作数
	ysyx_22041211_Reg #(DATA_WIDTH) my_Reg1 (clk,rst,rf[rsc1],regWrite,r_data1);
	ysyx_22041211_Reg #(DATA_WIDTH) my_Reg2 (clk,rst,rf[rsc2],regWrite,r_data2);
endmodule
