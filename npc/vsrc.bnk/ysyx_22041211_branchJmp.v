module ysyx_22041211_branchJmp(
	input		                		zero,
    input		                		SF,
	input		  [2:0]                 branch,
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

//branch
    //指令跳转
    //B 
    //000 no branch 
    //001 equal
    //010 unequal
    //011 <
    //100 >=
assign PCSrc = ({zero,branch,jmp} == 6'b1_001_00) ? 2'b01 : //001 equal
               ({zero,branch,jmp} == 6'b0_010_00) ? 2'b01 : //010 unequal
               ({SF,branch,jmp} == 6'b1_011_00) ? 2'b01 : //011 <
               ({SF,branch,jmp} == 6'b0_100_00) ? 2'b01 : //100 >=
               (jmp == 2'b01) ? 2'b01 : 
               (jmp == 2'b10) ? 2'b10 : 
               (invalid == 1'b0 ) ? 2'b00 : 2'b11; 


endmodule

