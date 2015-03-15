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
 * $RCSfile: FragmentOpEmulator.cpp,v $
 * $Revision: 1.17 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:16 $
 *
 * Fragment Operation Emulator class implementation file.
 *
 */

/**
 *
 *  @file FragmentOpEmulator.cpp
 *
 *  This file implements Fragment Emulator Operation class.
 *
 *  This class implements functions that emulate the operations
 *  performed to fragments after the pixel shader (Stencil test,
 *  Z test, blending, logical operations).
 *
 */

#include "FragmentOpEmulator.h"
#include "GPUMath.h"
#include "support.h"
#include <cstdio>

using namespace gpu3d;

/*  Fragment Operation Emulator constructor.  */
FragmentOpEmulator::FragmentOpEmulator(u32bit stampFrags)
{
    /*  Set number of fragments per stamp.  */
    stampFragments = stampFrags;

    /*  Allocate stamp stencil values buffer.  */
    stencil = new u8bit[stampFragments];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (stencil == NULL)
            panic("FragmentOpEmulator", "FragmentOpEmulator", "Error allocating stamp stencil buffer.");
    )

    /*  Allocate stamp z buffer values buffer.  */
    depth = new u32bit[stampFragments];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (depth == NULL)
            panic("FragmentOpEmulator", "FragmentOpEmulator", "Error allocating the stamp z buffer values buffer.");
    )

    /*  Allocate the stamp stencil test result array.  */
    stencilResult = new bool[stampFragments];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (stencilResult == NULL)
            panic("FragmentOpEmulator", "FragmentOpEmulator", "Error allocating the stamp stencil test result buffer.");
    )

    /*  Allocate the stamp z test result buffer.  */
    zResult = new bool[stampFragments];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (zResult == NULL)
            panic("FragmentOpEmulator", "FragmentOpEmulator", "Error allocating stamp z test result buffer.");
    )

    /*  Allocate the blend factor arrays.  */
    sFactor = new QuadFloat[stampFragments];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (sFactor == NULL)
            panic("FragmentOpEmulator", "FragmentOpEmulator", "Error allocating source factor array.");
    )

    dFactor = new QuadFloat[stampFragments];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (dFactor == NULL)
            panic("FragmentOpEmulator", "FragmentOpEmulator", "Error allocating destination factor array.");
    )

    /*  Set initial state.  */
    depthTest = FALSE;
    stencilTest = FALSE;
    stencilFunction = GPU_ALWAYS;
    stencilReference = 0x00;
    stencilTestMask = 0xff;
    stencilUpdateMask = 0xff;
    stencilFail = STENCIL_KEEP;
    depthFail = STENCIL_KEEP;
    depthPass = STENCIL_KEEP;
    depthFunction = GPU_LESS;
    depthMask =  TRUE;
}

/*  Enables or disables stencil test.  */
void FragmentOpEmulator::setStencilTest(bool enable)
{
    /*  Set stencil test.  */
    stencilTest = enable;
}

/*  Enables or disables Z test.  */
void FragmentOpEmulator::setZTest(bool enable)
{
    /*  Set z test.  */
    depthTest = enable;
}

/*  Configure stencil test.  */
void FragmentOpEmulator::configureStencilTest(CompareMode stencilFunc, u8bit ref,
        u8bit testMask, u8bit updateMask, StencilUpdateFunction stnclFail,
        StencilUpdateFunction zFail, StencilUpdateFunction zPass)
{
    /*  Configure Stencil test parameters.  */
    stencilFunction = stencilFunc;
    stencilReference = ref;
    stencilTestMask = testMask;
    stencilUpdateMask = updateMask;
    stencilFail = stnclFail;
    depthFail = zFail;
    depthPass = zPass;
}

/*  Configure depth test.  */
void FragmentOpEmulator::configureZTest(CompareMode zFunc, bool zMask)
{
    /*  Configure z test parameters.  */
    depthFunction = zFunc;
    depthMask = zMask;
}

/*  Set blending parameters.  */
void FragmentOpEmulator::setBlending(u32bit rt, BlendEquation eq, BlendFunction sRGB,
    BlendFunction sA, BlendFunction dRGB, BlendFunction dA, QuadFloat color)
{
    /*  Set blend parameters.  */
    equation[rt] = eq;
    srcRGB[rt] = sRGB;
    srcAlpha[rt] = sA;
    dstRGB[rt] = dRGB;
    dstAlpha[rt] = dA;
    constantColor[rt] = color;
}

/*  Set logical operation mode.  */
void FragmentOpEmulator::setLogicOpMode(LogicOpMode mode)
{
    logicOpMode = mode;
}

