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
 * $RCSfile: TriangleSetup.h,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:48 $
 *
 * Triangle Setup box definition file.
 *
 */


/**
 *
 *  @file TriangleSetup.h
 *
 *  This file defines the Triangle Setup box.
 *
 *
 */

#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
#include "Rasterizer.h"
#include "RasterizerCommand.h"
#include "RasterizerEmulator.h"
#include "TriangleSetupInput.h"
#include "TriangleSetupOutput.h"
#include <string>

#ifndef _TRIANGLESETUP_

#define _TRIANGLESETUP_

namespace gpu3d
{

/**  Defines the different Triangle Setup states.  */
enum TriangleSetupState
{
    TS_READY,
    TS_FULL,
};

/**
 *
 *  This class implements the Triangle Setup box.
 *
 *  This class implements the Triangle Setup box that
 *  simulates the Triangle Setup unit of a GPU.
 *
 *  Inherits from the Box class that offers basic simulation
 *  support.
 *
 */

class TriangleSetup : public Box
{

private:

    /*  Triangle Setup parameters.  */
    RasterizerEmulator &rastEmu;       /**<  Reference to the rasterizer emulator used by Triangle Setup.  */
    u32bit trianglesCycle;             /**<  Number of triangles per cycle received and sent.  */
    u32bit setupFIFOSize;              /**<  Size (triangles) of the setup FIFO.  */
    u32bit numSetupUnits;              /**<  Number of setup unit.  */
    u32bit setupLatency;               /**<  Latency in cycles of a setup unit.  */
    u32bit setupStartLatency;          /**<  Start latency in cycles of a setup unit (inverse of througput).  */
    u32bit paLatency;                  /**<  Latency of the triangle bus from primite assembly/clipper.  */
    u32bit tbLatency;                  /**<  Latency of the triangle bus from Triangle Bound unit.  */
    u32bit fgLatency;                  /**<  Latency of the triangle bus to fragment generation.  */
    bool   shaderSetup;                /**<  Perform triangle setup in the unified shader.  */
    bool   preTriangleBound;           /**<  A previous Triangle Bound Unit early computes incoming triangles BBs.  */
    u32bit triangleShaderQSz;          /**<  Size of the reorder queue for the triangles being shaded.  */

    /*  Triangle Setup signals.  */
    Signal *setupCommand;       /**<  Command signal from the Rasterizer main box.  */
    Signal *rastSetupState;     /**<  State signal to the Rasterizer main box.  */
    Signal *inputTriangle;      /**<  New triangle signal from Primitive Assembly.  */
    Signal *triangleRequest;    /**<  Request signal to the Clipper/Triangle Bound unit.  */
    Signal *setupOutput;        /**<  Setup output signal to the Triangle Traversal/Fragment Generator.  */
    Signal *setupRequest;       /**<  Request signal from the Triangle Traversal/Fragment Generator.  */
    Signal *setupStart;         /**<  Setup Triangle start signal.  Simulates the cost setting up a triangle.  */
    Signal *setupEnd;           /**<  Setup Triangle end signal.  End of the setup of a triangle.  */
    Signal *setupShaderInput;   /**<  Triangle input signal to the unified shaders (FragmentFIFO).  */
    Signal *setupShaderOutput;  /**<  Triangle output signal from the unified shaders (FragmentFIFO).  */
    Signal *setupShaderState;   /**<  Unified shader (FragmentFIFO) state signal.  */

    /*  Triangle Setup registers.  */
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    bool d3d9PixelCoordinates;  /**<  Use D3D9 pixel coordinates convention -> top left is pixel (0, 0).  */
    u32bit iniX;                /**<  Viewport initial x coordinate.  */
    u32bit iniY;                /**<  Viewport initial y coordinate.  */
    u32bit width;               /**<  Viewport width.  */
    u32bit height;              /**<  Viewport height.  */
    bool scissorTest;           /**<  Enables scissor test.  */
    s32bit scissorIniX;         /**<  Scissor initial x coordinate.  */
    s32bit scissorIniY;         /**<  Scissor initial y coordinate.  */
    u32bit scissorWidth;        /**<  Scissor width.  */
    u32bit scissorHeight;       /**<  Scissor height.  */
    f32bit near;                /**<  Near depth range.  */
    f32bit far;                 /**<  Far depth range.  */
    bool d3d9DepthRange;        /**<  Use D3D9 range for depth in clip space [0, 1].  */
    f32bit slopeFactor;         /**<  Depth slope factor.  */
    f32bit unitOffset;          /**<  Depth unit offset.  */
    u32bit zBitPrecission;      /**<  Z buffer bit precission.  */
    FaceMode faceMode;          /**<  Front face selection mode.  */
    bool d3d9RasterizationRules;/**<  Use D3D9 rasterization rules.  */
    bool twoSidedLighting;      /**<  Enables two sided lighting color selection.  */
    CullingMode culling;        /**<  The current face culling mode.  */

