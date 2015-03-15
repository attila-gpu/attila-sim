/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#ifndef VSLOADER_H
    #define VSLOADER_H

#include "ProgramObject.h"
#include "GPUDriver.h"
#include "gl.h"
#include "glext.h"
//#include <map> /* for mapping shaders */

namespace libgl
{
    
class GLContext;

/**
 * class used for loading vertex shaders in GPU
 *
 * In future it will include all T&L emulations via shaders
 */
class VSLoader
{
   
private:

    GPUDriver* driver; // current driver
    GLContext* ctx;
        
    void resolveBank(ProgramObject& po, RBank<float>& b);
        
public:

    VSLoader(GLContext* ctx, ::GPUDriver* driver);

    //VSLoader(GPUDriver* driver, const glsNS::GLState& gls, TLState& tls, FPState& fps);
    
    void initShader(ProgramObject& po, GLenum target, GLContext* ctx);
    
    /* Selects a shader and loads into GPU using driver interface */
    //u32bit setupShader(bool useUserShader, GLenum target);
    
    ~VSLoader();
    
};

} // namespace libgl

#endif
