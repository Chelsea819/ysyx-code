/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clk rst ren mem_raddr_i mem_rdata_usigned_o
module ysyx_22041211_inst_SRAM #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input									clk				,
	input									rst				,
	input									ren             , // (con_state == IFU_IDLE && next_state == IFU_WAIT_READY)
	input		[ADDR_LEN - 1:0]			mem_raddr_i     ,
    output reg	[DATA_LEN - 1:0]			mem_rdata_usigned_o            ,
	output reg	[DATA_LEN - 1:0]			id_inst_i
);	
	import "DPI-C" context function int pmem_read_task(input int raddr, input byte wmask);

    //fetch instruction
	always @(*) begin
		if(~rst) begin
			mem_rdata_usigned_o = pmem_read_task(mem_raddr_i, 8'b00001111);
		end else begin  
			mem_rdata_usigned_o = 32'b0;
		end
	end

    always @(posedge clk) begin
        if(ren) begin
            id_inst_i   <=   mem_rdata_usigned_o;
        end
	end
	
endmodule
