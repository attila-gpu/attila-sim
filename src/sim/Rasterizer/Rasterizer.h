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
 * $RCSfile: Rasterizer.h,v $
 * $Revision: 1.23 $
 * $Author: vmoya $
 * $Date: 2007-01-21 18:57:45 $
 *
 * Rasterizer box class definition file (unified shaders version).
 *
 */

/**
 *
 *  @file Rasterizer.h
 *
 *  This file defines the Rasterizer box (unified shaders version).
 *
 *  The Rasterizer box controls all the rasterizer boxes:
 *  Triangle Setup, Triangle Traversal, Interpolator and
 *  Fragment FIFO.
 *
 */


#ifndef _RASTERIZER_

#define _RASTERIZER_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "RasterizerCommand.h"
#include "GPU.h"
#include "RasterizerState.h"
#include "TriangleSetup.h"
#include "TriangleTraversal.h"
#include "HierarchicalZ.h"
#include "Interpolator.h"
#include "FragmentFIFO.h"

namespace gpu3d
{

/**
 *
 *  Rasterizer box class definition.
 *
 *  This class implements a Rasterizer box that renders
 *  triangles from the vertex calculated in the vertex
 *  shader.
 *
 */

class Rasterizer: public Box
{
private:

    /*  Rasterizer signals.  */
    Signal *rastCommSignal;     /**<  Command signal from the Command Processor.  */
    Signal *rastStateSignal;    /**<  Rasterizer state signal to the Command Processor.  */
    Signal *triangleSetupComm;  /**<  Command signal to Triangle Setup.  */
    Signal *triangleSetupState; /**<  State signal from Triangle Setup.  */
    Signal *triangleTraversalComm;  /**<  Command signal to Triangle Traversal.  */
    Signal *triangleTraversalState; /**<  State signal from Triangle Traversal.  */
    Signal *hzCommand;          /**<  Command signal to Hierarchical Z.  */
    Signal *hzState;            /**<  State signal from Hierarchical Z.  */
    Signal *interpolatorComm;   /**<  Command signal to Interpolator box.  */
    Signal *interpolatorState;  /**<  State signal from Interpolator box.  */
    Signal *fFIFOCommand;       /**<  Command signal to the Fragment FIFO box.  */
    Signal *fFIFOState;         /**<  State signal from the Fragment FIFO box.  */

    /*  Rasterizer boxes.  */
    TriangleSetup *triangleSetup;   /**<  Triangle Setup box.  */
    TriangleTraversal *triangleTraversal;   /**<  Triangle Traversal box.  */
    HierarchicalZ *hierarchicalZ;   /**<  Hierarchical Z box.  */
    Interpolator *interpolator;     /**<  Interpolator box.  */
    FragmentFIFO *fFIFO;            /**<  Fragment FIFO box.  */

    /*  Rasterizer registers.  */
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    bool d3d9PixelCoordinates;  /**<  Use D3D9 pixel coordinates convention -> top left is pixel (0, 0).  */
    s32bit viewportIniX;            /**<  Viewport initial (lower left) X coordinate.  */
    s32bit viewportIniY;            /**<  Viewport initial (lower left) Y coordinate.  */
    u32bit viewportHeight;          /**<  Viewport height.  */
    u32bit viewportWidth;           /**<  Viewport width.  */
    bool scissorTest;               /**<  Enables scissor test.  */
    s32bit scissorIniX;             /**<  Scissor initial (lower left) X coordinate.  */
    s32bit scissorIniY;             /**<  Scissor initial (lower left) Y coordinate.  */
    u32bit scissorHeight;           /**<  Scissor height.  */
    u32bit scissorWidth;            /**<  Scissor width.  */
    f32bit nearRange;               /**<  Near depth range.  */
    f32bit farRange;                /**<  Far depth range.  */
    bool d3d9DepthRange;            /**<  Use D3D9 range for depth in clip space [0, 1].  */
    f32bit slopeFactor;             /**<  Depth slope factor.  */
    f32bit unitOffset;              /**<  Depth unit offset.  */
    u32bit clearDepth;              /**<  Clear Z value.  */
    u32bit zBufferBitPrecission;    /**<  Z Buffer bit precission.  */
    bool frustumClipping;           /**<  Frustum clipping flag.  */
    QuadFloat userClip[MAX_USER_CLIP_PLANES];       /**<  User clip planes.  */
    bool userClipPlanes;            /**<  User clip planes enabled or disabled.  */
    FaceMode faceMode;              /**<  Front face selection mode.  */
    CullingMode cullMode;           /**<  Culling Mode.  */
    bool hzEnable;                  /**<  HZ test enable flag.  */
    bool earlyZ;                    /**<  Early Z test flag (Z/Stencil before shading).  */
    bool d3d9RasterizationRules;    /**<  Use D3D9 rasterization rules.  */
    bool twoSidedLighting;          /**<  Enables two sided lighting color selection.  */
    bool multisampling;             /**<  Flag that stores if MSAA is enabled.  */
    u32bit msaaSamples;             /**<  Number of MSAA Z samples generated per fragment when MSAA is enabled.  */
    bool interpolation[MAX_FRAGMENT_ATTRIBUTES];        /**<  Interpolation enabled or disabled (FLAT/SMOTH).  */
    bool fragmentAttributes[MAX_FRAGMENT_ATTRIBUTES];   /**<  Fragment input attributes active flags.  */
    bool modifyDepth;               /**<  Flag storing if the fragment shader modifies the fragment depth component.  */
    bool stencilTest;               /**<  Enable/Disable Stencil test flag.  */
    bool depthTest;                 /**<  Enable/Disable depth test flag.  */
    CompareMode depthFunction;      /**<  Depth test comparation function.  */
    bool rtEnable[MAX_RENDER_TARGETS];      /**<  Render target enable.  */
    bool colorMaskR[MAX_RENDER_TARGETS];    /**<  Update to color buffer Red component enabled.  */
    bool colorMaskG[MAX_RENDER_TARGETS];    /**<  Update to color buffer Green component enabled.  */
    bool colorMaskB[MAX_RENDER_TARGETS];    /**<  Update to color buffer Blue component enabled.  */
    bool colorMaskA[MAX_RENDER_TARGETS];    /**<  Update to color buffer Alpha component enabled.  */

