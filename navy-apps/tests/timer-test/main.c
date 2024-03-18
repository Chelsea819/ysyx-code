#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    struct timeval tv;
    struct timezone tz;
    int n = 1;

    // store the time
    suseconds_t  time = 0;

    while(1){
        do{
            assert(gettimeofday(&tv, &tz));
            time = tv.tv_usec;
        }while(time / 500000 < n);
        printf("NO.%d output.\n",n ++);
    }

  return 0;
}