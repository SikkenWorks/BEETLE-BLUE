#include <assert.h>
#include <stdio.h>
#include "beetle_blue_live_autosave.h"
struct mock {unsigned calls;uint8_t success;};
static uint8_t callback(void *p){
 struct mock *m=(struct mock*)p;m->calls++;return m->success;
}
int main(void){
 struct mock m={0u,1u};uint8_t dirty=0u,status=0u;
 assert(blue_live_autosave_dirty(&dirty,&status,callback,&m));
 assert(!m.calls&&!dirty&&!status);
 dirty=1u;assert(blue_live_autosave_dirty(&dirty,&status,callback,&m));
 assert(m.calls==1u&&!dirty&&!status);
 dirty=1u;m.success=0u;
 assert(!blue_live_autosave_dirty(&dirty,&status,callback,&m));
 assert(dirty&&status==2u&&m.calls==2u);
 m.success=1u;
 assert(blue_live_autosave_dirty(&dirty,&status,callback,&m));
 assert(!dirty&&!status&&m.calls==3u);
 dirty=1u;assert(!blue_live_autosave_dirty(&dirty,&status,0,0));
 assert(dirty&&status==2u);
 puts("PASS setlist auto-save: no-op, success, fail-closed, retry, missing writer");
}
