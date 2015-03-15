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
 * $RCSfile: FragmentFIFO.h,v $
 * $Revision: 1.18 $
 * $Author: vmoya $
 * $Date: 2006-11-03 15:19:23 $
 *
 * Fragment FIFO (unified shaders version) class definition file.
 *
 */

/**
 *
 *  @file FragmentFIFO.h
 *
 *  This file defines the Fragment FIFO class.
 *
 *  This class implements a Fragment FIFO simulation box.  The Fragment FIFO stores
 *  and directs stamps of fragments from the rasterizer to the Interpolator and Shader
 *  units.  It also stores shaded fragments and sends them Z Stencil Test.
 *
 */


#ifndef _FRAGMENTFIFO_

#define _FRAGMENTFIFO_



#include "Box.h"
#include "GPUTypes.h"
#include "support.h"
#include "GPU.h"
#include "RasterizerState.h"
#include "RasterizerCommand.h"
#include "Interpolator.h"
#include "ZStencilTestV2.h"
#include "ColorWriteV2.h"
#include "ShaderFetch.h"
#include "ConsumerStateInfo.h"
#include "FragmentInput.h"
#include "FragmentFIFOState.h"
//#include <fstream>
#include "zfstream.h"

using namespace std;

namespace gpu3d
{

/**
 *
 *  Defines a number of cycles as threshold after a group of vertex inputs
 *  smaller than STAMP_FRAGMENTS is sent to the shaders.
 *
 */
static const u32bit VERTEX_CYCLE_THRESHOLD = 1000;

/**
 *
 *  This class implements the Fragment FIFO box (unified shaders version).
 *
 *  The Fragment FIFO box simulates a storage unit that redirects fragments produced in
 *  the rasterizer to the Interpolator and Shader units for shading, receives shaded
 *  fragments from the Shader and stores them until they can be processed in the
 *  Z and Stencil Test unit.
 *
 *  Inherits from the Box class that offers basic simulation support.
 *
 */

class FragmentFIFO: public Box
{

private:

#ifdef GPU_TEX_TRACE_ENABLE
    //ofstream fragTrace;
    gzofstream fragTrace;
#endif

    /*  Fragment FIFO Signals.  */
    Signal *hzInput;            /**<  Stamp signal from Hierarchical/Early Z box.  */
    Signal *ffStateHZ;          /**<  Fragment FIFO tate signal to the Hierarchical/Early Z box.  */
    Signal *interpolatorInput;  /**<  Input stamp signal to the Interpolator unit.  */
    Signal *interpolatorOutput; /**<  Output stamp signal from the Interpolator unit.  */
    Signal **shaderInput;       /**<  Array of stamp input signal to the Shader unit.  */
    Signal **shaderOutput;      /**<  Array of stamp output signal from the Shader unit.  */
    Signal **shaderState;       /**<  Array of state signal from the Shader unit.  */
    Signal **ffStateShader;     /**<  Array of Fragment FIFO state signals to the Shader unit.  */
    Signal **vertexInput;       /**<  Array of vertex input signals from Streamer Loader.  */
    Signal **vertexOutput;      /**<  Array of vertex output signals to Streamer Commit.  */
    Signal **vertexState;       /**<  Array of vertex state signals to Streamer Loader.  */
    Signal **vertexConsumer;    /**<  Array of vertex consumer state signals from Streamer Commit.  */
    Signal *triangleInput;      /**<  Triangle shader input signal from Triangle Setup.  */
    Signal *triangleOutput;     /**<  Triangle shader output signal to Triangle Setup.  */
    Signal *triangleState;      /**<  Triangle shader state signal to Triangle Setup.  */
    Signal **zStencilInput;     /**<  Array of input stamp signal to the Z Stencil Test unit.  */
    Signal **zStencilOutput;    /**<  Array of stamp signal from the Z Stencil Test unit.  */
    Signal **zStencilState;     /**<  Array of state signal from the Z Stencil Test unit.  */
    Signal **fFIFOZSTState;     /**<  Array of state signal to the Z Stencil Test unit.  */
    Signal **cWriteInput;       /**<  Array of input stamp signal to the Color Write unit.  */
    Signal **cWriteState;       /**<  Array of state signal from the Color Write unit.  */
    Signal *fFIFOCommand;       /**<  Command signal from the main rasterizer box.  */
    Signal *fFIFOState;         /**<  Command signal to the main rasterizer box.  */

