`include "./ysyx_22041211_define.v"
module ysyx_22041211_wb #(parameter DATA_LEN = 32)(
    input		                		rst		,
    input		                		wd_i		,
    input		[4:0]		            wreg_i		,
    input		[DATA_LEN - 1:0]		alu_result_i,
    input		                		mem_wen_i	,
	input		[DATA_LEN - 1:0]		mem_wdata_i	,
    input       [2:0]                   load_type_i , 
    output		                		wd_o		,
    output		[4:0]		            wreg_o		,
    output		[DATA_LEN - 1:0]		wdata_o
);
    wire [31:0] mem_waddr;
    wire [31:0] mem_raddr;
    wire [31:0] mem_rdata;
    reg  [31:0] mem_rdata_rare;
    wire [7:0]  mem_rmask;
    wire        mem_to_reg;
    assign wd_o = wd_i;
    assign wreg_o = wreg_i;
    assign mem_to_reg = |load_type_i;
    assign wdata_o = (mem_to_reg == 1'b1) ? mem_rdata : alu_result_i;
    assign mem_waddr = alu_result_i;
    assign mem_raddr = alu_result_i;

    assign mem_rdata = (load_type_i == `LOAD_LB_8)  ? {{24{mem_rdata_rare[7]}}, mem_rdata_rare[7:0]} : 
                       (load_type_i == `LOAD_LH_16) ? {{16{mem_rdata_rare[7]}}, mem_rdata_rare[15:0]}: 
                       mem_rdata_rare;

    assign mem_rmask = (load_type_i == `LOAD_LB_8 || load_type_i == `LOAD_LBU_8)   ? `LOAD_MASK_8 : 
                       (load_type_i == `LOAD_LH_16 || load_type_i == `LOAD_LHU_16) ? `LOAD_MASK_16 :
                       (load_type_i == `LOAD_LW_32)                                ? `LOAD_MASK_32 : 
                       0;
   
    // 访存指令
    import "DPI-C" context function int pmem_read_task(input int raddr, input byte wmask);
     //assign mem_rdata_rare = mem_raddr != 0 ? pmem_read_task(mem_raddr, mem_rmask) : 0;
	always @(*) begin
        if(rst)  mem_rdata_rare = 0;
        else if(mem_to_reg) begin
            $display("rst = %b",rst); 
            mem_rdata_rare = 0;
            //mem_rdata_rare = pmem_read_task(mem_raddr, mem_rmask);
        end else 
            mem_rdata_rare = 0;
	end
	import "DPI-C" function void pmem_write_task(input int waddr, input int wdata);
	always @(*) begin
  		if (mem_wen_i) // 有写请求时
      		pmem_write_task(mem_waddr, mem_wdata_i);
	end
endmodule
