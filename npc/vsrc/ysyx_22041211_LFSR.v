module ysyx_22041211_LFSR#(DATA_LEN = 4)(
    input									    clock				,
	input									    rstn			,
    input       [DATA_LEN-1:0]                  initial_var     ,
    output reg  [DATA_LEN-1:0]                  result                              
);
    wire                                       new_bit;

    assign new_bit = result[0] ^ result[DATA_LEN-1];
    always @(posedge clock ) begin
        if(rstn)
            result <= {new_bit, result[DATA_LEN-1:1]};
        else 
            result <= initial_var;
    end

endmodule