    f64bit minTriangleArea;     /**<  The minimum area that a triangle must have to be rasterized (pixel area?).  */

    /*  Triangle Setup state.  */
    RasterizerState state;              /**<  Current triangle setup working state.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit setupWait;                   /**<  Number of cycles remaining until the next triangle can start setup.  */
    u32bit triangleCounter;             /**<  Number of processed triangles.  */
    TriangleSetupOutput **setupFIFO;    /**<  Setup triangles FIFO.  Used to keep setup triangles before triangle traversal.  */
    u32bit setupTriangles;              /**<  Number of triangle stored in the setup FIFO.  */
    u32bit setupReserves;               /**<  Number of setup FIFO reserved entries.  */
    u32bit firstSetupTriangle;          /**<  First setup triangle stored in the setup FIFO.  */
    u32bit nextFreeSTFIFO;              /**<  Next free entry in the setup FIFO.  */
    u32bit trianglesRequested;          /**<  Number of setup triangle that have been requested by Triangle Traversal.  */
    bool lastTriangle;                  /**<  Last triangle received from the Clipper/Triangle Bound unit.  */

    /*  Triangle queue for setup in the unified shaders.  */

    /**
     *
     *  Defines an entry for the reorder queue for the triangles sent to the shaders.
     *
     */

    struct TriangleShaderInput
    {
        TriangleSetupInput *tsInput;    /**<  Triangle setup input waiting to be shaded.  */
        bool shaded;                    /**<  Flag storing the the triangle setup result has been received.  */
        QuadFloat A;                    /**<  Stores the triangle A equation coefficient attribute.  */
        QuadFloat B;                    /**<  Stores the triangle B equation coefficient attribute.  */
        QuadFloat C;                    /**<  Stores the triangle C equation coefficient attribute.  */
        f32bit  tArea;                  /**<  Stores the triangle estimated 'area' attribute.  */
    };

    TriangleShaderInput *triangleShaderQueue;   /**<  Stores triangles waiting to be processed in the shader.  */
    u32bit nextSendShTriangle;                  /**<  Next triangle to be sent to the shaders.  */
    u32bit nextShaderTriangle;                  /**<  Next shaded triangle to commit.  */
    u32bit nextFreeShaderTriangle;              /**<  Next free entry in the triangle shader queue.  */
    u32bit sendShTriangles;                     /**<  Triangles waiting to be sent to the shaders.  */
    u32bit shaderTriangles;                     /**<  Triangles waiting for shader result.  */
    u32bit freeShTriangles;                     /**<  Free entries in the triangle shader queue.  */
    u32bit shTriangleReserves;                  /**<  Number of reserved entries in the triangle shader queue.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;   /**<  Number of input triangles.  */
    GPUStatistics::Statistic *outputs;  /**<  Number of outputs triangles.  */
    GPUStatistics::Statistic *requests; /**<  Triangle requests from Fragment Generation.  */
    GPUStatistics::Statistic *culled;   /**<  Number of culled triangles.  */
    GPUStatistics::Statistic *front;    /**<  Number of front facing triangles.  */
    GPUStatistics::Statistic *back;     /**<  Number of back facing triangles.  */

