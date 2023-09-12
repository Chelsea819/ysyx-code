

module ysyx_22041211_Decode #(parameter DATA_LEN = 32)(
    input       [DATA_LEN - 1:0]	inst,
    output	    [DATA_LEN - 1:0]	imm ,
    input                           clk,
    output      [4:0]               rd  ,
    output      [4:0]               rsc1,
    output      [4:0]               rsc2,
    output      [2:0]               key


);
    wire    [5:0]                   key_all ;
    wire    [31:0]                  key_tmp;
    wire   [4:0]                   rsc1_0;
    wire   [4:0]                   rsc1_1;
                                                                            
    assign  rd      = inst[11:7];
    //assign  rsc1    = inst[19:15];
    assign  rsc2    = inst[24:20];
    assign  key_tmp = {{26{1'b0}},key_all};
    assign  key     = key_all[2:0];
    assign  rsc1    = rsc1_0 | rsc1_1;


    // 3'b000  I 
    // 3'b001  N 
    // 3'b010  U
    // 3'b011  R
    // 3'b100  S

    // //tell-opcode 
    // ysyx_22041211_MuxKeyWithDefault #(6, 7, 3) tell_opcode (key_all[2:0], inst[6:0], 3'b111,{
    //     7'b0010011 , 3'b000, // 3'b000  I addi sltiu srai andi
    //     7'b1110011 , 3'b001, // 3'b001  N ecall ebreak 
    //     7'b0110111 , 3'b010, // 3'b010  U lui
    //     7'b0010111 , 3'b010, // 3'b010  U auipc
    //     7'b0110011 , 3'b011, // 3'b011  R add sub
    //     7'b0100011 , 3'b100  // 3'b100  S sb sw sh
    // });

    //tell-opcode 
    assign key_all[2:0] = inst[6:0] == 7'b0010011 ? 3'b000 :                               // 3'b000  I addi sltiu srai andi
                          inst[6:0] == 7'b1110011 ? 3'b001 :                               // 3'b001  N ecall ebreak  
                          inst[6:0] == 7'b0110111 || inst[6:0] == 7'b0010111 ? 3'b010 :    // 3'b010  U lui auipc
                          inst[6:0] == 7'b0110011 ? 3'b011 :                               // 3'b011  R add sub
                          inst[6:0] == 7'b0100011 ? 3'b100 : 3'b111;                       // 3'b100  S sb sw sh


    // //type_N 识别具体是哪一条指令
    // ysyx_22041211_MuxKeyWithDefault #(2, 25, 3) tell_inst (key_all[5:3], inst[31:7], 3'b111,{
    //     25'b0000000000000000000000000 , 3'b000,  //N-ecall
    //     25'b0000000000010000000000000 , 3'b001   //N-ebreak
    // });

    //type_N 识别具体是哪一条指令
    assign key_all[5:3] = inst[31:7] == 25'b0000000000000000000000000 ? 3'b000 :             //N-ecall
                          inst[31:7] == 25'b0000000000010000000000000 ? 3'b001 : 3'b111;     //N-ebreak

    //rs1
    ysyx_22041211_MuxKeyWithDefault #(1, 7, 5) rs1_0_lui (rsc1_0, inst[6:0], inst[19:15],{
        7'b0110111 , 5'b0   //lui
    });
    ysyx_22041211_MuxKeyWithDefault #(1, 32, 5) rs1_0_ebreak (rsc1_1, inst, inst[19:15],{
        32'b00000000000100000000000001110011 , 5'b0   //ebreak 001001
    });




    // 检测到ebreak
    import "DPI-C" context function void ifebreak_func(int key);
    always @(posedge clk)
        dpi_key(key_tmp);

    task dpi_key(input reg [31:0] k);  // 在任务中使用 input reg 类型
        /* verilator no_inline_task */
        ifebreak_func(k);
    endtask


    //imm
    ysyx_22041211_MuxKeyWithDefault #(2, 3, 32) imm_choose (imm, key_all[2:0], 32'b0,{
        3'b000 , {{20{inst[31]}},inst[31:20]}, // 3'b000  I
        3'b010 , {inst[31:12],{12{1'b0}}}      // 3'b010  U
    });


endmodule
