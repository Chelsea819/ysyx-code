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
	input								rst		,
	input								regWrite,
	output		[DATA_WIDTH - 1:0]		r_data1	,
	output		[DATA_WIDTH - 1:0]		r_data2	
);
	reg [DATA_WIDTH - 1:0] rf [ADDR_WIDTH - 1:0];

	always @(*) begin
		if (regWrite && rd != 0) rf[rd] = wdata & (~{32{rst}});
		else if(regWrite && rd == 0) rf[rd] <= 0;
	end

	//读取操作数
	assign r_data1 = rf[rsc1];
	assign r_data2 = rf[rsc2];

endmodule
