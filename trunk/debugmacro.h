#ifdef _DEBUG_MACRO_
#include <stdio.h>
#include <stdlib.h>
/* Log FILE, FUNCTION, LINE for logging data */
#define _DEBUG(fmt, args...) printf("%s:%s():%d: "fmt, __FILE__, __FUNCTION__,__LINE__, args)

#else

#define _DEBUG(x,args...)
#endif
