/* 嵌入二进制数据和只读数据到目标文件中 */
/* C 程序通过 extern 关键字声明了在汇编代码中定义的全局符号，
   然后在函数中使用这些符号来访问嵌入的数据 */
.section .data                          //数据段，存放程序运行时需要读写的数据（如全局变量/静态变量）
.global ramdisk_start, ramdisk_end      //声明全局符号
ramdisk_start:
.incbin "build/ramdisk.img"             //将外部文件的内容直接包含到目标文件中
ramdisk_end:

.section .rodata                        //只读数据段
.globl logo
logo:
.incbin "resources/logo.txt"
.byte 0                                 //通常用于标记字符串的结束
