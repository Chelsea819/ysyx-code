/*************************************************************************
> File Name: ysyx_22041211_MuxKeyWithDefault.v
> Author: Chelsea
> Mail: 1938166340@qq.com 
> Created Time: 2023年08月04日 星期五 17时46分44秒
************************************************************************/

module ysyx_22041211_MuxKeyWithDefault #(parameter NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
	output [DATA_LEN-1:0] out,
	input [KEY_LEN-1:0] key,
	input [DATA_LEN-1:0] default_out,
	input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);
	ysyx_22041211_MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 1) i0 (out, key, default_out, lut);
endmodule


