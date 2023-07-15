/*************************************************************************
    > File Name: main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月14日 星期五 21时58分49秒
  > Created Time: 2023年07月14日 星期五 21时58分49秒
 ************************************************************************/

#include <nvboard.h>
#include <Vtop.h>

static TOP_NAME dut;

void nvboard_bind_all_pins(Vtop* top);

//static void single_cycle(){
//	dut.clk = 0; dut.eval();
//	dut.clk = 1; dut.eval();
//}
//
//static void reset(int n){
//	dut.rst = 1;
//	while (n -- >0) single_cycle();
//	dut.rst = 0;
//}

int main(){
	nvboard_bind_all_pins(&dut);

	nvboard_init();

	//reset(10);

	while(1){
		nvboard_update();
		dut.eval();
	//	single_cycle();
	}
//	nvboard_quit();
}


