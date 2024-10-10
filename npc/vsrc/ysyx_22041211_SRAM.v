/*************************************************************************
	> File Name: ysyx_22041211_counter.v
	> Author: Chelsea
	> Mail: 1938166340@qq.com 
	> Created Time: 2023年08月05日 星期六 22时12分23秒
 ************************************************************************/
// clock reset waddr wdata wen wmask
`include "ysyx_22041211_define.v"
module ysyx_22041211_SRAM #(parameter ADDR_LEN = 32, DATA_LEN = 32)(
	input								rstn			,
    input		                		clock			,
    input		                		ren			,
    input		                		mem_wen_i	,
	input		[DATA_LEN - 1:0]		mem_wdata_i	,
	input		[ADDR_LEN - 1:0]		mem_waddr_i	,
	input		[ADDR_LEN - 1:0]		mem_raddr_i	,
	input		[7:0]  					mem_wmask	,
    output	reg	[DATA_LEN - 1:0]		mem_rdata_usigned_o
);	

	// always @(*) $display("mren: [%d] mem_raddr_i: [%d] mem_raddr: [%x]",ren ,mem_raddr_i, mem_raddr_i);

	// 访存指令
    import "DPI-C" context function int pmem_read_task(input int raddr, input byte wmask);
	always @(*) begin
        if(ren & rstn)begin 
			// $display("mem_to_reg: [%d] mem_raddr_i: [%d] mem_raddr: [%x]",ren ,mem_raddr_i, mem_raddr_i);
            mem_rdata_usigned_o = pmem_read_task(mem_raddr_i, {8{ren}});
            end
        else begin
            mem_rdata_usigned_o = 0;
        end
	end
	import "DPI-C" function void pmem_write_task(input int waddr, input int wdata, input byte wmask);
	always @(posedge clock) begin
  		if (mem_wen_i) begin // 有写请求时
			// $display("mem_wen_i = %d mem_wmask = %x mem_waddr = %x\n",mem_wen_i, mem_wmask, mem_waddr_i);
            pmem_write_task(mem_waddr_i, mem_wdata_i, mem_wmask);
		end
	end

endmodule
