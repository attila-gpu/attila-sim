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

#include "VSLoader.h"
#include "GLContext.h"
#include "ProgramManager.h"
#include "TLFactory.h"
#include "FPFactory.h"
#include <string>

using namespace std;
using namespace libgl;
using namespace libgl::glsNS;

VSLoader::VSLoader(GLContext* ctx, ::GPUDriver* driver) : driver(driver), ctx(ctx)
{
    GPU_DEBUG(cout << "Creating VSLoader..." << endl;)
}

VSLoader::~VSLoader()
{}



void VSLoader::initShader(ProgramObject& po, GLenum target, GLContext* ctx)
{

    if ( po.getTargetName() != target )
        panic("VSLoader", "initShader", "Program object target does not match");

    if ( !po.isCompiled() )
    {
        po.compile(); /* compile if required */
        
    }
    
    //  Check if the program has been translated/transformed by the driver
    if (!po.isTranslated())
    {
        //  Perform optimization/translation/transformation of the shader using the GPU Driver
        //  functions.
        
        GLsizei sourceCodeSize = po.getBinarySize();
        u32bit translatedCodeSize = sourceCodeSize * 10;
        
        //  Allocate buffer for the input source code and the transformed source code.
        u8bit *inCode = new u8bit[sourceCodeSize];
        u8bit *outCode = new u8bit[translatedCodeSize];
        
        //  Get the source binary code.
        po.getBinary((GLubyte *) inCode, sourceCodeSize);

        /////////////////////////////////////////////////////////////////////////////
        //  Get the API information required for the microtriangle transformation.
        /////////////////////////////////////////////////////////////////////////////
        GPUDriver::MicroTriangleRasterSettings settings;
        
        bool frontIsCCW = (ctx->getFaceMode() == GL_CCW);
        bool frontCulled = ((ctx->getCullFace() == GL_FRONT) || (ctx->getCullFace() == GL_FRONT_AND_BACK));
        bool backCulled = ((ctx->getCullFace() == GL_BACK) || (ctx->getCullFace() == GL_FRONT_AND_BACK));

        settings.faceCullEnabled = ctx->testFlags(GLContext::flagCullFace);

        if (frontIsCCW)
        {
            settings.CCWFaceCull = frontCulled;
            settings.CWFaceCull = backCulled;
        }
        else // backface is CCW
        {
            settings.CCWFaceCull = backCulled;
            settings.CWFaceCull = frontCulled;
        }

        //  Check the type of projection.
        Matrixf mat = ctx->mtop(GLContext::PROJECTION);
        settings.zPerspective = (mat[3][2] != 0.0f) && (mat[3][3] == 0.0f); 

        //  Initialize the attributes. Smooth interpolated and perspective correct if
        //  perspective transformation.
        for (unsigned int attr = 0; attr < gpu3d::MAX_FRAGMENT_ATTRIBUTES; attr++)
        {
            settings.smoothInterp[attr] = true;
            settings.perspectCorrectInterp[attr] = settings.zPerspective;
        }
        
        //  Set flat interpolation for the color attribute if GL_FLAT shade model used.
        settings.smoothInterp[gpu3d::COLOR_ATTRIBUTE] = (ctx->getShadeModel() == GL_TRUE);
        settings.smoothInterp[gpu3d::COLOR_ATTRIBUTE_SEC] = (ctx->getShadeModel() == GL_TRUE);

        /////////////////////////////////////////////////////////////////////////////

        // Use the GPU driver shader program capabilities.
        driver->translateShaderProgram(inCode, sourceCodeSize, outCode, translatedCodeSize,
                                       (target == GL_VERTEX_PROGRAM_ARB), po.getResources().maxAliveTemps, settings);
        
        //  Set the transformed shader program as the new binary code.
        po.setBinary((GLubyte *) outCode, translatedCodeSize);
    
        //  Set program as translated by the driver.
        po.setAsTranslated();
            
        //  Delete buffers.        
        delete[] inCode;
        delete[] outCode;
    }
    
    //po.printSource();
//po.printASM();

    gpu3d::GPURegData data;

    // It is not need to be called each time (patch for now)
    // maxAliveTemps is available after a PO is compiled
    data.uintVal = po.getResources().maxAliveTemps;

    /*   Convert temporal registers to thread resources.  One thread resource => 2 temporal registers.  */
    //data.uintVal = (data.uintVal > 0)?((data.uintVal >> 1) + (data.uintVal & 0x01)):1;
    data.uintVal = (data.uintVal > 0)?data.uintVal:1;

    /*  Determine target.  */
    if ( target == GL_FRAGMENT_PROGRAM_ARB )
    {
        /*  Write fragment per thread resource usage.  */
        driver->writeGPURegister(gpu3d::GPU_FRAGMENT_THREAD_RESOURCES, data);
    }
    else
    {
        /*  Write vertex per thread resource usage.  */
        driver->writeGPURegister(gpu3d::GPU_VERTEX_THREAD_RESOURCES, data);
    }
    
    // Allocate/synchronize program
    ctx->gpumem().allocate(po);
    /*
    if ( ctx->gpumem().allocate(po) )
        cout << "Synchronizing Program object (copying to GPU)" << endl;
    else
        cout << "Program object is already synchronized" << endl;
    */


    RBank<float>& constants = po.getClusterBank();

    resolveBank(po, constants);

    u32bit i;

    /* Copy constants to Shader program constant bank */
    for ( i = 0; i < constants.size(); i++ )
    {
        if ( !constants.usedPosition(i) )
            break;

        data.qfVal[0] = constants[i][0];
        data.qfVal[1] = constants[i][1];
        data.qfVal[2] = constants[i][2];
        data.qfVal[3] = constants[i][3];

        if ( target == GL_VERTEX_PROGRAM_ARB )
            driver->writeGPURegister(gpu3d::GPU_VERTEX_CONSTANT, i, data);
        else
            driver->writeGPURegister(gpu3d::GPU_FRAGMENT_CONSTANT, i, data);
    }

    if ( target == GL_VERTEX_PROGRAM_ARB )
        driver->commitVertexProgram(ctx->gpumem().md(po), po.getBinarySize(), 0);
    else if ( target == GL_FRAGMENT_PROGRAM_ARB )
        driver->commitFragmentProgram(ctx->gpumem().md(po), po.getBinarySize(), 0);

}




