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
 * $RCSfile: FragmentOpEmulator.h,v $
 * $Revision: 1.8 $
 * $Author: christip $
 * $Date: 2008-02-25 22:32:12 $
 *
 * Fragment Operation Emulator class definition file.
 *
 */

/**
 *
 *  @file FragmentOpEmulator.h
 *
 *  This file defines the Fragment Operation Emulator class.
 *
 *  This class defines functions that perform the operations
 *  in fragments after the pixel shader (Z test, stencil test,
 *  blend, logical operation).
 *
 */

#ifndef _FRAGMENTOPEMULATOR_

#define _FRAGMENTOPEMULATOR_

#include "GPUTypes.h"
#include "GPU.h"

namespace gpu3d
{

/*  Defines the maximum number of pixels for a block to compress.  */
static const u32bit MAX_COMPR_FRAGMENTS = 128;

/**
 *
 *  This class implements emulation functions for the fragment operation
 *  boxes: Z Test and Color Write.
 *
 */


class FragmentOpEmulator
{

private:

    /**
     *
     *  Defines the reference value identifiers for HILO compression
     *  method.
     *
     */
    enum ReferenceValue
    {
        REF_MIN,    /**<  Minimum block value as reference.  */
        REF_MAX,    /**<  Maximum block value as reference.  */
        REF_A,      /**<  Minimum + 1 as reference.  */
        REF_B       /**<  Maximum - 1 as reference.  */
    };

    u32bit stampFragments;      /**<  Number of fragments per stamp.  */

    /*  Stencil test parameters.  */
    bool stencilTest;               /**<  Enable/Disable Stencil test flag.  */
    CompareMode stencilFunction;    /**<  Stencil test comparation function.  */
    u8bit stencilReference;         /**<  Stencil reference value for the test.  */
    u8bit stencilTestMask;          /**<  Stencil mask for the stencil operands test.  */
    u8bit stencilUpdateMask;        /**<  Stencil mask for the stencil update.  */
    StencilUpdateFunction stencilFail;  /**<  Update function when stencil test fails.  */
    StencilUpdateFunction depthFail;    /**<  Update function when depth test fails.  */
    StencilUpdateFunction depthPass;    /**<  Update function when depth test passes.  */

    /*  Depth test parameters.  */
    bool depthTest;                 /**<  Enable/Disable depth test flag.  */
    CompareMode depthFunction;      /**<  Depth test comparation function.  */
    bool depthMask;                 /**<  If depth buffer update is enabled or disabled.  */

    /*  Blend parameters.  */
    BlendEquation equation[MAX_RENDER_TARGETS];     /**<  Current blending equation mode.  */
    BlendFunction srcRGB[MAX_RENDER_TARGETS];       /**<  Source RGB weight funtion.  */
    BlendFunction dstRGB[MAX_RENDER_TARGETS];       /**<  Destination RGB weight function.  */
    BlendFunction srcAlpha[MAX_RENDER_TARGETS];     /**<  Source alpha weight function.  */
    BlendFunction dstAlpha[MAX_RENDER_TARGETS];     /**<  Destination alpha weight function.  */
    QuadFloat constantColor[MAX_RENDER_TARGETS];    /**<  Blend constant color.  */

    /*  Logic operation parameters.  */
    LogicOpMode logicOpMode;    /**<  Current logic operation mode.  */

    /*  Fragment operator buffers.  */
    u8bit *stencil;             /**<  Pointer to the array storing the stamp stencil values.  */
    u32bit *depth;              /**<  Pointer to the array storing the stamp depth values in the Z buffer.  */
    bool *stencilResult;        /**<  Pointer to the array storing the results of the stencil test for the stamp.  */
    bool *zResult;              /**<  Pointer to the array storing the results of the z test for the stamp.  */
    QuadFloat *sFactor;         /**<  Pointer to the array for the source blend factor.  */
    QuadFloat *dFactor;         /**<  Pointer to the array for the destination blend factor.  */


    /*  Private functions.  */

    /**
     *
     *  Creates the blend factor for RGB components.
     *
     *  @param function The blend factor function.
     *  @param constantColor The constant blend color.
     *  @param source Pointer to the source color array.
     *  @param destination Pointer to the destination color array.
     *  @param factor Pointer to the factor array.
     *
     */

