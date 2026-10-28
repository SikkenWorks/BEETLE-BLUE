#include <assert.h>
#include <stdio.h>
#include "beetle_battery_policy.h"
int main(void){
 assert(beetle_battery_bars(0u)==1u);
 assert(beetle_battery_bars(550u)==2u);
 assert(beetle_battery_bars(614u)==2u);
 assert(beetle_battery_bars(615u)==3u);
 assert(beetle_battery_bars(685u)==4u);
 assert(beetle_battery_bars(1023u)==4u);
 assert(beetle_battery_filter(0u,800u,0u)==800u);
 puts("PASS uncalibrated battery bar rendering policy boundaries");
}