    /*  Fragment FIFO parameters.  */
    u32bit hzStampsCycle;       /**<  Number of stamps received from Hierarchical Z per cycle.  */
    u32bit stampsCycle;         /**<  Number of stamps per cycle send down the pipeline.  */
    bool unifiedModel;          /**<  Simulate an unified shader architecture.  */
    bool shadedSetup;           /**<  Triangle setup performed in the shader.  */
    u32bit trianglesCycle;      /**<  Triangles received/sent from/to Triangle Setup per cycle.  */
    u32bit triangleLat;         /**<  Latency of the triangle input/output signals with Triangle Setup.  */
    u32bit numVShaders;         /**<  Number of virtual vertex shader visible to Stremer units.  */
    u32bit numFShaders;         /**<  Number of Fragment Shaders attached to Fragment FIFO.  */
    u32bit threadGroup;         /**<  Size of a thread group (packet of inputs to send to the shader).  */
    u32bit shInputsCycle;       /**<  Shader inputs per cycle that can be sent to a shader.  */
    u32bit shOutputsCycle;      /**<  Shader outputs per cycle that can be received from a shader.  */
    u32bit fshOutLatency;       /**<  Maximum latency of the shader stamp output signal.  */
    u32bit interpolators;       /**<  Number of attribute interpolator ALUs in the Interpolator unit.  */
    u32bit shInputQSize;        /**<  Size of the shader input queue.  */
    u32bit shOutputQSize;       /**<  Size of the shader output queue.  */
    u32bit shInputBatchSize;    /**<  Consecutive shader inputs assigned per shader unit work batch.  */
    bool tiledShDistro;         /**<  Enable/disable distribution of fragment inputs to the shader units on a tile basis.  */
    u32bit vInputQueueSz;       /**<  Vertex input queue size (in vertices).  */
    u32bit vShadedQueueSz;      /**<  Vertex output/shaded queue size (in vertices).  */
    u32bit triangleInQueueSz;   /**<  Size of the triangle input queue (in triangles).  */
    u32bit triangleOutQueueSz;  /**<  Size of the triangle output queue (in triangles).  */
    u32bit rastQueueSz;         /**<  Rasterizer (Generated fragments) queue size (per stamp unit).  */
    u32bit testQueueSz;         /**<  Early Z Test (fragments after Z Stencil) queue size.  */
    u32bit intQueueSz;          /**<  Interpolator (interpolated fragments) queue size.  */
    u32bit shQueueSz;           /**<  Shader (shaded fragments) queue size (per stamp unit).  */
    u32bit numStampUnits;       /**<  Number of stamp units attached to the Rasterizer.  */

    u32bit stampsPerUnit;       /**<  Precalculated number of stamps to be processed per cycle and stamp/quad unit (ZST or CW).  */
    u32bit stampsPerGroup;      /**<  Precalculated number of stamps per thread group.  */

    /*  Fragment FIFO structures.  */
    FragmentInput ****rastQueue;/**<  Queue for the stamps received from Fragment Generation per stamp unit.  */
    u32bit *nextRast;           /**<  Next stamp in the rasterizer queue to be sent to the Interpolator.  */
    u32bit *nextFreeRast;       /**<  Next free entry in the rasterizer queue.  */
    u32bit *rastStamps;         /**<  Stamps in the rasterizer stamp queue.  */
    u32bit *freeRast;           /**<  Number of free entries in the rasterizer stamp queue.  */
    u32bit allRastStamps;       /**<  Number of stamps in all the rasterizer stamp queues.  */
    u32bit nextRastUnit;        /**<  Pointer to the next queue from where to read rasterizer stamps.  */

    FragmentInput ****testQueue;/**<  Queue for the stamps after early Z/Stencil test per stamp unit.  */
    u32bit *nextTest;           /**<  Next stamp in the post early z test queue.  */
    u32bit *nextFreeTest;       /**<  Next free entry in the post early z test queue.  */
    u32bit *testStamps;         /**<  Stamps in the post early z stamp queue.  */
    u32bit *freeTest;           /**<  Number of free entries in the post early z test stamp queue.  */
    u32bit allTestStamps;       /**<  Number of stamps in all the after early Z stamp queues.  */
    u32bit nextTestUnit;        /**<  Pointer to the next queue from where to read tested stamps.  */

