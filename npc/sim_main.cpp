/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "svdpi.h"
#include "Vysyx_22041211_top__Dpi.h"
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top___024root.h"
#include "npc.h"


#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;
#define PMEM_LEFT  ((uint32_t )CONFIG_MBASE)
#define CONFIG_MBASE 0X80000000
#define CONFIG_PC_RESET_OFFSET 0x0
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)



static  TOP_NAME dut;
static uint32_t pmem[20] = {0};
bool ifbreak = false;
static char *img_file = NULL;

void ifebreak_func(int key){
	//printf("while key = %d\n",key);
	if(key == 9) {ifbreak = true; } 
}


static int parseArgs(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"ftrace"   , required_argument, NULL, 'f'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:f:", table, NULL)) != -1) {
    //参数个数 参数数组 短选项列表 长选项列表 处理长选项时返回选项的索引
    switch (o) {
      case 'b': break;
      case 'p': break;
      case 'l': break;
      case 'd': break;
      case 'f': break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-f,--ftrace=FILE        get elf from FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_mem_npc(){
	//80000000-80000004-80000008-8000000c-80000010-80000014-80000024-80000028-8000002c-80000018-8000001c-80000020-80000030

	pmem[0] = 0b00000000000100000000000010010011; //addi    x[1] = 0 + 1     	80000000	1048723	100093
	pmem[1] = 0b00000000010000000000000100010011; //2   x[1] = 0 + 2 			80000004	4194579	400113	
	pmem[2] = 0b00000000000000000001010000010111; //auipc						80000008	5143
	pmem[3] = 0b00000000000000000010010000010111; //auipc						8000000c	9239
	pmem[4] = 0b00000000000000000110000110110111; //lui							80000010	25015
	pmem[5] = 0b00000000100000000000000011101111; //jal   x[1] = pc + 4 pc += 8 80000014	8388847
	pmem[6] = 0b00000000000000001110001110110111; //lui							80000018	58295
	pmem[7] = 0b00000000000000011110011110110111; //lui							8000001c	124855
	pmem[8] = 0b00000000100000000000000011101111; //jal   x[1] = pc + 4 pc += 8 80000020	8388847
	pmem[9] = 0b00000000111000000000001110010011; //addi x[3] = 0 + 6 			80000024	14680979
	pmem[10] = 0b00000001000000000000010000010011; //addi x[4] = 0 + 7			80000028	16778259
	pmem[11] = 0b00000000000000001000000111100111; //jalr 					    8000002c   	41447		x[3] = pc + 4 pc = x[1] + imm = 80000018 + 0
	pmem[12] = 0b00000000000100000000000001110011; //ebreak						80000030

	return ;
}

static inline uint32_t host_read(void *addr) { 
	//printf("before host_read addr = %p\n",addr);
    return *(uint32_t *)addr;
	//printf("after host_read\n");
}


uint32_t* guest_to_host(uint32_t paddr) { 
	// printf("before  guest_to_host\n");
	// printf("pmem + (paddr - CONFIG_MBASE) / 4 = 0x%x\n",pmem + (paddr - CONFIG_MBASE) / 4);
	return pmem + (paddr - CONFIG_MBASE) / 4; 
	}

uint32_t pmem_read_npc(uint32_t addr) {
	//printf("before mem_read_npc!\n");
	uint32_t ret = host_read(guest_to_host(addr));
	//printf("after pmem_read_npc!\n");
	printf("inst = %d 16进制:%08x  pc = 0x0=%08x\n",ret,ret,addr);
  return ret;
}

// void get_inst(){
// 	if(dut.pc != 0) dut.inst = pmem_read_npc(dut.pc);
	
// }

static long load_img() {
  if (img_file == NULL) {
    printf("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
    //如果img_file为NULL,说明没有指定文件,则使用默认内置的镜像,返回其大小4096
  }

  FILE *fp = fopen(img_file, "rb");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  //ftell()可以获取文件当前的读写位置偏移量
  long size = ftell(fp);

  printf("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  //fread()可以高效地从文件流中读取大块的二进制数据,放入指定的内存缓冲区中
  assert(ret == 1);

  fclose(fp);
  return size;
}


int main(int argc, char** argv, char** env) {
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
	VerilatedVcdC *m_trace = new VerilatedVcdC;

	parseArgs(argc, argv);

	init_mem_npc();  //初始化内存

	load_img();

	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	
	dut.clk = 0; 
	dut.eval();
	dut.rst = 1;
	dut.eval();

	m_trace->dump(sim_time);
	sim_time++;

	dut.clk = 1;
	dut.eval(); 
	dut.rst = 0;
	dut.eval();

	int n = 0;
	dut.inst = pmem_read_npc(dut.pc);
	

	dut.eval();
	m_trace->dump(sim_time);
	sim_time++;

	while (sim_time < MAX_SIM_TIME) {		
		dut.clk ^= 1;
		dut.eval();
		if(dut.clk == 1) {
			dut.inst = pmem_read_npc(dut.pc);
		}
		dut.eval();
		m_trace->dump(sim_time);
		sim_time++;

		if(ifbreak) {
			printf("\nebreak!\n");
			break;
		}
	}
	dut.final();
	m_trace->close();	//关闭波形跟踪文件
	exit(EXIT_SUCCESS);
}