    void factorRGB(BlendFunction function, QuadFloat constantColor, QuadFloat *source, QuadFloat *destination, QuadFloat *factor);

    /**
     *
     *  Creates the blend factor for alpha component.
     *
     *  @param function The blend factor function.
     *  @param constantColor The constant blend color.
     *  @param source Pointer to the source color array.
     *  @param destination Pointer to the destination color array.
     *  @param factor Pointer to the factor array.
     *
     */

    void factorAlpha(BlendFunction function, QuadFloat constantColor, QuadFloat *source,
        QuadFloat *destination, QuadFloat *factor);


    /**
     *
     *  Updates the stencil values for a fragment.
     *
     *  @param func Stencil update function.
     *  @param stencilVal Current stencil value of the fragment.
     *  @param bufferVal Reference to the current value stored in
     *  the z/stencil buffer for the value.  The updated stencil value will
     *  be stored here.
     *
     */

    void updateStencil(StencilUpdateFunction func, u8bit stencilVal,
        u32bit &bufferVal);

    /**
     *
     *  Compares the stencil values for a stamp against the reference
     *  stencil value and returns an array with the result.
     *
     *  @param func The stencil compare function.
     *  @param value Pointer to an array with the stamp stencil values.
     *  @param ref Stencil reference value.
     *  @param result Pointer to a boolen array where to store the
     *  results of the stencil test.
     */

    void compare(CompareMode func, u8bit *value, u8bit ref, bool *result);

    /**
     *
     *  Compares the stamp z values against the z values stored in the
     *  framebuffer.
     *
     *  @param func The z test compare function.
     *  @param value Pointer to an array storing the stamp z values.
     *  @param ref Pointer to an array storing the stamp z buffer values.
     *  @param result Pointer to a boolean array where to store the results
     *  of the z test.
     *
     */

    void compare(CompareMode func, u32bit *value, u32bit *ref, bool *result);

    /**
     *
     *  Compares two unsigned byte integer values.
     *
     *  @param func Compare function to use.
     *  @param val Value to compare.
     *  @param ref Reference value to compare with.
     *
     *  @result The result of the compare operation.
     *
     */

    bool compare(CompareMode func, u8bit value, u8bit ref);

    /**
     *
     *  Compares two unsigned 32 bit integer values.
     *
     *  @param func Compare function to use.
     *  @param val Value to compare.
     *  @param ref Reference value to compare with.
     *
     *  @result The result of the compare operation.
     *
     */

    bool compare(CompareMode func, u32bit value, u32bit ref);

    /**
     *
     *  Test if a given value can be compressed using the given reference
     *  values.
     *
     *  @param a Reference value.
     *  @param b Reference value.
     *  @param min Reference value.
     *  @param max Reference value.
     *  @param hiMask Bitmask for the high bits of the reference values.
     *  @param value The value to associate with one of the reference
     *  values and determine if it is compressable.
     *  @param ref Reference to a variable where to store the identifier
    *  of the reference value for the value.
     *
     *  @return If the value can be associated with one of the four
     *  reference values (is compressable) return TRUE, otherwise
     *  return FALSE.
     *
     */

    static bool testCompression(u32bit a, u32bit b, u32bit min,
        u32bit max, u32bit hiMask, u32bit value, ReferenceValue &ref);


public:

    /**
     *
     *  Defines the different compression levels of a block of pixels.
     *
     */

    enum CompressionMode
    {
        UNCOMPRESSED,   /**<  Uncompressed mode.  */
        COMPR_L0,       /**<  Compression level 0.  */
        COMPR_L1        /**<  Compression level 1.  */
    };

    /**
     *
     *  Fragment Operation Emulator constructor.
     *
     *  @param stampFrags Number of fragments per stamp.
     *
     *  @return A new fragment operation emulator object.
     *
     */

    FragmentOpEmulator(u32bit stampFrags);


    /**
     *
     *  Sets the number of bytes per fragment.
     *
     *  @param bytesFrag Number of bytes per fragment.
     *
     */

    void setFragmentBytes(u32bit bytesFrag);