/*  Perform Stencil and Z test for a stamp.  */
void FragmentOpEmulator::stencilZTest(u32bit *stampZ, u32bit *bufferZ,
    bool *stampCull)
{
    u32bit i;

    /*  NOTE: ONLY WORKS FOR 32 BIT STENCIL/Z MODE (8/24).  */

//printf("read stencil values -> ");
    /*  Get the stencil values for the stamp.  */
    for(i = 0; i < stampFragments; i++)
    {
        stencil[i] = (bufferZ[i] >> 24);
//printf("%x ", stencil[i]);
    }
//printf("\n", stencil[i]);

    /*  Get the z buffer values for the stamp.  */
    for(i = 0; i < stampFragments; i++)
    {
        depth[i] = (bufferZ[i] & 0x00ffffff);
    }

    /*  Perform stencil test.  */
    compare(stencilFunction, stencil, stencilReference, stencilResult);


//printf("FrOpEmu >> depthFunction %d\n", depthFunction);

    /*  Perform z test.  */
    compare(depthFunction, stampZ, depth, zResult);

    /*  Update stencil.  */
    if (stencilTest)
    {
        /*  Update stencil for stencil fail case.  */
        /*  Update stencil for depth fail case.  */
        /*  Update stencil for depth pass case.  */
        for(i = 0; i < stampFragments; i++)
        {
            /*  Check if fragment is culled.  */
            if (!stampCull[i])
            {
                /*  Check if stencil test failed for the fragment.  */
                if (!stencilResult[i])
                {
                    /*  Calculate new stencil value.  */
                    updateStencil(stencilFail, stencil[i], bufferZ[i]);
                }
                else
                {
                    /*  Check if depth passed.  */
                    if (zResult[i])
                    {
                        /*  Calculate new stencil value.  */
                        updateStencil(depthPass, stencil[i], bufferZ[i]);
                    }
                    else
                    {
                        /*  Depth test failed.  Calculate new stencil value.  */
                        updateStencil(depthFail, stencil[i], bufferZ[i]);
                    }
                }
            }
        }

    }

    /*  Update z buffer.  */
    if (depthTest)
    {
        /*  Update stamp fragment Z.  */
        for(i = 0; i < stampFragments; i++)
        {
//printf("FrOpEmu >> Depth Test -> Fragment %d stampCull = %d stencilResult = %d zResult %d depthMask %d\n",
//i, stampCull[i], stencilResult[i], zResult[i], depthMask);

            /*  Check that the fragment is not culled, passed stencil
                and depth test, and depth update is enabled.  */
            if (!stampCull[i] && stencilResult[i] && zResult[i]
                && depthMask)
            {
                /*  Update fragment z value in the framebuffer.  */
                bufferZ[i] = (bufferZ[i] & 0xff000000) |
                    (stampZ[i] & 0x00ffffff);
//printf("FrOpEmu >> Updating z buffer data at %p\n", &bufferZ[i]);
            }
        }
    }

    /*  Update stamp fragment culling.  */
    for(i = 0; i < stampFragments; i++)
    {
        /*  Update fragment cull bit.  */
        stampCull[i] = stampCull[i] || !stencilResult[i] || !zResult[i];
    }
}


/*  Blends two stamps using the current blend mode.  */
void FragmentOpEmulator::blend(u32bit rt, QuadFloat *color, QuadFloat *source,
    QuadFloat *destination)
{
    u32bit i;

    /*  Calculate source factor RGB components.  */
    factorRGB(srcRGB[rt], constantColor[rt], source, destination, sFactor);

    /*  Calculate source factor alpha component.  */
    factorAlpha(srcAlpha[rt], constantColor[rt], source, destination, sFactor);

    /*  Calculate destination factor RGB components.  */
    factorRGB(dstRGB[rt], constantColor[rt], source, destination, dFactor);

    /*  Calculate destination factor alpha component.  */
    factorAlpha(dstAlpha[rt], constantColor[rt], source, destination, dFactor);

    /*  Blend source and destination.  */
    switch(equation[rt])
    {
        case BLEND_FUNC_ADD:

            /*  Add source and destination colors.  */
            for(i = 0; i < stampFragments; i++)
            {
                color[i][0] = source[i][0] * sFactor[i][0] + destination[i][0] * dFactor[i][0];
                color[i][1] = source[i][1] * sFactor[i][1] + destination[i][1] * dFactor[i][1];
                color[i][2] = source[i][2] * sFactor[i][2] + destination[i][2] * dFactor[i][2];
                color[i][3] = source[i][3] * sFactor[i][3] + destination[i][3] * dFactor[i][3];
            }

            break;

        case BLEND_FUNC_SUBTRACT:

            /*  Subtract source and destination colors.  */
            for(i = 0; i < stampFragments; i++)
            {
                color[i][0] = source[i][0] * sFactor[i][0] - destination[i][0] * dFactor[i][0];
                color[i][1] = source[i][1] * sFactor[i][1] - destination[i][1] * dFactor[i][1];
                color[i][2] = source[i][2] * sFactor[i][2] - destination[i][2] * dFactor[i][2];
                color[i][3] = source[i][3] * sFactor[i][3] - destination[i][3] * dFactor[i][3];
            }

            break;

        case BLEND_FUNC_REVERSE_SUBTRACT:

            /*  Subtract destination and source colors.  */
            for(i = 0; i < stampFragments; i++)
            {
                color[i][0] = -source[i][0] * sFactor[i][0] + destination[i][0] * dFactor[i][0];
                color[i][1] = -source[i][1] * sFactor[i][1] + destination[i][1] * dFactor[i][1];
                color[i][2] = -source[i][2] * sFactor[i][2] + destination[i][2] * dFactor[i][2];
                color[i][3] = -source[i][3] * sFactor[i][3] + destination[i][3] * dFactor[i][3];
            }

            break;

        case BLEND_MIN:

            /*  Minimum.  */
            for(i = 0; i < stampFragments; i++)
            {
                color[i][0] = GPU_MIN(source[i][0], destination[i][0]);
                color[i][1] = GPU_MIN(source[i][1], destination[i][1]);
                color[i][2] = GPU_MIN(source[i][2], destination[i][2]);
                color[i][3] = GPU_MIN(source[i][3], destination[i][3]);
            }

            break;

        case BLEND_MAX:

            /*  Maximum.  */
            for(i = 0; i < stampFragments; i++)
            {
                color[i][0] = GPU_MAX(source[i][0], destination[i][0]);
                color[i][1] = GPU_MAX(source[i][1], destination[i][1]);
                color[i][2] = GPU_MAX(source[i][2], destination[i][2]);
                color[i][3] = GPU_MAX(source[i][3], destination[i][3]);
            }

            break;

        default:
            panic("FragmentOpEmulator", "blend", "Unsupported blend equation mode.");
            break;
    }
}

