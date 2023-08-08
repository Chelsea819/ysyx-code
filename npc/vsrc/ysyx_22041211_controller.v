module ysyx_22041211_controller #(parameter DATA_LEN = 10)(
    input           [DATA_LEN - 1:0]              inst,
    input           [2:0]                         key,
    output                                        add_en,
    output                                        reg_wen
);
    //add_en  
    ysyx_22041211_MuxKeyWithDefault #(1, 10, 1) i0 (add_en, inst, 1'b0,{
        10'b0000010011 , 1'b1
    });

    //reg_wen----I/R/U
    ysyx_22041211_MuxKeyWithDefault #(1, 3, 1) i1 (reg_wen, key, 1'b0,{
        3'b000 , 1'b1
    });
    
endmodule
