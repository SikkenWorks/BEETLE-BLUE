#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "beetle_help.h"
int main(void){
 assert(BEETLE_HELP_TOPIC_COUNT==10u);
 assert(BEETLE_HELP_CARD_COUNT==66u);
 uint8_t at=0u;
 for(uint8_t t=0u;t<BEETLE_HELP_TOPIC_COUNT;t++){
  const struct beetle_help_topic *topic=&beetle_help_topics[t];
  assert(topic->first==at && topic->count>0u);
  at=(uint8_t)(at+topic->count);
  uint8_t pages=beetle_help_page_count(t);
  assert(pages==(topic->count+2u)/3u);
  assert(beetle_help_page_shift(t,0u,-999)==0u);
  assert(beetle_help_page_shift(t,0u,999)==pages-1u);
  for(uint8_t page=0u;page<pages;page++){
   unsigned visible=0;
   for(uint8_t row=0u;row<3u;row++){
    uint8_t local=(uint8_t)(page*3u+row);
    if(local>=topic->count)continue;
    const struct beetle_help_card *card=&beetle_help_cards[topic->first+local];
    assert(strlen(card->title)>0u&&strlen(card->title)<=28u);
    assert(strlen(card->detail)>0u&&strlen(card->detail)<=33u);
    visible++;
   }
   assert(visible>=1u&&visible<=3u);
  }
 }
 assert(at==BEETLE_HELP_CARD_COUNT);
 assert(beetle_help_topic_shift(0u,-100)==0u);
 assert(beetle_help_topic_shift(0u,100)==BEETLE_HELP_TOPIC_COUNT-1u);
 assert(strstr(beetle_help_cards[beetle_help_topics[1].first+1u].detail,"SEQ")!=0);
 assert(strstr(beetle_help_cards[beetle_help_topics[6].first+4u].detail,"V33")!=0);
 return 0;
}