    FragmentInput ****intQueue; /**<  Queue for the stamps interpolated per stamp unit.  */
    u32bit *nextInt;            /**<  Next stamp in the interpolated queue to be sent to the shader.  */
    u32bit *nextFreeInt;        /**<  Next free entry in the interpolator queue.  */
    u32bit *intStamps;          /**<  Stamps in the interpolated stamp queue.  */
    u32bit *freeInt;            /**<  Number of free entries in the interpolated stamp queue.  */
    u32bit allIntStamps;        /**<  Number of interpolated stamps in all the queues.  */
    u32bit nextIntUnit;         /**<  Pointer to the next queue from which to read interpolated stamps.  */

    FragmentInput ****shQueue;  /**<  Queue for the stamps shaded per stamp unit.  */
    bool **shaded;              /**<  Stores if a stamp in the shaded queue has been really shaded per stamp unit (to keep order even with Shader out ordering the fragments).  */
    u32bit *nextShaded;         /**<  Next stamp in the shaded queue to be sent to Z Stencil Test per stamp unit.  */
    u32bit *nextFreeSh;         /**<  Next free enry in the shaded queue per stamp unit.  */
    u32bit *shadedStamps;       /**<  Stamps in the shaded stamp queue per stamp unit.  */
    u32bit *freeShaded;         /**<  Number of free entries in the shaded stamp queue per stamp unit.  */

    /*  Vertex queues.  */
    ShaderInput **inputVertexQueue; /**<  Queue storing vertex inputs (unshaded).  */
    u32bit nextInputVertex;         /**<  Pointer to the next vertex in the vertex input queue.  */
    u32bit nextFreeInput;           /**<  Pointer to the next free entry in the vertex input queue.  */
    u32bit vertexInputs;            /**<  Number of vertices in the input queue.  */
    u32bit freeInputs;              /**<  Number of free entries in the vertex input queue.  */

    ShaderInput **shadedVertexQueue;/**<  Queue storing vertex output (shaded).  */
    u32bit nextShadedVertex;        /**<  Pointer to the next vertex in the shaded vertex queue.  */
    u32bit nextFreeShVertex;        /**<  Pointer to the next free entry in the shaded vertext queue.  */
    u32bit shadedVertices;          /**<  Number of vertices in the shaded vertex queue.  */
    u32bit freeShVertices;          /**<  Number of free entries in the shaded vertex queue.  */

    /*  Triangle queue.  */
    ShaderInput **triangleInQueue;  /**<  Queue storing triangle shader inputs.  */
    u32bit nextInputTriangle;       /**<  Pointer to the next triangle to send to the shader.  */
    u32bit nextFreeInTriangle;      /**<  Pointer to the next free entry in the queue.  */
    u32bit inputTriangles;          /**<  Number of triangles waiting to be sent to the shader.  */
    u32bit freeInputTriangles;      /**<  Number of free triangle input queue entries.  */

    ShaderInput **triangleOutQueue; /**<  Queue storing triangle shader outputs.  */
    u32bit nextOutputTriangle;      /**<  Pointer to the next triangle to send back to Triangle Setup.  */
    u32bit nextFreeOutTriangle;     /**<  Pointer to the next free entry in the queue.  */
    u32bit outputTriangles;         /**<  Number of triangles waiting to be sent back to Triangle Setup.  */
    u32bit freeOutputTriangles;     /**<  Number of free triangle output queue entries.  */

    /*  Shader batch queues.  */
    ShaderInput ***shaderInputQ;    /**<  Queue storing shader inputs to be sent to the shader (per shader unit).  */
    u32bit *nextShaderInput;        /**<  Next input to send to a shader in the shader input queue.  */
    u32bit *nextFreeShInput;        /**<  Next free entry in the shader input queue.  */
    u32bit *numShaderInputs;        /**<  Number of inputs waiting to be sent in the shader input queue.  */
    u32bit *numFreeShInputs;        /**<  Number of free entries in the shader input queue.  */
    u32bit nextFreeShInQ;           /**<  Next queue where to store shader inputs.  */
    u32bit nextSendShInQ;           /**<  Next queue from which to send inputs to the shader.  */
    u64bit inputFragments;          /**<  Number of inputs fragments sent to the shaders.  */