    /*  Rasterizer parameters.  */
    RasterizerEmulator &rastEmu;    /**<  Reference to the Rasterizer Emulator that is going to be used.  */
    u32bit trianglesCycle;          /**<  Number of triangles received from Clipper/PA per cycle.  */
    u32bit triangleSetupFIFOSize;   /**<  Size of the Triangle Setup FIFO.  */
    u32bit triangleSetupUnits;      /**<  Number of triangle setup units.  */
    u32bit triangleSetupLatency;    /**<  Latency of the triangle setup units.  */
    u32bit triangleSetupStartLat;   /**<  Start latency of the triangle setup units.  */
    u32bit triangleInputLat;        /**<  Latency of the triangle bus from PA/Clipper.  */
    u32bit triangleOutputLat;       /**<  Latency of the triangle bus between Triangle Setup and Triangle Traversal/Fragment Generation.  */
    bool shadedSetup;               /**<  Triangle setup performed in the unified shader.  */
    u32bit triangleShQSz;           /**<  Size of the triangle shade queue in Triangle Setup.  */
    u32bit stampsCycle;             /**<  Number of stamps generated/processed per cycle.  */
    u32bit samplesCycle;            /**<  Number of depth samples that can be generated per fragment each cycle at Triangle Traversal/Fragment Generation when MSAA is enabled.  */
    u32bit overW;                   /**<  Over scan tile width in scan tiles.  */
    u32bit overH;                   /**<  Over scan tile height in scan tiles.  */
    u32bit scanW;                   /**<  Scan tile width in pixels.  */
    u32bit scanH;                   /**<  Scan tile height in pixels.  */
    u32bit genW;                    /**<  Generation tile width in pixels.  */
    u32bit genH;                    /**<  Generation tile height in pixels.  */
    u32bit trBatchSize;             /**<  Size of a rasterization triangle batch (triangle traversal).  */
    u32bit trBatchQueueSize;        /**<  Triangles in the batch queue.  */
    bool recursiveMode;             /**<  Determines the rasterization method (T: recursive, F: scanline).  */
    bool disableHZ;                 /**<  Disables Hierarchical Z.  */
    u32bit blockStamps;             /**<  Stamps per HZ block.  */
    u32bit hzBufferSize;            /**<  Size of the Hierarchical Z Buffer (number of blocks stored).  */
    u32bit hzCacheLines;            /**<  Number of lines int the fast access Hierarchical Z Buffer cache.  */
    u32bit hzCacheLineSize;         /**<  Blocks per line of the fast access Hierarchical Z Buffer cache.  */
    u32bit hzQueueSize;             /**<  Size of the stamp queue for the Early test.  */
    u32bit hzBufferLatency;         /**<  Access latency to the HZ Buffer Level 0.  */
    u32bit hzUpdateLatency;         /**<  Update signal latency from Z Stencil box.  */
    u32bit clearBlocksCycle;        /**<  Number of blocks that can be cleared per cycle.  */
    u32bit interpolators;           /**<  Number of hardware interpolators.  */
    u32bit shInputQSz;              /**<  Shader input queue size (per shader unit).  */
    u32bit shOutputQSz;             /**<  Shader output queue size (per shader unit).  */
    u32bit shInputBatchSz;          /**<  Number of consecutive shader inputs assigned per shader unit.  */
    bool tiledShDistro;             /**<  Enable/disable distribution of fragment inputs to the shader units on a tile basis.   */
    bool unifiedModel;              /**<  Simulate an unified shader model architecture.  */
    u32bit numVShaders;             /**<  Number of fragment shaders attached to the Rasterizer.  */
    u32bit numFShaders;             /**<  Number of fragment shaders attached to the Rasterizer.  */
    u32bit shInputsCycle;           /**<  Shader inputs that can be sent to a shader per cycle.  */
    u32bit shOutputsCycle;          /**<  Shader outputs that can be received from a shader per cycle.  */
    u32bit threadGroup;             /**<  Number of inputs that form a shader thread group.  */
    u32bit maxFShOutLat;            /**<  Maximum latency of the shader output signal to the FFragment FIFO.  */
    u32bit vInputQueueSz;           /**<  Size of the vertex input queue.  */
    u32bit vShadedQueueSz;          /**<  Size of the vertex output queue.  */
    u32bit trInQueueSz;             /**<  Triangle input queue size (for triangle setup on shaders). */
    u32bit trOutQueueSz;            /**<  Triangle output queue size (for triangle setup on shaders).  */
    u32bit rastQueueSz;             /**<  Size of the generated stamp queue in the Fragment FIFO (per stamp unit).  */
    u32bit testQueueSz;             /**<  Size of the early z tested stamp queue in the Fragment FIFO.  */
    u32bit intQueueSz;              /**<  Size of the interpolated stamp queue in the Fragment FIFO.  */
    u32bit shQueueSz;               /**<  Size of the shaded stamp queue in the Fragment FIFO (per stamp unit).  */
    u32bit numStampUnits;           /**<  Number of stamp units attached to the Rasterizer.  */

