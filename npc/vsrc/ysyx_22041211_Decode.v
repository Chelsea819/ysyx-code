

module ysyx_22041211_Decode #(parameter DATA_LEN = 32)(
    input       [DATA_LEN - 1:0]	inst,
    output	    [DATA_LEN - 1:0]	imm ,
    output      [4:0]               rd  ,
    output      [4:0]               rsc1,
    output      [4:0]               rsc2


);
    reg     [2:0]                   key ;

    assign  rd      = inst[11:7];
    assign  rsc1    = inst[19:15];
    assign  rsc2    = inst[24:20];

    // 3'b000  I 
    // 3'b001  N 
    // 3'b010  U

    //type
    ysyx_22041211_MuxKeyWithDefault #(2, 7, 3) i0 (key, inst[6:0], 3'b0,{
        7'b0010011 , 3'b000,
        7'b1110011 , 3'b001
    });

    //imm
    ysyx_22041211_MuxKeyWithDefault #(2, 3, 32) i1 (imm, key, 32'b0,{
        3'b000 , {{20{inst[31]}},inst[31:20]},
        3'b010 , {inst[31:12],{12{1'b0}}}
    });


endmodule