    ShaderInput ***shaderOutputQ;   /**<  Queue storing shader outputs received from the shader (per shader unit).  */
    u32bit *nextShaderOutput;       /**<  Next output received from a shader to process in the shader output queue.  */
    u32bit *nextFreeShOutput;       /**<  Next free entry in the shader output queue.  */
    u32bit *numShaderOutputs;       /**<  Number of outputs waiting to be processed in the shader output queue.  */
    u32bit *numFreeShOutputs;       /**<  Number of free entries in the shader output queue.  */
    u32bit nextProcessShOutQ;       /**<  Next queue from which to process shader outputs.  */
    u64bit outputFragments;         /**<  Number of output fragments sent to the shaders.  */

    u32bit *batchedShInputs;        /**<  Number of consecutive shader inputs sent to a shader unit.  */

    //  Stall detection state.
    u64bit *lastCycleShInput;       /**<  Stores per shader unit the last cycle a shader input was sent to the shader unit.  */
    u64bit *lastCycleShOutput;      /**<  Stores per shader unit the last cycle a shader output was received from the shader unit.  */
    u32bit *shaderElements;         /**<  Stores per shader unit the number of elements being processed in the shader unit.  */
    u64bit *lastCycleZSTIn;         /**<  Stores per ROP unit the last cycle an input was sent to the ZStencilTest unit.  */
    u64bit *lastCycleCWIn;          /**<  Stores per ROP unit the last cycle an input was sent to the Color Write unit.  */
    
    /*  Fragment FIFO registers.  */
    bool earlyZ;                /**<  Flag that enables or disables early Z testing (Z Stencil before shading).  */
    bool fragmentAttributes[MAX_FRAGMENT_ATTRIBUTES];   /**<  Stores the fragment input attributes that are enabled and must be calculated.  */
    bool stencilTest;               /**<  Enable/Disable Stencil test flag.  */
    bool depthTest;                 /**<  Enable/Disable depth test flag.  */
    bool rtEnable[MAX_RENDER_TARGETS];      /**<  Enable/Disable render target.  */
    bool colorMaskR[MAX_RENDER_TARGETS];    /**<  Update to color buffer Red component enabled.  */
    bool colorMaskG[MAX_RENDER_TARGETS];    /**<  Update to color buffer Green component enabled.  */
    bool colorMaskB[MAX_RENDER_TARGETS];    /**<  Update to color buffer Blue component enabled.  */
    bool colorMaskA[MAX_RENDER_TARGETS];    /**<  Update to color buffer Alpha component enabled.  */


    /*  Fragment FIFO state.  */
    RasterizerState state;      /**<  Current Fragment FIFO box state.  */
    RasterizerCommand *lastRSCommand;   /**<  Last rasterizer command received.  */
    ConsumerState *consumerState;       /**<  Array for storing the state from Streamer Commit.  */
    ShaderState *shState;       /**<  Array for storing the state from the Fragment shaders.  */
    ROPState *zstState;         /**<  Array for storing the state from the Z Stencil Units.  */
    ROPState *cwState;          /**<  Array for storing the state from the Color Write Units.  */
    u32bit fragmentCounter;     /**<  Number of received fragments.  */
    bool lastFragment;          /**<  Stores if the last fragment has been received.  */
    u32bit interpCycles;        /**<  Cycles until the next stamp can be sent to the Interpolator (depends on the number of attributes to interpolate).  */
    u32bit waitIntCycles;       /**<  Cycles remaining until the interpolator is ready again to receive a stamp.  */
    bool *lastStampInterpolated;/**<  Flag storing if the last stamp passed through the interpolator.  */
    bool *lastShaded;           /**<  Stores if the last stamp has been queued in the stamp unit shaded queues.  */
    u32bit notLastSent;         /**<  Number of Z Stencil Test/Color Writes units waiting for last fragment.  */
    bool lastVertexInput;       /**<  Flag storing if the last vertex of a batch was issued by any Streamer Loader Unit.  */
    bool lastVertexCommit;      /**<  Flag storing if the last vertex of a batch was send to PA by the Streamer Commit unit.  */
    bool lastTriangle;          /**<  Last triangle input in the batch received.  */
    bool firstVertex;           /**<  Stores if the first vertex of the current batch has been received.  */
    u64bit lastVertexGroupCycle;/**<  Stores the cycle that the last vertex group was issued the shader units (used to prevent deadlocks).  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;       /**<  Input fragments from HZ.  */
    GPUStatistics::Statistic *interpolated; /**<  Interpolated fragments.  */
    GPUStatistics::Statistic *shadedFrag;   /**<  Shaded fragments.  */
    GPUStatistics::Statistic *outputs;      /**<  Output fragments to Z Stencil.  */
    GPUStatistics::Statistic *inVerts;      /**<  Input vertices.  */
    GPUStatistics::Statistic *shVerts;      /**<  Shaded vertices.  */
    GPUStatistics::Statistic *outVerts;     /**<  Output vertices.  */
    GPUStatistics::Statistic *inTriangles;  /**<  Input triangles.  */
    GPUStatistics::Statistic *outTriangles; /**<  Output triangles.  */
    GPUStatistics::Statistic *shTriangles;  /**<  Shaded triangles.  */
    GPUStatistics::Statistic *rastGLevel;   /**<  Occupation of the rasterized stamp queues.  */
    GPUStatistics::Statistic *testGLevel;   /**<  Occupation of the early z tested stamp queues.  */
    GPUStatistics::Statistic *intGLevel;    /**<  Occupation of the interpolated stamp queues.  */
    GPUStatistics::Statistic *shadedGLevel; /**<  Occupation of the shader stamp queues.  */
    GPUStatistics::Statistic *vertInLevel;  /**<  Occupation of the vertex input queue.  */
    GPUStatistics::Statistic *vertOutLevel; /**<  Occupation of the vertex output queue.  */
    GPUStatistics::Statistic *trInLevel;    /**<  Occupation of the triangle input queue.  */
    GPUStatistics::Statistic *trOutLevel;   /**<  Occupation of the triangle output queue.  */

