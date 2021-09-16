#ifndef flags_h
#define flags_h

extern bool DEBUG_PRINT_CODE;
extern bool DEBUG_TRACE_EXECUTION;
extern bool DEBUG_STRESS_GC;
extern bool DEBUG_LOG_GC;
//#define NAN_BOXING

#define MAX_CASES 256
#define GC_HEAP_GROW_FACTOR 2
#include <cstdlib>

#endif /* flags_h */
