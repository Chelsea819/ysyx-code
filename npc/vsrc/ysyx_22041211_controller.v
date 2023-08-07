module ysyx_22041211_controller #(parameter DATA_LEN = 10)(
    input           [DATA_LEN - 1:0]              inst,
    output                                        add_en,
    output                                        reg_wen
);
    //add_en
    ysyx_22041211_MuxKeyWithDefault #(1, 10, 1) i0 (add_en, inst, 1'b0,{
        10'b0000010011 , 1'b1
    });

    //reg_en
    ysyx_22041211_MuxKeyWithDefault #(1, 10, 1) i1 (reg_wen, inst, 1'b0,{
        10'b0000010011 , 1'b1
    });
    
endmodule