void VSLoader::resolveBank(ProgramObject& po, RBank<float>& b)
{
    const glsNS::GLState& gls = ctx->getGLState();

    RBank<float>& loc = po.getLocalParams();
    RBank<float>& env = po.getEnvParams();


    for ( u32bit i = 0; i < b.size(); i++ )
    {
        GLint pos, pos2;
        RType regType = b.getType(i,pos,pos2);

        switch ( regType )
        {
            case QR_PARAM_LOCAL:
                if ( loc.usedPosition(pos) )
                    b[i] = loc[pos]; // copy local parameter
                else
                    b[i] = QuadReg<float>(0,0,0,0);
                break;
            case QR_PARAM_ENV:
                if ( env.usedPosition(pos) )
                    b[i] = env[pos]; // copy global parameter
                else
                    b[i] = QuadReg<float>(0,0,0,0);
                break;
            case QR_GLSTATE:
                // pos contains state
                // access state
                if ( pos < BASE_STATE_MATRIX )
                    b[i] = gls.getVector(pos);
                else
                {
                    Matrixf temp = gls.getMatrix(pos);
                    b[i][0] = temp[pos2][0];
                    b[i][1] = temp[pos2][1];
                    b[i][2] = temp[pos2][2];
                    b[i][3] = temp[pos2][3];
                    //b[i] = gls.getMatrix(pos)[pos2];
                }
                break;
            default:
                ;
                //panic("VSLoader","resolveBank()", "QR_UNKNOWN not allowed here");
        }
    }
}