    /*   Triangle size related statistics.  */
    GPUStatistics::Statistic *count_less_1;         /**<  Non-culled triangles of less than 1 pixel size.  */ 
    GPUStatistics::Statistic *count_1_to_4;         /**<  Non-culled triangles between 1 and less than 4 pixels size.  */
    GPUStatistics::Statistic *count_4_to_16;        /**<  Non-culled triangles between 4 and less than 16 pixels size.  */
    GPUStatistics::Statistic *count_16_to_100;      /**<  Non-culled triangles between 16 and less than 100 pixels size.  */
    GPUStatistics::Statistic *count_100_to_1000;    /**<  Non-culled triangles between 100 and less than 1000  pixels size.  */
    GPUStatistics::Statistic *count_1000_to_screen; /**<  Non-culled triangles between 1000 and less than screen pixels size.  */
    GPUStatistics::Statistic *count_greater_screen; /**<  Non-culled triangles greater than screen pixels size.  */
    GPUStatistics::Statistic *count_overlap_1x1;    /**<  Non-culled triangles overlapping just one screen pixel.  */
    GPUStatistics::Statistic *count_overlap_2x2;    /**<  Non-culled triangles overlapping just 2x2 screen pixels.  */ 

    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *
     */

    void processCommand(RasterizerCommand *command);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Triangle Setup register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

    /**
     *
     *  Performs the cull face test for a triangle.
     *
     *  @param triangleID Identifier of the the triangle (index?).
     *  @param setupID Identifier of the setup triangle.
     *  @param tArea Reference to a 64 bit float point variable where
     *  to store the calculated are approximation of the triangle.
     *
     *  @return If the triangle must be dropped after the cull face test.
     *  TRUE means that it must be dropped, FALSE that it must not be dropped.
     *
     */

    bool cullFaceTest(u32bit triangleID, u32bit setupID, f64bit &tArea);

    /**
     *
     *  Sends triangles to triangle traversal/fragment generation.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendTriangles(u64bit cycle);

    /**
     *
     *  Processes a setup triangle.
     *
     *  Performes face and area culling.  Inverts triangle edge and z equations as
     *  required.  Selects color attribute for two faced lighting.
     *
     *  @param tsInput Pointer to the triangle that has been setup.
     *  @param triangleID Identifier of the setup triangle in the Rasterizer Emulator.
     *
     */

    void processSetupTriangle(TriangleSetupInput *tsInput, u32bit triangleID);

public:

    /**
     *
     *  Triangle Setup constructor.
     *
     *  Creates and initializes a new Triangle Setup box.
     *
     *  @param rastEmu Reference to the rasterizer emulator to be used by the Triangle Setup box.
     *  @param trianglesCycle Number of triangles recieved or sent to Fragment Generation per
     *  cycle.
     *  @param setupFIFOSize Size of the setup FIFO.
     *  @param setupUnits Number of setup units in the box.  Each setup unit can start the setup
     *  of a triangle in paralel.
     *  @param latency Latency of a setup unit.
     *  @param startLatency Start latency of a setup unit.
     *  @param paLat Latency of the bus from Primitive Assembly/Clipper.
     *  @param tbLat Latency of the bus from Triangle Bound unit.
     *  @param fgLat Latency of the bus to Primitive Assembly.
     *  @param shaderSetup Perform triangle setup in the unified shaders.
     *  @pamra preTriangleBound A previous Triangle Bound Unit early computes incoming triangles BBs.
     *  front of Triangle Setup.
     *  @param microTriBypass Bypass of microtriangles is enabled (MicroPolygon rasterizer only).
     *  @param triShQSz Size of the triangle shader result reorder queue.
     *  @param name Box name.
     *  @param parent Box parent box.
     *
     *  @return A new initialized Triangle Setup box.
     *
     */

    TriangleSetup(RasterizerEmulator &rastEmu, u32bit trianglesCycle, u32bit setupFIFOSize,
        u32bit setupUnits, u32bit latency, u32bit startLatency, u32bit paLat, u32bit tbLat, 
        u32bit fgLat, bool shaderSetup, bool preTriangleBound, bool microTriBypass, 
        u32bit triShQSz, char *name, Box *parent);

    /**
     *
     *  Triangle Setup simulation function.
     *
     *  Simulates one cycle of the Triangle Setup GPU unit.
     *
     *  @param cycle The cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Triangle Setup box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(std::string &stateString);

};

} // namespace gpu3d

#endif
