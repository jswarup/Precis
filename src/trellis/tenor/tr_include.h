// tr_include.h _____________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_include.h"
//_____________________________________________________________________________________________________________________________


#define CV_CACHELINE_SIZE 64
#define CV_PREFETCH_CACHE( Addr)               {  _builtin_prefetch( Addr);  
    
#include <atomic>
#ifdef PR_VISUALSTUDIO

#include    <intrin.h>
#else
#include <x86intrin.h>

#endif

//_____________________________________________________________________________________________________________________________
