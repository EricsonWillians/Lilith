#ifndef LILITH_GC_H
#define LILITH_GC_H

#ifdef __cplusplus
extern "C" {
#endif

void gc_track(void *obj);
void gc_collect(void);

#ifdef __cplusplus
}
#endif

#endif