    /*  Private functions.  */

    /**
     *
     *  Receives new stamps from Fragment Generation.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void receiveStamps(u64bit cycle);

    /**
     *
     *  Receives triangles from Triangle Setup.
     *
     *  @param cycle The current simulation cycle.
     *
     */
    void receiveTriangles(u64bit cycle);

    /**
     *
     *  Receives vertices from Streamer Loader.
     *
     *  @param cycle The current simulation cycle.
     *
     */
    void receiveVertices(u64bit cycle);

    /**
     *
     *  Sends shaded stamps to the Z Stencil unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendStampsZStencil(u64bit cycle);

    /**
     *
     *  Sends shaded stamps to the Color Write unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendStampsColorWrite(u64bit cycle);

    /**
     *
     *  Sends z tested stamps to the Color Write unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendTestedStampsColorWrite(u64bit cycle);

    /**
     *
     *  Sends shaded triangles to Triangle Setup.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendTriangles(u64bit cycle);

    /**
     *
     *  Sends shaded vertices to Streamer Commit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendVertices(u64bit cycle);

    /**
     *
     *  Shades stamps.  Sends interpolated stamps to the Fragment Shader units.
     *
     *  @param cycle Current simulation cycle.
     */

    void shadeStamps(u64bit cycle);

    /**
     *
     *  Shades triangles.  Sends input triangles to the Shader units.
     *
     *  @param cycle Current simulation cycle.
     */

    void shadeTriangles(u64bit cycle);

    /**
     *
     *  Shades vertices.  Sends input vertices to the Shader units.
     *
     *  @param cycle Current simulation cycle.
     */

    void shadeVertices(u64bit cycle);

    /**
     *
     *  Receives shaded vertices or stamps from the Fragment Shader units.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void receiveShaderOutput(u64bit cycle);

    /**
     *
     *  Distributes the shader outputs to the propper shaded (vertex, triangle or fragment) queues.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void distributeShaderOutput(u64bit cycle);

    /**
     *
     *  Sends shader inputs to the shader units.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendShaderInputs(u64bit cycle);

    /**
     *
     *  Interpolates stamps.  Sends stamps to the Interpolator unit from
     *  rasterized stamp queue.
     *
     *  @param cycle Current simulation cycle.
     */

    void startInterpolation(u64bit cycle);

    /**
     *
     *  Interpolates stamps.  Sends stamps to the Interpolator unit from
     *  early Z tested stamp queue.
     *
     *  @param cycle Current simulation cycle.
     */

    void startInterpolationEarlyZ(u64bit cycle);