    /**
     *
     *  Enables or disables Z test.
     *
     *  @param enable If the stencil test is enabled.
     *
     */

    void setZTest(bool enable);

    /**
     *
     *  Enables or disables Stencil test.
     *
     *  @param enable If the stencil test is enabled.
     *
     */

    void setStencilTest(bool enable);

    /**
     *
     *  Configure Z test parameters.
     *
     *  @param zFuntion Z test compare function.
     *  @param depthMask Masks Z value update.
     *
     */

    void configureZTest(CompareMode zFunction, bool zMask);

    /**
     *
     *  Configure Stencil test parameters.
     *
     *  @param stencilFunc The stencil compare function.
     *  @param ref Reference stencil value.
     *  @param testMask Stencil mask for compare operation.
     *  @param updateMask Stencil mask for updates.
     *  @param stencilFail Stencil update function for stencil fail case.
     *  @param zFail Stencil update function for z test fail case.
     *  @param zPass Stencil update function for z test pass case.
     *
     */

    void configureStencilTest(CompareMode stencilFunc, u8bit ref,
        u8bit testMask, u8bit updateMask, StencilUpdateFunction stencilFail,
        StencilUpdateFunction zFail, StencilUpdateFunction zPass);

    /**
     *
     *  Sets blending parameters.
     *
     *  @param rt The render target identifier which blending parameters are to be updated.
     *  @param eq The blend equation mode.
     *  @param sRGB The source RGB components blend mode.
     *  @param sA The source alpha component blend mode.
     *  @param dRGB The destination RGB components blend mode.
     *  @param dA The destination alpha component blend mode.
     *  @param color The blend constant color.
     *
     */

    void setBlending(u32bit rt, BlendEquation eq, BlendFunction sRGB, BlendFunction sA,
                     BlendFunction dRGB, BlendFunction dA, QuadFloat color);

    /**
     *
     *  Sets the logical operation mode.
     *
     *  @param mode The logical operation mode to set.
     *
     */

    void setLogicOpMode(LogicOpMode mode);

    /**
     *
     *  Blends the fragments in a stamp with the pixels from the
     *  render target.
     *
     *  @param rt The render target for which to blend fragments.
     *  @param color Pointer to the buffer where the blended stamp fragments
     *  color is going to be written.
     *  @param source Pointer to a buffer with the stamp fragments color.
     *  @param destination Pointer to a buffer with the render target pixels
     *  color.
     *
     */

    void blend(u32bit rt, QuadFloat *color, QuadFloat *source, QuadFloat *destination);

    /**
     *
     *  Combines incoming stamp fragment color values with the color buffer
     *  pixel color values using the defined logical operation mode.
     *
     *  @param color Pointer to the buffer where the blended stamp fragments
     *  color is going to be written.
     *  @param source Pointer to a buffer with the stamp fragments color.
     *  @param destination Pointer to a buffer with the color buffer pixels
     *  color.
     *
     */

    void logicOp(u8bit *color, u8bit *source, u8bit *destination);

    /**
     *
     *  Performs the Stencil and Z tests and updates the stencil
     *  and Z values for the processed stamp.
     *
     *  @param stampZ Pointer to a buffer storing the z values for
     *  the stamp fragments.
     *  @param bufferZ Pointer to a buffer storing the stencil
     *  and Z values stored for the pixels to be tested against the
     *  stamp fragment.  After the stencil and z operations have
     *  been performed this buffer will store updated values.
     *  @param stampCull Pointer to a buffer storing which fragments
     *  in the stamp must be culled.  After the stencil and z operations
     *  have been performed the buffer will store the update cull
     *  fragments.
     *
     */

    void stencilZTest(u32bit *stampZ, u32bit *bufferZ, bool *stampCull);