    /*  Rasterizer state.  */
    RasterizerState state;      /**<  Current state of the Rasterizer unit.  */
    RasterizerCommand *lastRastComm;    /**<  Last rasterizer command received.  */

    /*  Private functions.  */

    /**
     *
     *  Processes new rasterizer commands received from the
     *  Command Processor.
     *
     *  @param rastComm The RasterizerCommand to process.
     *  @param cycle The current simulation cycle.
     *
     */

    void processRasterizerCommand(RasterizerCommand *rastComm, u64bit cycle);

    /**
     *
     *  Processes a rasterizer register write.
     *
     *  @param reg The rasterizer register to write to.
     *  @param subreg The Rasterizer register subregister to write to.
     *  @param data The data to write to the register.
     *  @param cycle The current simulation cycle.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data,
        u64bit cycle);

public:

    /**
     *
     *  Rasterizer constructor.
     *
     *  This function creates and initializates the Rasterizer box state and
     *  structures.
     *
     *  @param rastEmu Reference to the Rasterizer Emulator that will be
     *  used for emulation.
     *  @param trianglesCycle Triangles received from PA/Clipper per cycle.
     *  @param tsFIFOSize Size of the Triangle Setup FIFO.
     *  @param trSetupUnits Number of triangle setup units.
     *  @param trSetupLat Latency of the triangle setup units.
     *  @param trSetupStartLat Start latency of the triangle setup units.
     *  @param trInputLat Latency of the triangle bus from PA/Clipper.
     *  @param trOutputLat Latency of the triangle bus between Triangle Setup and Triangle Traversal/Fragment Generation.
     *  @param shSetup Triangle setup to be performed in the unified shaders.
     *  @param trShQSz Size of the triangle shader queue in Triangle Setup.
     *  @param stampsCycle Stamps generated/processed per cycle.
     *  @param samplesCycle Number of depth samples that can generated per fragment each cycle in Triangle Traversal/Fragment Generation when MSAA is enabled.
     *  @param overW Over scan tile width in scan tiles (may become a register!).
     *  @param overH Over scan tile height in scan tiles (may become a register!).
     *  @param scanW Scan tile width in fragments.
     *  @param scanH Scan tile height in fragments.
     *  @param genW Generation tile width in fragments.
     *  @param genH Generation tile height in fragments.
     *  @param trBatchSize Size of the rasterizer triangle batch.
     *  @param trBatchQueueSize Size of the batch queue in triangles.
     *  @param recursive Flag that determines the rasterization method (T: recursive, F: scanline).
     *  @param disableHZ Disables Hierarchical Z.
     *  @param blockStamps Stamps per HZ block.
     *  @param hzBufferSize Size of the Hierarchical Z Buffer (number of blocks stored).
     *  @param hzCacheLines Lines in the fast access Hierarchical Z Buffer cache.
     *  @param hzCacheLineSize Blocks per line of the fast access Hierarchical Z Buffer cache.
     *  @param hzQueueSize Size of the stamp queue for the Early test.
     *  @param hzBufferLatency Access latency to the HZ Buffer Level 0.
     *  @param hzUpdateLantecy Update signal latency from Z Stencil Test box.
     *  @param clearBlocksCycle Number of blocks that can be cleared per cycle.
     *  @param interp  Number of hardware interpolators available.
     *  @param shInQSz Shader Input Queue size (per shader unit).
     *  @param shOutQSz Shader Output Queue size (per shader unit).
     *  @param shInBSz Shader inputs (consecutive) assigned per shader unit.
     *  @param shTileDistro Enable/disable distribution of fragment inputs to the shader units on a tile basis.
     *  @param unified Simulate an unified shader model architecture.
     *  @param nVShaders Number of virtual vertex shaders simulated by the rasterizer (unified model only).
     *  @param vshPrefix Array of virtual vertex shader prefixes (unified model only).
     *  @param nFShaders Number of fragment shaders attached to the rasterizer.
     *  @param fshPrefix Array of fragment shader prefixes.
     *  @param shInCycle Shader inputs per cycle and shader.
     *  @param shOutCycle Shader outputs per cycle and shader.
     *  @param thGroup Threads per shader group.
     *  @param fshOutLat Latency of the shader output signal.
     *  @param vInQSz Vertex input queue size.
     *  @param vOutQSz Vertex output queue size.
     *  @param trInQSz Triangle input queue size.
     *  @param trOutQSz Triangle output queue size.
     *  @param rastQSz Rasterized stamp queue size (Fragment FIFO) (per stamp unit).
     *  @param testQSz Early Z tested stamp queue size (Fragment FIFO).
     *  @param intQSz Interpolated stamp queue size (Fragment FIFO).
     *  @param shQSz Shaded stamp queue size (Fragment FIFO) (per stamp unit).
     *  @param nStampUnits Number of stamp units attached to the rasterizer.
     *  @param suPrefixes Array of stamp unit prefixes.
     *  @param name  Name of the box.
     *  @param parent  Pointer to a parent box.
     *
     */

