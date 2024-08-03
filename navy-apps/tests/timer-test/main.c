#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <NDL.h>

int main() {
    int n = 1;

    // store the time
    suseconds_t  time = 0;

    while(1){
        do{
            time = NDL_GetTicks();
        }while(time / 500000 < n);
        printf("NO.%d output.\n",n ++);
    }

  return 0;
}