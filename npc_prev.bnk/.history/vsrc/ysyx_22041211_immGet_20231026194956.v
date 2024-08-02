/* verilator lint_off UNUSEDSIGNAL */
module ysyx_22041211_immGet #(parameter DATA_LEN = 32)(
    input       [DATA_LEN - 1:0]	inst,
    output	    [DATA_LEN - 1:0]	imm 
);
//     wire    [2:0]                   key_opcode;
//     wire    [2:0]                   key_certain;
//     wire    [31:0]                  key_tmp;

 
    //imm 
    assign imm = inst[6:0] == 7'b0010011 ? {{20{inst[31]}},inst[31:20]} :         // 3'b000  I addi sltiu srai andi 
                 (inst[6:0] == 7'b0110111 || inst[6:0] == 7'b0010111) ? {inst[31:12],{12{1'b0}}} :  // 3'b010  U lui auipc
                 inst[6:0] == 7'b1101111 ? {{11{inst[31]}},inst[31],inst[19:12],inst[20],inst[30:21],1'b0}  : 32'b0;   // 3'b101  J                  

endmodule
/* verilator lint_on UNUSEDSIGNAL */
