module ysyx_22041211_branchJmp(
	input		                		zero,
	input		                        branch,
	input 		  [1:0]                 jmp,
    input                               invalid,
	output		  [1:0]                 PCSrc
);
    //00--default
    //01
    //10
    //B---branch zero---01  branch zero  1
    //J---              01  jmp 01
    //jalr              10  jmp 10

assign PCSrc = ({zero,branch,jmp} == 4'b1100) ? 2'b01 : 
               (jmp == 2'b01) ? 2'b01 : 
               (jmp == 2'b10) ? 2'b10 : 
               (invalid == 1'b0 ) ? 2'b00 : 2'b11; 


endmodule

