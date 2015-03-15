/***************************
 * includes all gl prototypes *
 ****************************/
#include "gl.h"
#include "glext.h"

#ifdef WIN32

   #include "mesa_wgl.h"
   #include "wglext.h"

#else /* UNIX/LINUX */

/* Include here gl unix/linux specific header files */

#endif
