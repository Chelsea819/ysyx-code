module ysyx_22041211_branchJmp(
	input		                		zero,
	input		                        branch,
	input 		                        jmp,
	output		  [1:0]                 PCSrc
);
    //00--default
    //01
    //10
    //B---branch zero---01  branch zero  1
    //J---              01  jmp 01
    //jalr              10  jmp 10

ysyx_22041211_MuxKeyWithDefault #(3,4,2) my_pcSrc (PCSrc, {zero,branch,jmp}, 2'b00,{
		4'b1100, 2'b01,
        4'b0001, 2'b01,
        4'b0010, 2'b10
});


endmodule

