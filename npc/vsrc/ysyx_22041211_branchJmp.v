module ysyx_22041211_branchJmp(
	input		                		zero,
	input		                        branch,
	input 		                        jmp,
	output		                        PCSrc
);
    assign PCSrc = jmp | (zero & branch);

endmodule

