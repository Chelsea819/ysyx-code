module ysyx_22041211_immGet #(parameter DATA_LEN = 32)(
    input       [DATA_LEN - 1:0]	inst,
    output	    [DATA_LEN - 1:0]	imm ,
    output      [2:0]               key
);
    wire    [2:0]                   key_opcode;
    wire    [2:0]                   key_certain;
    wire    [31:0]                  key_tmp;

 
    //imm 
    assign imm = inst[6:0] == 7'b0010011 ? {{20{inst[31]}},inst[31:20]} :         // 3'b000  I addi sltiu srai andi 
                 (inst[6:0] == 7'b0110111 || inst[6:0] == 7'b0010111) ? {inst[31:12],{12{1'b0}}} :  // 3'b010  U lui auipc
                 inst[6:0] == 7'b1101111 ? {{11{inst[31]}},inst[31],inst[19:12],inst[20],inst[30:21],1'b0}  : 32'b0;   // 3'b101  J                  

                 
    // // 3'b000  I 
    // // 3'b001  N 
    // // 3'b010  U
    // // 3'b011  R
    // // 3'b100  S
    // // 3'b101  J
    // //tell-opcode 
    // assign key_opcode = inst[6:0] == 7'b0010011 ? 3'b000 :                               // 3'b000  I addi sltiu srai andi
    //                       inst[6:0] == 7'b1110011 ? 3'b001 :                               // 3'b001  N ecall ebreak  
    //                       (inst[6:0] == 7'b0110111 || inst[6:0] == 7'b0010111) ? 3'b010 :  // 3'b010  U lui auipc
    //                       inst[6:0] == 7'b0110011 ? 3'b011 :                               // 3'b011  R add sub
    //                       inst[6:0] == 7'b0100011 ? 3'b100 :                               // 3'b100  S sb sw sh
    //                       inst[6:0] == 7'b1101111 ? 3'b101 : 3'b111;                       // 3'b101  J                  


    // //type_N 识别具体是哪一条指令
    // assign key_certain = inst[31:7] == 25'b0000000000000000000000000 ? 3'b000 :             //N-ecall
    //                      inst[31:7] == 25'b0000000000010000000000000 ? 3'b001 : 3'b111;     //N-ebreak

    // //imm
    // assign imm = key_opcode == 3'b000 ? {{20{inst[31]}},inst[31:20]} :      //I
    //              key_opcode == 3'b101 ? {{11{inst[31]}},inst[31],inst[19:12],inst[20],inst[30:21],1'b0} :      //J
    //              key_opcode == 3'b010 ? {inst[31:12],{12{1'b0}}} : 32'b0;   //U
                 
                 
    // // 检测到ebreak
    // import "DPI-C" context function void ifebreak_func(int key);
    // always @(posedge clk)
    //     dpi_key(key_tmp);

    // task dpi_key(input reg [31:0] k);  // 在任务中使用 input reg 类型
    //     /* verilator no_inline_task */
    //     ifebreak_func(k);
    // endtask


endmodule