    Rasterizer(RasterizerEmulator &rastEmu, u32bit trianglesCycle, u32bit tsFIFOSize,
        u32bit trSetupUnits, u32bit trSetupLat, u32bit trSetupStartLat, u32bit trInputLat, u32bit trOutputLat,
        bool shSetup, u32bit trShQSz, u32bit stampsCycle, u32bit samplesCycle, u32bit overW, u32bit overH, u32bit scanW, u32bit scanH,
        u32bit genW, u32bit genH, u32bit trBatchSize, u32bit trBatchQueueSize, bool recursive,
        bool disableHZ, u32bit blockStamps, u32bit hzBufferSize,
        u32bit hzCacheLines, u32bit hzCacheLineSize, u32bit hzQueueSize, u32bit hzBufferLatency,
        u32bit hzUpdateLatency, u32bit clearBlocksCycle, u32bit interp, u32bit shInQSz, u32bit shOutQSz, u32bit shInBSz,
        bool shTileDistro, bool unified, u32bit nVShaders, char **vshPrefix, u32bit nFShaders, char **fshPrefix,
        u32bit shInCycle, u32bit shOutCycle, u32bit thGroup, u32bit fshOutLat,
        u32bit vInQSz, u32bit vOutQSz, u32bit trInQSz, u32bit trOutQSz, u32bit rastQSz, u32bit testQSz,
        u32bit intQSz, u32bit shQSz, u32bit nStampUnits, char **suPrefixes,
        char *name, Box *parent = 0);

    /**
     *
     *  Rasterizer destructor.
     *
     */
     
    ~Rasterizer();
    
    /**
     *
     *  Performs the simulation and emulation of the rasterizer
     *  cycle by cycle.
     *
     *  @param cycle  The cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Rasterizer box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);

    /**
     *
     *  Saves the content of the Hierarchical Z buffer into a file.
     *
     */
     
    void saveHZBuffer();
     
    /**
     *
     *  Loads the content of the Hierarchical Z buffer from a file.
     */
     
    void loadHZBuffer();

    /**
     *
     *  Detects stall conditions in the Fragment FIFO box.
     *
     *  @param cycle Current simulation cycle.
     *  @param active Reference to a boolean variable where to return if the stall detection logic is enabled/implemented.
     *  @param stalled Reference to a boolean variable where to return if the FragmentFIFO has been detected to be stalled.
     *
     */
     
    void detectStall(u64bit cycle, bool &active, bool &stalled);
    
    /**
     *
     *  Writes into a string a report about the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to store the stall state report for the box.
     *
     */
     
    void stallReport(u64bit cycle, string &stallReport);
    
};

} // namespace gpu3d

#endif
