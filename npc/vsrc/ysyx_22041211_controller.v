module ysyx_22041211_controller #(parameter DATA_LEN = 10)(
    input           [DATA_LEN - 1:0]              inst, //func+opcode
    input           [2:0]                         key,
    //output          [2:0]                         alu_control,
    output                                        add_en,
    output                                        regWrite,
    // output                                        mem_toReg,
    // output                                        mem_write, //写内存操作
    output                                        alu_srcA,
    output                                        alu_srcB
);
    
    //ALU
    ysyx_22041211_MuxKeyWithDefault #(1, 10, 1) alu_mode (add_en, inst, 1'b0,{
        10'b0000010011 , 1'b1 //tell addi 
    });

    //choosing src1
    ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) src1_choose (alu_srcA, inst[6:0], 1'b0,{
        7'b0010111 , 1'b1 
    });

    //choosing src2
    //B/S/R--reg
    //I/U/J--imm
    ysyx_22041211_MuxKeyWithDefault #(1, 3, 1) src2_choose (alu_srcB, key, 1'b1,{
        3'b011 , 1'b0 
    });

    // //if store data to memory
    // ysyx_22041211_MuxKeyWithDefault #(1, 3, 1) w_mem (mem_write, key, 1'b0,{
    //     3'b100 , 1'b1 
    // });

    //store data from memory to reg
    //mem_toReg
    //sw（写内存）不需要写寄存器文件 不关心mem_toReg 
    //lw(写寄存器 读出内存中的值) mem_toReg = 1 
    //R（写寄存器 直接用ALU的结果） mem_toReg = 0
    // ysyx_22041211_MuxKeyWithDefault #(1, 10, 3) ifmem_toReg (mem_toReg, inst, 1'b0,{
    //     10'b1000000011 , 1'b1, //tell lbu
    //     10'b0100000011 , 1'b1, //tell lw
    //     10'b0010000011 , 1'b1, //tell lh
    //     10'b1010000011 , 1'b1  //tell lhu
    // });



    //regWrite----I/R/U 是否往reg里面写东西
    ysyx_22041211_MuxKeyWithDefault #(1, 3, 1) w_reg (regWrite, key, 1'b0,{
        3'b000 , 1'b1
    });
    
endmodule