    /**
     *
     *  Swizzles the bits of the 32-bit words in the input block of data based on a list
     *  of ranges.
     *
     *  @param input Pointer to the data block with the input data.
     *  @param output Pointer to the data block where to store the swizzled data.
     *  @param size Number of 32-bit words in the input block of data.
     *  @param regions Number of regions inside the 32-bit words to swizzle.
     *  @param inShift Pointer to an array with the the shift (right) that is to be applied to an input
     *  32-bit word to align the data for a bit region.
     *  @param inMask Pointer to an array with the mask that is to be applied to an input 32-bit word
     *  to select the bits for a bit region.
     *  @param outShift Pointer to an array with the shift (left) that is to be applied to a bit region
     *  to get placed in the proper position inside the output 32-bit word.
     *
     */
    static void compressionSwizzle(u32bit *input, u32bit *output, u32bit size, u32bit regions,
                              u32bit *inShift, u32bit *inMask, u32bit *outShift);

    /**
     *
     *  Compresses an input value stream (32 bit) using the HILO method.
     *
     *  @param input Pointer to the input values.
     *  @param output Pointer to a byte array where to store the compressed stream.
     *  @param size Number of values in the stream.
     *  @param hiMaskL0 Mask for high reference bits for level 0 compression.
     *  @param hiMaskL1 Mask for high reference bits for level 1 compression.
     *  @param loShiftL0 Number of low bits per compressed value for level 0 compression.
     *  @param loShiftL1 Number of low bits per compressed value for level 1 compression.
     *  @param loMaskL0 Bitmask for the value low bits for level 0 compression.
     *  @param loMaskL1 Bitmask for the value low bits for level 1 compression.
     *  @param min Input minimum value.
     *  @param max Input maximum value.
     *
     *  @return Compression level of the output stream.
     *
     */

    static CompressionMode hiloCompress(u32bit *input, u8bit *output,
        u32bit size, u32bit hiMaskL0, u32bit hiMaskL1,
        u32bit loShiftL0, u32bit loShiftL1, u32bit loMaskL0,
        u32bit loMaskL1, u32bit min, u32bit max);

    /**
     *
     *  Calculates the minimum and maximum 32 bit integer values in a
     *  block.
     *
     *  @param input Pointer to a block of 32 bit integer values.
     *  @param size The number of values in the block.
     *  @param min Reference to a variable where to store the block
     *  minimum value.
     *  @param max Reference to a variable where to store the block
     *  maximum value.
     *
     */

    static void blockMinMax(u32bit *input, u32bit size, u32bit &min, u32bit &max);

    /**
     *
     *  Calculates the minimum and maximum 32 bit integer values in a
     *  block and the minimum and maximum 24 bit Z value in a block.
     *
     *  @param input Pointer to a block of 32 bit integer values.
     *  @param size The number of values in the block.
     *  @param min Reference to a variable where to store the block
     *  24-bit Z minimum value.
     *  @param max Reference to a variable where to store the block
     *  24 bit Z maximum value.
     *  @param min Reference to a variable where to store the block
     *  minimum value.
     *  @param max Reference to a variable where to store the block
     *  maximum value.
     *
     */

    static void blockMinMaxZ(u32bit *input, u32bit size, u32bit &minZ, u32bit &maxZ,
        u32bit &min, u32bit &max);

    /**
     *
     *  Calculates the maximum 24 bit Z value in a block.
     *
     *  @param input Pointer to a block of 32 bit integer values.
     *  @param size The number of values in the block.
     *  @param max Reference to a variable where to store the block
     *  24 bit Z maximum value.
     *
     */
    
    static void blockMaxZ(u32bit *input, u32bit size, u32bit &maxZ);
    
    /**
     *
     *  Decompresses an input stream using HILO compressing method.
     *
     *  @param input Pointer to the input stream.
     *  @param output Pointer to the array where to store the uncompressed
     *  values.
     *  @param size Number of values in the stream.
     *  @param level Compression level of the input stream.
     *  @param hiMaskL0 Mask for high reference bits for level 0 compression.
     *  @param hiMaskL1 Mask for high reference bits for level 1 compression.
     *  @param loShiftL0 Number of low bits per compressed value for level 0 compression.
     *  @param loShiftL1 Number of low bits per compressed value for level 1 compression.
     *
     */

    static void hiloUncompress(u8bit *input, u32bit *output, u32bit size, CompressionMode level,
        u32bit hiMaskL0, u32bit hiMaskL1, u32bit loShiftL0, u32bit loShiftL1);

};

} // namespace gpu3d

#endif
