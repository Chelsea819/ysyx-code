/* verilator lint_off UNUSEDSIGNAL */

module ysyx_22041211_controller #(parameter DATA_LEN = 32)(
    input           [DATA_LEN - 1:0]              inst, //func+opcode
    input           [2:0]                         key,
    //output          [2:0]                         alu_control,
    output                                        add_en,
    output                                        regWrite,
    output                                        branch,
    output                                        pc_choose1,
    // output                                        mem_toReg,
    // output                                        mem_write, //写内存操作
    output                                        alu_srcA,
    output           [1:0]                        alu_srcB
);
    // wire                                          alu_addi;
    // wire                                          alu_apuic;

    // assign add_en = alu_addi | alu_apuic;
    
    //ALU
    // ysyx_22041211_MuxKeyWithDefault #(1, 10, 1) alu_mode1 (alu_addi, inst, 1'b0,{
    //     10'b0000010011 , 1'b1 //tell addi 
        
    // });
    // ysyx_22041211_MuxKeyWithDefault #(2, 7, 1) alu_mode2 (alu_apuic, inst[6:0], 1'b0,{
    //     7'b0010111 , 1'b1 , //tell aupic 
    //     7'b0110111 , 1'b1   //tell lui 
        
    // });

    //alu
    assign add_en = 1'b1;
    //assign add_en = {inst[14:12],inst[6:0]} == 10'b0000010011 || inst[6:0] ==  7'b0010111 || inst[6:0] == 7'b0110111 ? 1'b1 : 1'b0;


    //choosing src1
    assign alu_srcA = (inst[6:0] ==  7'b0010111 || inst[6:0] ==  7'b1101111) ? 1'b1 : 1'b0;


    // ysyx_22041211_MuxKeyWithDefault #(1, 7, 1) src1_choose (alu_srcA, inst[6:0], 1'b0,{
    //     7'b0010111 , 1'b1 //auipc -- pc
    // });

    //choosing src2
    //B/S/R--reg        00
    //I/U/J--imm        01
    //jalr jal      +4  10
    assign alu_srcB = ({inst[14:12],inst[6:0]} == 10'b0001100111 || inst[6:0] == 7'b1101111) ? 2'b10 :
                      (key == 3'b011) ? 2'b0 : 2'b1;

    // ysyx_22041211_MuxKeyWithDefault #(1, 3, 1) src2_choose (alu_srcB, key, 1'b1,{
    //     3'b011 , 1'b0   //reg-data -- reg
    // });

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



    //regWrite----I/R/U/J 是否往reg里面写东西
    //
    assign regWrite = (key == 3'b000 || key == 3'b011 || key == 3'b010 || key == 3'b011 || key == 3'b101) ? 1'b1 : 1'b0;
    // ysyx_22041211_MuxKeyWithDefault #(1, 3, 1) w_reg (regWrite, key, 1'b0,{
    //     3'b000 , 1'b1
    // });

    //branch----jalr jal-J B 
    assign branch = ({inst[14:12],inst[6:0]} == 10'b0001100111 || inst[6:0] == 7'b1101111) ? 1'b1 : 1'b0;
    
    //pc_src1 jalr----reg1  0
    assign pc_choose1 = ({inst[14:12],inst[6:0]} == 10'b0001100111) ? 1'b0 : 1'b1;
endmodule

/* verilator lint_on UNUSEDSIGNAL */