    /**
     *
     *  Interpolates stamps.   Receives interpolated stamps from the Interpolator unit.
     *
     *  @param cycle Current simulation cycle.
     */

    void endInterpolation(u64bit cycle);

    /**
     *
     *  Send stamps to Z Stencil Test (early Z) from rasterized queue.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void startEarlyZ(u64bit cycle);

    /**
     *
     *  Receive stamps from early z test and store them in the early z tested stamp queue.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void endEarlyZ(u64bit cycle);

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *  @param cycle  Current simulation cycle.
     *
     */

    void processCommand(RasterizerCommand *command, u64bit cycle);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Triangle Traversal register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

public:

    /**
     *
     *  Fragment FIFO class constructor.
     *
    *  Creates and initializes a Fragment FIFO box.
     *
     *  @param hzStampsCycle Stamps received from Hierarchical Z per cycle.
     *  @param stampsCycle Stamps issued down the pipeline per cycle.
     *  @param unified Simulate an unified shader model.
     *  @param shadedSetup Triangle setup on the shader.
     *  @param trCycle Triangles to be shaded received/sent per cycle from Triangle Setup.
     *  @param trLat Triangle signal from/to Triangle Setup latency.
     *  @param numVShaders Number of Virtual Vertex Shaders supported in the Fragment FIFO (only for unified model).
     *  @param numFShaders Number of Fragment Shaders attached to Fragment FIFO.
     *  @param shInCycle Shader inputs that can be sent per cycle to a shader.
     *  @param shOutCycle Shader outputs that can be received per cycle from a shader.
     *  @param thGroup Size of a shader thread group (inputs grouped to be sent to the shader).
     *  @param fshOutLat Fragment Shader output signal maximum latency.
     *  @param interp Number of attribute interpolators in the Interpolator unit.
     *  @param shInQSz Shader Input queue size (per shader unit).
     *  @param shOutQSz Shader Output queue size (per shader unit).
     *  @param shInBSz Consecutive shader inputs assigned per shader unit.
     *  @param shTileDistro Enable/disable distribution of fragment inputs to the shader units on a tile basis.
     *  @param vInQSz Vertex input queue size (only for unified model).
     *  @param vOutQSz Vertex output queue size (only for unified model).
     *  @param trInQSz Triangle input queue size (for setup on shader).
     *  @param trOutQSz Triangle output queue size (for setup on shader).
     *  @param rastQSz Size of the rasterizer stamp queue (per stamp unit).
     *  @param testQSz Size of the early z stamp queue.
     *  @param intQSz Size of the interpolator stamp queue.
     *  @param shaderQSz Size of the shader stamp queue.
     *  @param vshPrefixes Array of shader prefixes for the virtual vertex shaders (only for unified model).
     *  @param fshPrefixes Array of shader prefixes for the Fragment Shaders attached to Fragment FIFO.
     *  @param nStampUnits Number of stamp units attached to Fragment FIFO.
     *  @param suPrefixes Array of stamp unit prefixes.
     *  @param name Name of the box.
     *  @param parent Pointer to the box parent box.
     *
     *  @return A new initialized Fragment FIFO box.
     *
     */

    FragmentFIFO(u32bit hzStampsCycle, u32bit stampsCycle, bool unified, bool shadedSetup,
        u32bit trCycle, u32bit trLat, u32bit numVShaders, u32bit numFShaders, u32bit shInCycle, u32bit shOutCycle,
        u32bit thGroup, u32bit fshOutLat, u32bit interp, u32bit shInQSz, u32bit shOutQSz, u32bit shInBSz,
        bool shTileDistro, u32bit vInQSz, u32bit vOutQSz, u32bit trInQSz, u32bit trOutQSz,
        u32bit rastQSz, u32bit testQSz, u32bit intQSz, u32bit shaderQSz,
        char **vshPrefixes, char **fshPrefixes, u32bit nStampUnits, char **suPrefixes, char *name, Box* parent);

    /**
     *
     *  FragmentFIFO destructor.
     *
     *  Closes files and such.
     *
     */
     
    ~FragmentFIFO();
    
    /**
     *
     *  Fragment FIFO class simulation function.
     *
     *  Simulates a cycle of the Fragment FIFO box.
     *
     *  @param cycle The cycle to simulate in the Fragment FIFO box.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Fragment FIFO box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);
    
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
