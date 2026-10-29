#include <assert.h>
#include <stdio.h>
#include "beetle_battery_policy.h"
int main(void){
 assert(beetle_battery_bars(530u)==0u);
 assert(beetle_battery_bars(531u)==1u);
 assert(beetle_battery_bars(560u)==1u);
 assert(beetle_battery_bars(561u)==3u);
 assert(beetle_battery_bars(590u)==3u);
 assert(beetle_battery_bars(591u)==4u);
 assert(beetle_battery_raw_valid(540u));
 assert(!beetle_battery_raw_valid(0u));
 assert(!beetle_battery_raw_valid(1023u));
 assert(beetle_battery_filter(0u,800u,0u)==800u);
 assert(beetle_battery_filter(600u,500u,1u)==588u);
 puts("PASS: FM1 Felucca ADC battery voltage bands, no fabricated percentage or charge flag");
}