/*  Performs a logical operation between the incoming stamp and the color buffer.  */
void FragmentOpEmulator::logicOp(u8bit *color, u8bit *source, u8bit *destination)
{
    u32bit i;

    //
    //  Note: only 32-bit color format supported (RGBA8).
    //
    
    /*  Which logical operation mode to use?  */
    switch(logicOpMode)
    {

        case LOGICOP_CLEAR:

            /*  Clear color.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Clear byte.  */
                color[i] = 0;
            }

            break;

        case LOGICOP_AND:

            /*  And source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical and.  */
                color[i] = source[i] & destination[i];
            }

            break;

        case LOGICOP_AND_REVERSE:

            /*  And reverse source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical and reverse.  */
                color[i] = source[i] & ~destination[i];
            }

            break;

        case LOGICOP_COPY:

            /*  Copy source color.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Just copy.  */
                color[i] = source[i];
            }

            break;

        case LOGICOP_AND_INVERTED:

            /*  And inverted source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical and inverted .  */
                color[i] = ~source[i] & destination[i];
            }

            break;

        case LOGICOP_NOOP:

            /*  Do not change destination color.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Mantein destination color.  */
                color[i] = destination[i];
            }

            break;

        case LOGICOP_XOR:

            /*  Xor source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical xor.  */
                color[i] = source[i] ^ destination[i];
            }

            break;

        case LOGICOP_OR:

            /*  Or source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical or.  */
                color[i] = source[i] | destination[i];
            }

            break;

        case LOGICOP_NOR:

            /*  Nor source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical nor.  */
                color[i] = ~(source[i] | destination[i]);
            }

            break;

        case LOGICOP_EQUIV:

            /*  Equiv (?) source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical equiv (?).  */
                color[i] = ~(source[i] ^ destination[i]);
            }

            break;

        case LOGICOP_INVERT:

            /*  Invert destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Invert destination.  */
                color[i] = ~destination[i];
            }

            break;

        case LOGICOP_OR_REVERSE:

            /*  Or reverse source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical or reverse.  */
                color[i] = source[i] | ~destination[i];
            }

            break;

        case LOGICOP_COPY_INVERTED:

            /*  Copy inverted source.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Invert source.  */
                color[i] = ~source[i];
            }

            break;

        case LOGICOP_OR_INVERTED:

            /*  Or inverted source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical or inverted.  */
                color[i] = ~source[i] | destination[i];
            }

            break;

        case LOGICOP_NAND:

            /*  Not and source and destination colors.  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Logical not and.  */
                color[i] = ~(source[i] & destination[i]);
            }

            break;

        case LOGICOP_SET:

            /*  Set color (to all 1s).  */
            for(i = 0; i < (stampFragments * 4); i++)
            {
                /*  Set byte to all 1s.  */
                color[i] = 0xff;
            }

            break;

        default:
            panic("FragmentOpEmulator", "logicOp", "Unsupported logical operation mode.");
            break;
    }
}


/*  Sets blending factor RGB components.  */
void FragmentOpEmulator::factorRGB(BlendFunction bf, QuadFloat constColor, QuadFloat *source,
    QuadFloat *destination, QuadFloat *factor)
{
    u32bit i;

    switch(bf)
    {
        case BLEND_ZERO:

            /*  Set RGB components to 0.  */
            for(i = 0; i < stampFragments; i++)
            {
                /*  Clear RGB components.  */
                factor[i][0] = 0.0;
                factor[i][1] = 0.0;
                factor[i][2] = 0.0;
            }
            break;

        case BLEND_ONE:

            /*  Set RGB components to 1.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0;
                factor[i][1] = 1.0;
                factor[i][2] = 1.0;
            }

            break;

        case BLEND_SRC_COLOR:

            /*  Use source color.  */
            for(i = 0; i < stampFragments; i++)
            {
                /*  Copy source RGB components.  */
                factor[i][0] = source[i][0];
                factor[i][1] = source[i][1];
                factor[i][2] = source[i][2];
            }

            break;

        case BLEND_ONE_MINUS_SRC_COLOR:

            /*  Use 1 - source.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0f - source[i][0];
                factor[i][1] = 1.0f - source[i][1];
                factor[i][2] = 1.0f - source[i][2];
            }

            break;

        case BLEND_DST_COLOR:

            /*  Use destination color.  */
            for(i = 0; i < stampFragments; i++)
            {
                /*  Copy destination color RGB components.  */
                factor[i][0] = destination[i][0];
                factor[i][1] = destination[i][1];
                factor[i][2] = destination[i][2];
            }

            break;

        case BLEND_ONE_MINUS_DST_COLOR:

            /*  Use 1 - destination.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0f - destination[i][0];
                factor[i][1] = 1.0f - destination[i][1];
                factor[i][2] = 1.0f - destination[i][2];
            }

            break;

        case BLEND_SRC_ALPHA:

            /*  Use source alpha component.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][0] = source[i][3];
                factor[i][1] = source[i][3];
                factor[i][2] = source[i][3];
            }

            break;

        case BLEND_ONE_MINUS_SRC_ALPHA:

            /*  Use 1 - source alpha component.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0f - source[i][3];
                factor[i][1] = 1.0f - source[i][3];
                factor[i][2] = 1.0f - source[i][3];
            }

            break;

        case BLEND_DST_ALPHA:

            /*  Use destination alpha component.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][0] = destination[i][3];
                factor[i][1] = destination[i][3];
                factor[i][2] = destination[i][3];
            }

            break;

        case BLEND_ONE_MINUS_DST_ALPHA:

            /*  Use 1 - source alpha component.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0f - destination[i][3];
                factor[i][1] = 1.0f - destination[i][3];
                factor[i][2] = 1.0f - destination[i][3];
            }

            break;

        case BLEND_CONSTANT_COLOR:

            /*  Use constant color RGB components.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][0] = constColor[0];
                factor[i][1] = constColor[1];
                factor[i][2] = constColor[2];
            }

            break;

        case BLEND_ONE_MINUS_CONSTANT_COLOR:

            /*  Use 1 - constant color RGB components.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0f - constColor[0];
                factor[i][1] = 1.0f - constColor[1];
                factor[i][2] = 1.0f - constColor[2];
            }

            break;

        case BLEND_CONSTANT_ALPHA:

            /*  Use constant color alpha component.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][0] = constColor[3];
                factor[i][1] = constColor[3];
                factor[i][2] = constColor[3];
            }

            break;

        case BLEND_ONE_MINUS_CONSTANT_ALPHA:

            /*  Use 1 - constant color alpha component.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][0] = 1.0f - constColor[3];
                factor[i][1] = 1.0f - constColor[3];
                factor[i][2] = 1.0f - constColor[3];
            }

            break;

        case BLEND_SRC_ALPHA_SATURATE:

            /*  Use alpha saturate function.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][0] = GPU_MIN(source[i][3], 1.0f - destination[i][3]);
                factor[i][1] = factor[i][0];
                factor[i][2] = factor[i][0];
            }

            break;

        default:
            panic("FragmentOpEmulator", "factorRGB", "Unsupported blend factor function.");
            break;
    }

}

/*  Sets blending factor alpha component.  */
void FragmentOpEmulator::factorAlpha(BlendFunction bf, QuadFloat constColor, QuadFloat *source,
    QuadFloat *destination, QuadFloat *factor)
{
    u32bit i;
    switch(bf)
    {
        case BLEND_ZERO:

            /*  Set alpha components to 0.  */
            for(i = 0; i < stampFragments; i++)
            {
                /*  Clear alpha component.  */
                factor[i][3] = 0.0;
            }
            break;

        case BLEND_ONE:
        case BLEND_SRC_ALPHA_SATURATE:

            /*  Set alpha components to 1.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][3] = 1.0;
            }

            break;

        case BLEND_SRC_COLOR:
        case BLEND_SRC_ALPHA:

            /*  Use source alpha component.  */
            for(i = 0; i < stampFragments; i++)
            {
                /*  Copy source alpha component.  */
                factor[i][3] = source[i][3];
            }

            break;

        case BLEND_ONE_MINUS_SRC_COLOR:
        case BLEND_ONE_MINUS_SRC_ALPHA:

            /*  Use 1 - source alpha component.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][3] = 1.0f - source[i][3];
            }

            break;

        case BLEND_DST_COLOR:
        case BLEND_DST_ALPHA:

            /*  Use destination alpha component.  */
            for(i = 0; i < stampFragments; i++)
            {
                /*  Copy destination alpha component.  */
                factor[i][3] = destination[i][3];
            }

            break;

        case BLEND_ONE_MINUS_DST_COLOR:
        case BLEND_ONE_MINUS_DST_ALPHA:

            /*  Use 1 - destination alpha.  */
            for (i = 0; i < stampFragments; i++)
            {
                factor[i][3] = 1.0f - destination[i][3];
            }

            break;

        case BLEND_CONSTANT_COLOR:
        case BLEND_CONSTANT_ALPHA:

            /*  Use constant color alpha component.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][3] = constColor[3];
            }

            break;

        case BLEND_ONE_MINUS_CONSTANT_COLOR:
        case BLEND_ONE_MINUS_CONSTANT_ALPHA:

            /*  Use 1 - constant color alpha component.  */
            for(i = 0; i < stampFragments; i++)
            {
                factor[i][3] = 1.0f - constColor[3];
            }

            break;

        default:
            panic("FragmentOpEmulator", "factorA", "Unsupported blend factor function.");
            break;
    }

}

/*  Updates the stencil value for a fragment.  */
void FragmentOpEmulator::updateStencil(StencilUpdateFunction func,
    u8bit stencilVal, u32bit &bufferVal)
{
    u8bit stencilAux;

    /*  Choose the update function.  */
    switch(func)
    {
        case STENCIL_KEEP:

            /*  Keep the same value.  */
            stencilAux = stencilVal;
            break;

        case STENCIL_ZERO:

            /*  Set to zero.  */
            stencilAux = 0;

            break;

        case STENCIL_REPLACE:

            /*  Replace with the stencil reference value.  */
            stencilAux = stencilReference;

            break;

        case STENCIL_INCR:

            /*  Increment with saturation.  */
            stencilAux = (stencilVal == 0xff)?0xff:stencilVal + 1;

            break;

        case STENCIL_DECR:

            /*  Decrement with saturation.  */
            stencilAux = (stencilVal == 0)?0:stencilVal - 1;

            break;

        case STENCIL_INVERT:

            /*  Invert/negate the stencil value.  */
            stencilAux = ~stencilVal;

            break;

        case STENCIL_INCR_WRAP:

            /*  Increment with wrapping.  */
            stencilAux = stencilVal + 1;

            break;

        case STENCIL_DECR_WRAP:

            /*  Decrement with wrapping.  */
            stencilAux = stencilVal - 1;

            break;

        default:
            panic("FragmentOpEmulator", "updateStencil", "Undefined stencil update function.");
            break;
    }


    /*  Store the new stencil value applying the stencil mask.  */
    bufferVal = (bufferVal & 0x00ffffff) |
        ((stencilAux & stencilUpdateMask) << 24);

//printf("update stencil -> func %d stencilVal %x  stencilAux %x  bufferVal %x stencilUpdateMask %x\n",
//    func, stencilVal, stencilAux, bufferVal, stencilUpdateMask);

}

/*  Compares a byte array (stencil vales) against a reference byte value (stencil reference).  */
void FragmentOpEmulator::compare(CompareMode func, u8bit *value,
    u8bit ref, bool *result)
{
    u32bit i;

//printf("FragmentOpEmulator::compare -> func %d value %p ref %x result %p stencilTest %s\n", func, value, ref, result,
//    stencilTest?"T":"F");

    /*  Test each fragment stencil against the reference value.  */
    for(i = 0; i < stampFragments; i++)
    {
        /*  Compare reference and input value.  */
        result[i] = compare(func, value[i], ref) || !stencilTest;
    }
}

/*  Compare two integer arrays (z values).  */
void FragmentOpEmulator::compare(CompareMode func, u32bit *value,
    u32bit *ref, bool *result)
{
    u32bit i;

    /*  Test each fragment z with stored z.  */
    for(i = 0; i < stampFragments; i++)
    {
        /*  Compare value with reference.  */
        result[i] = compare(func, value[i], ref[i]) || !depthTest;
    }
}

/*  Compare an input value with a reference byte.  */
bool FragmentOpEmulator::compare(CompareMode func, u8bit value, u8bit ref)
{
    switch(func)
    {
        case GPU_NEVER:

            /*  Always return false.  */
            return FALSE;

            break;

        case GPU_ALWAYS:

            /*  Always return true.  */
            return TRUE;

            break;

        case GPU_LESS:

            /*  If value is less than the reference.  */
            return ((ref & stencilTestMask) < (value & stencilTestMask));

            break;


        case GPU_LEQUAL:

            /*  If value is less or equal than the reference.  */
            return ((ref & stencilTestMask) <= (value & stencilTestMask));

            break;

        case GPU_EQUAL:

            /*  If value is equal to reference.  */
            return ((ref & stencilTestMask) == (value & stencilTestMask));

            break;

        case GPU_GEQUAL:

//printf("compare(stencil) => mask (%x) ref(%x) >= value(%x) -> %s\n", stencilTestMask, ref, value, ((ref & stencilTestMask) >= (value & stencilTestMask))?"T":"F");
            /*  If value is greater or equal.  */
            return ((ref & stencilTestMask) >= (value & stencilTestMask));

            break;

        case GPU_GREATER:

//printf("compare(stencil) => mask (%x) | ref(%x) > value(%x) -> %s\n", stencilTestMask, ref, value, ((ref & stencilTestMask) > (value & stencilTestMask))?"T":"F");
            /*  If value is greater than reference.  */
            return ((ref & stencilTestMask) > (value & stencilTestMask));

            break;

        case GPU_NOTEQUAL:

            /*  If value and reference are not equal.  */
            return ((ref & stencilTestMask) != (value & stencilTestMask));

            break;

        default:
            panic("FragmentOpEmulator", "compare", "Undefined compare mode.");
            break;
    }
    return false;
}

/*  Compare two 32 bit integers.  */
bool FragmentOpEmulator::compare(CompareMode func, u32bit value, u32bit ref)
{
    switch(func)
    {
        case GPU_NEVER:

            /*  Always return false.  */
            return FALSE;

            break;

        case GPU_ALWAYS:

            /*  Always return true.  */
            return TRUE;

            break;

        case GPU_LESS:

            /*  If value is less than the reference.  */
            return (value < ref);

            break;


        case GPU_LEQUAL:

//printf("LEQUAL value(%x) <= ref(%x) -> %s\n", value, ref, (value <= ref)?"T":"F");

            /*  If value is less or equal than the reference.  */
            return (value <= ref);

            break;

        case GPU_EQUAL:

//printf("EQUAL value(%x) == ref(%x) -> %s\n", value, ref, (value == ref)?"T":"F");

            /*  If value is equal to reference.  */
            return (value == ref);

            break;

        case GPU_GEQUAL:

            /*  If value is greater or equal.  */
            return (value >= ref);

            break;

        case GPU_GREATER:

            /*  If value is greater than reference.  */
            return (value > ref);

            break;

        case GPU_NOTEQUAL:

            /*  If value and reference are not equal.  */
            return (value != ref);

            break;

        default:
            panic("FragmentOpEmulator", "compare", "Undefined compare mode.");
            break;
    }
    return false;
}

/*  Calculates the z and z-stencil maximum value.  */
void FragmentOpEmulator::blockMaxZ(u32bit *input, u32bit size, u32bit &maxZ)
{
    /*  Initial Z maximum is the minimum 24 bit value.  */
    maxZ = 0x00000000L;

    /*  Search for the min and max z/stencil values.  */
    for(u32bit i = 0; i < size; i++)
    {
        /*  Get the maximum Z value.  */
        u32bit d = input[i] & 0x00ffffff;
        maxZ = d > maxZ ? d : maxZ;
    }
}

/*  Calculates the z and z-stencil minimum and maximum.  */
void FragmentOpEmulator::blockMinMaxZ(u32bit *input, u32bit size,
    u32bit &minZ, u32bit &maxZ, u32bit &min, u32bit &max)
{
    u32bit i;

    /*  Initial minimum is the maximum 32 bit value.  */
    min = 0xffffffffL;

    /*  Initial maximum is the minimum 32 bit value.  */
    max = 0x00000000L;

    /*  Initial Z minimum is the maximum 24 bit value.  */
    minZ = 0x00ffffffL;

    /*  Initial Z maximum is the minimum 24 bit value.  */
    maxZ = 0x00000000L;

    /*  Search for the min and max z/stencil values.  */
    for(i = 0; i < size; i++)
    {
        /*  Get the minimun value.  */
        min = (input[i] < min)?input[i]:min;

        /*  Get the maximum value.  */
        max = (input[i] > max)?input[i]:max;

        /*  Get the minimun Z value.  */
        minZ = ((input[i] & 0x00ffffff) < minZ)?(input[i] & 0x00ffffff):minZ;

        /*  Get the maximum Z value.  */
        maxZ = ((input[i] & 0x00ffffff) > maxZ)?(input[i] & 0x00ffffff):maxZ;
    }
}

/*  Calculates the block minimum and maximum.  */
void FragmentOpEmulator::blockMinMax(u32bit *input, u32bit size,
    u32bit &min, u32bit &max)
{
    u32bit i;

    /*  Initial minimum is the maximum 32 bit value.  */
    min = 0xffffffffL;

    /*  Initial maximum is the minimum 32 bit value.  */
    max = 0x00000000L;

    /*  Search for the min and max values.  */
    for(i = 0; i < size; i++)
    {
        /*  Get the minimun value.  */
        min = (input[i] < min)?input[i]:min;

        /*  Get the maximum value.  */
        max = (input[i] > max)?input[i]:max;
    }
}

/*  Compresses a block of pixel values.  */
FragmentOpEmulator::CompressionMode FragmentOpEmulator::hiloCompress(
    u32bit *input, u8bit *output,
    u32bit size, u32bit hiMaskL0, u32bit hiMaskL1,
    u32bit loShiftL0, u32bit loShiftL1, u32bit loMaskL0,
    u32bit loMaskL1, u32bit min, u32bit max)
{
    u32bit i;
    u32bit aL0, bL0, aL1, bL1;
    ReferenceValue flagL0;
    ReferenceValue flagL1;
    u32bit byteOff;
    u32bit bitOff;
    u16bit loBitsL0[MAX_COMPR_FRAGMENTS];
    u8bit loBitsL1[MAX_COMPR_FRAGMENTS];
    bool level0;
    bool level1;
    u32bit aux;

    /*  Initial minimum is the maximum 32 bit value.  */
    //min = 0xffffffffL;

    /*  Initial maximum is the minimum 32 bit value.  */
    //max = 0x00000000L;

    /*  Search for the min and max z/stencil values.  */
    //for(i = 0; i < size; i++)
    //{
        /*  Get the minimun value.  */
        //min = (input[i] < min)?input[i]:min;

        /*  Get the maximum value.  */
        //max = (input[i] > max)?input[i]:max;
    //}

    /*  Calculate the two remaining reference values.  */
    aL0 = min + (1 << loShiftL0);
    bL0 = max - (1 << loShiftL0);
    aL1 = min + (1 << loShiftL1);
    bL1 = max - (1 << loShiftL1);

    /*  Compression level admited by the line.  */
    level0 = TRUE;
    level1 = TRUE;

//printf("==> MIN %x MAX %x L0 A %x B %x L1 A %x B %x\n", min, max, aL0, bL0, aL1, bL1);

    //printf("hiloCompress => min = %08x | max = %08x | aL0 = %08x | bL0 = %08x | aL1 = %08x | bL1 = %08x\n",
    //    min, max, aL0, bL0, aL1, bL1);
    //printf("hiloCompress => hiMaskL0 = %08x | loShiftL0 = %d | loMaskL0 = %08x | hiMaskL1 = %08x | loSfhitL1 = %d | loMaskL1 = %08x\n",
    //    hiMaskL0, loMaskL0, hiMaskL1, loShiftL1, loMaskL1);
                
    /*  Create the vectors of low bits from the reference values.  */
    for(i = 0; (i < size) && (level0 || level1); i++)
    {
    
        /*  Check level 0 compression level.  */
        if (level0)
        {
            /*  Test if current value is compressable with level 0.  */
            level0 = testCompression(aL0, bL0, min, max, hiMaskL0, input[i], flagL0);
        }

        /*  Check level 1 compression.  */
        if (level1)
        {
            /*  Test if current value is compressable at level 1.  */
            level1 = testCompression(aL1, bL1, min, max, hiMaskL1, input[i], flagL1);
        }

        /*  Check if level 0 compression is enabled.  */
        if (level0)
        {
            /*  Get value low bits for level 0.  */
            loBitsL0[i] = ((flagL0 << loShiftL0) | (input[i] & loMaskL0)) << (16 - 2 - loShiftL0);
        }

        /*  Check if level 1 compression is enabled.  */
        if (level1)
        {
            /*  Get value low bits for level 1.  */
            loBitsL1[i] = ((flagL1 << loShiftL1) | (input[i] & loMaskL1)) << (8 - 2 - loShiftL1);
        }
        //printf(" <<< input = %08x | level0 = %s | level1 = %s | flagL0 = %02x | flagL1 = %02x\n", input[i],
        //    level0 ? "T" : "F", level1 ? "T" : "F", flagL0, flagL1);

    }

    /*  Reset current compression offset.  */
    byteOff = 8;
    bitOff = 0;

    /*  Write reference values first (little endian).  */
    *((u32bit *) &output[0]) = min;
    *((u32bit *) &output[4]) = max;

    /*  Reset encode register.  */
    aux = 0;

//printf("==> Output %x %x %x %x  %x %x %x %x\n", output[0], output[1], output[2], output[3],
//    output[4], output[5], output[6], output[7]);

    /*  Check which compression level to use.  */
    if (level1)
    {
//printf("==> Level 1\n");

        /*  Encode the output as level 1 compression.  */
        for(i = 0; i < size; i++)
        {
            /*  Add low bits and reference flag of the value to the bit stream.  */
            aux = aux | (loBitsL1[i] << (32 - 8 - bitOff));

            /*  Update bit offset.  */
            bitOff = bitOff + loShiftL1 + 2;

            /*  Check if current bytes has been fully written.  */
            if ((bitOff > 7) || ((i + 1) == size))
            {
                /*  Module 8.  */
                bitOff = bitOff & 0x07;

                /*  Write top byte to the output (little endian).  */
                output[byteOff] = ((u8bit *) &aux)[3];

//printf("==> Output %d : %x\n", byteOff, output[byteOff]);

                /*  Shift bitstream a byte.  */
                aux = aux << 8;

                /*  Move to the next byte.  */
                byteOff++;
            }
        }

        /*  Return compression level 1.  */
        return COMPR_L1;
    }
    else if (level0)
    {
//printf("==> Level 0\n");

        /*  Encode the output as level 2 compression.  */
        for(i = 0; i < size; i++)
        {
            /*  Store two values.  */
            aux = aux | (loBitsL0[i] << (32 - 16 - bitOff));

            /*  Update bit offset.  */
            bitOff = bitOff + loShiftL0 + 2;

            /*  Check if current bytes has been fully written.  */
            if ((bitOff > 15) || ((i + 1) == size))
            {
                /*  Module 16.  */
                bitOff = bitOff & 0x0f;

                /*  Write top two bytes to the output (little endian).  */
                output[byteOff] = ((u8bit *) &aux)[3];
                output[byteOff + 1] = ((u8bit *) &aux)[2];

//printf("==> Output %d : %x %x\n", output[byteOff], output[byteOff + 1]);

                /*  Shift bitstream two bytes.  */
                aux = aux << 16;

                /*  Move to the next word.  */
                byteOff += 2;
            }
        }

//printf("==> Output bytes %d\n", byteOff);

        /*  Return compression level 0.  */
        return COMPR_L0;
    }
    else
    {
        /*  Just copy the input.  No compression level.  */
        for(i = 0; i < size; i++)
            *((u32bit *) &output[i * 4]) = input[i];

        /*  Return uncompressable block.  */
        return UNCOMPRESSED;
    }

}

/*  Tests if a value is compressable for a given compression level (mask).  */
bool FragmentOpEmulator::testCompression(u32bit a, u32bit b, u32bit min,
    u32bit max, u32bit hiMask, u32bit value, FragmentOpEmulator::ReferenceValue &ref)
{
    bool compressable;

    compressable = TRUE;

    /*  Check with reference values.  */
    if ((a & hiMask) == (value & hiMask))
    {
        /*  Set A as reference value for the current value.  */
        ref = REF_A;
    }
    else if ((b & hiMask) == (value & hiMask))
    {
        /*  Set B as reference value for the current value.  */
        ref = REF_B;
    }
    else if ((min & hiMask) == (value & hiMask))
    {
        /*  Set MIN as reference value for the current value.  */
        ref = REF_MIN;
    }
    else if ((max & hiMask) == (value & hiMask))
    {
        /*  Set MAX as reference value for the current value.  */
        ref = REF_MAX;
    }
    else
    {
        /*  Set as not compressable.  */
        compressable = FALSE;
    }

    return compressable;
}

/*  Uncompresses a block.  */
void FragmentOpEmulator::hiloUncompress(u8bit *input, u32bit *output,
    u32bit size, CompressionMode level, u32bit hiMaskL0, u32bit hiMaskL1,
    u32bit loShiftL0, u32bit loShiftL1)
{
    u32bit ref[4];
    u32bit min;
    u32bit max;
    u32bit a;
    u32bit b;
    u32bit i;
    u32bit bitOff;
    u32bit byteOff;
    u8bit flag;
    u32bit lowBits;
    u32bit aux;

    /*  Check compression level of the input.  */
    switch (level)
    {
        case UNCOMPRESSED:

            /*  Just copy input to output.  */
            for(i = 0; i < size; i++)
            {
                output[i] = *((u32bit *) &input[i * 4]);
            }

            break;

        case COMPR_L1:

            /*  Decompress level 1 stream.  */

//printf(">>> Input %x %x %x %x  %x %x %x %x\n", input[0], input[1], input[2], input[3],
//    input[4], input[5], input[6], input[7]);

            /*  Get min reference value.  */
            min = *((u32bit *) &input[0]);

            /*  Get max reference value.  */
            max = *((u32bit *) &input[4]);

            /*  Calculate a reference value.  */
            a = min + (1 << loShiftL1);

            /*  Calculate b reference value.  */
            b = max - (1 << loShiftL1);

            /*  Build high bit reference array.  */
            ref[REF_MIN] = min & hiMaskL1;
            ref[REF_MAX] = max & hiMaskL1;
            ref[REF_A] = a & hiMaskL1;
            ref[REF_B] = b & hiMaskL1;

//printf("Ref MIN %x\n", ref[REF_MIN]);
//printf("Ref MAX %x\n", ref[REF_MAX]);
//printf("Ref A %x\n", ref[REF_A]);
//printf("Ref B %x\n", ref[REF_B]);

            /*  Reset byte offset.  */
            byteOff = 12;

            /*  Reset bit offset.  */
            bitOff = 0;

            /*  Get 4 compressed bitstream bytes.  */
            aux = (input[8] << 24) | (input[9] << 16) | (input[10] << 8) | input[11];

            /*  Reconstruct all the values.  */
            for(i = 0; i < size; i++)
            {
                /*  Reference value flag.  */
                flag = aux >> (32 - 2);

//printf("===>> Flag %d\n", flag);

                /*  Drop current value reference flag bits from bitstream.  */
                aux = aux << 2;

                /*  Get current value low bits.  */
                lowBits = aux >> (32 - loShiftL1);

//printf("===>> Low bits %x\n", lowBits);

                /*  Drop current low bits from the bitstream.  */
                aux = aux << loShiftL1;

                /*  Update number of stream bits removed.  */
                bitOff += loShiftL1 + 2;

                /*  Check if a full byte has been removed.  */
                if ((bitOff > 7) && ((i + 1) < size))
                {
                    /*  Module 8.  */
                    bitOff = bitOff & 0x07;

                    /*  Read next byte from the input stream.  */
                    aux = aux | (((u32bit) input[byteOff]) << bitOff);

                    /*  Update pointer to the next byte in the input stream.  */
                    byteOff++;
                }

                /*  Combine with corresponding high bits.  */
                output[i] = ref[flag] | lowBits;

//printf("===> Output %d : %x\n", i, output[i]);
            }


            break;

        case COMPR_L0:

            /*  Decompress level 0 stream.  */

            /*  Get min reference value.  */
            min = *((u32bit *) &input[0]);

            /*  Get max reference value.  */
            max= *((u32bit *) &input[4]);

            /*  Calculate a reference value.  */
            a = min + (1 << loShiftL0);

            /*  Calculate b reference value.  */
            b = max - (1 << loShiftL0);

            /*  Build high bit reference array.  */
            ref[REF_MIN] = min & hiMaskL0;
            ref[REF_MAX] = max & hiMaskL0;
            ref[REF_A] = a & hiMaskL0;
            ref[REF_B] = b & hiMaskL0;

//printf("Ref MIN %x\n", ref[REF_MIN]);
//printf("Ref MAX %x\n", ref[REF_MAX]);
//printf("Ref A %x\n", ref[REF_A]);
//printf("Ref B %x\n", ref[REF_B]);

            /*  Reset byte offset.  */
            byteOff = 12;

            /*  Reset bit offset.  */
            bitOff = 0;

            /*  Get 4 compressed bitstream bytes.  */
            aux = (input[8] << 24) | (input[9] << 16) | (input[10] << 8) | input[11];

            /*  Reconstruct all the values.  */
            for(i = 0; i < size; i++)
            {

                /*  Reference value flag.  */
                flag = aux >> (32 - 2);

//printf("===>> Flag %d\n", flag);

                /*  Drop current value reference flag bits from bitstream.  */
                aux = aux << 2;

                /*  Get current value low bits.  */
                lowBits = aux >> (32 - loShiftL0);

//printf("===>> Low bits %x\n", lowBits);

                /*  Drop current low bits from the bitstream.  */
                aux = aux << loShiftL0;

                /*  Update number of stream bits removed.  */
                bitOff += loShiftL0 + 2;

                /*  Check if 2 bytes have been removed.  */
                if ((bitOff > 15) && ((i + 1) < size))
                {
                    /*  Module 16.  */
                    bitOff = bitOff & 0x0f;

                    /*  Read next two bytes from the input stream.  */
                    aux = aux | (((u32bit) input[byteOff]) << (8 + bitOff)) | (((u32bit) input[byteOff + 1]) << bitOff);

                    /*  Update pointer to the next byte in the input stream.  */
                    byteOff += 2;
                }

                /*  Combine with corresponding high bits.  */
                output[i] = ref[flag] | lowBits;

//printf("===> Output %d : %x\n", i, output[i]);
            }


            break;

        default:
            panic("FragmentOpEmulator", "hiloUncompress", "Undefined compression level.");
            break;
    }
}



void FragmentOpEmulator::compressionSwizzle(u32bit *input, u32bit *output, u32bit size, u32bit regions,
                                            u32bit *inShift, u32bit *inMask, u32bit *outShift)
{
    u32bit regionData;
    u32bit inData;
    u32bit swizzledData;
    
    for(u32bit i = 0; i < size; i++)
    {
        //  Read input data.
        inData = input[i];
        
        //  Clean the swizzled data.
        swizzledData = 0;

        //printf(">> inData = %08x\n", inData);

        //  Swizzle each of the regions from the input data word.        
        for(u32bit r = 0; r < regions; r++)
        {
            //printf(" -- region %d inshift = %d mask = %08x outShift = %d\n", r, inShift[r], inMask[r], outShift[r]);

            //  Shift to the right to align the bit region.
            regionData = inData >> inShift[r];
            
            //  Mask to remove bits that aren't in the bit region
            regionData = regionData & inMask[r];

            //  Shift to the left to position bit region in the swizzled position.
            regionData = regionData << outShift[r];
            
            //  OR the bit region with the swizzled output word.
            swizzledData = swizzledData | regionData;

            //printf(" -- regionData = %08x swizzledData = %08x\n", regionData, swizzledData);
        }
        
        //  Write swizzled data.
        output[i] = swizzledData;
    }
}
