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

#ifndef GLISTATSMANAGER_H
    #define GLISTATSMANAGER_H

#include "GLResolver.h"
#include <vector>
#include "GLIStat.h"

class GLIStatsManager {

private:

    static bool callPlayable[];

    struct CCount /* call counting */
    {
        int frame; /* or frame or batch */
        int count;
        int batch;
        CCount* next;
    };

    struct CCountList
    {
        CCount* first;
        CCount* last;

        void add(CCount* cc);
    };

    CCountList frames[APICall_UNDECLARED]; /* stored frames info */
    CCountList batches[APICall_UNDECLARED]; /* stored batches info */
    
    bool statCallActive[APICall_UNDECLARED];

    int minFrame;
    int maxFrame;

    /**
     * [0] -> outside of batch
     * [1] -> inside a batch
     */
    int callCounter[2][APICall_UNDECLARED];

    /** 
     * selector: 0 means outside of a batch
     *           1 means inside of a batch
     */
    int pos;


    /* Method for adding partial stats to final stats */
    void add( APICall apicall, int frame, int count ); /* add frame stat */
    void add( APICall apicall, int frame, int batch, int count); /* add batch stat */
    
    /* flags for control begin/end of batches and frames */

    enum  AddCallMode
    {
        ADDCALL, /* default */
        ADDCALL_ENDFRAME,
        ADDCALL_ENDBATCH,
        ADDCALL_BEGINBATCH
    };

    /* current Add Call Mode */
    AddCallMode acm;

    bool perBatchStats;
    bool perFrameStats;

    struct UserStat
    {
        GLIStat* stat;
        std::vector<int> frame; // information per frame
        std::vector<std::vector<int> > batch; // information per batch (frame:batch)

        UserStat(GLIStat* stat) : stat(stat) {}
    };

    std::vector<int> batchesInFrameCount; // Batches per frame is a built-in stat

    std::vector<UserStat> userStats;
    

    /* advanced frame stats */
    /*
    std::vector<int> triangleFrameCount;
    std::vector<int> vertexFrameCount;
    

    std::vector<double> avgVshInstrPerFrame; ///< Average number of vsh instructions (per vertex) per frame
    std::vector<double> avgFshInstrPerFrame; ///< Average number of fsh instructions (per fragment) per frame
    std::vector<double> avgFshTexInstrPerFrame; ///< Average number of texture instructions (per fragment) per frame

    // advanced batch stats 
    std::vector<std::vector<int> > triangleBatchCount;
    std::vector<std::vector<int> > vertexBatchCount;
    std::vector<GLenum> batchPrimitives;

    std::vector<std::vector<int> > vshInstrPerBatch; ///< Number of vsh instructions (per vertex) per batch
    std::vector<std::vector<int> > fshInstrPerBatch; ///< Number of fsh instructions (per fragment) per batch
    std::vector<std::vector<int> > fshTexInstrPerBatch; ///< Number of texture instructions (per fragment) per batch

    */

    int currentFrame;
    int currentBatch; /* current batch of current frame */
    int nBatches; /* number of batches */

    /* AdvancedObject storing partially result for a frame */
    //AdvancedStats as;

    bool dumpFrameStatsHorizontal( const char* file, int firstFrame, int lastFrame );

    bool dumpFrameStatsVertical( const char* file, int firstFrame, int lastFrame );
    
public:

    GLIStatsManager();

    /**
     * Adds a new user stat 
     *
     * @see GLIStat.h
     */
    GLIStat* addUserStat(GLIStat* userStat);

    void setPerBatchStats( bool mode );
    bool isPerBatchStats() const;

    void setPerFrameStats( bool mode );
    bool isPerFrameStats() const;

    void setStatCall( APICall call, bool active );

    bool getStatCall( APICall call );

    /* initialize start frame */
    /* and reset all previous data */
    void init( int startFrame );

    /* mark batch start */
    void beginBatch();

    /* mark batch end */
    void endBatch();

    /* mark frame start */
    void beginFrame();

    /* mark frame end */
    void endFrame();

    /* inc counting */
    void incCall( APICall apicall, int incValue = 1 );

    /* 
     * the next incCall will be counted in the current batch
     * and after this incCall the batch will be considered ended
     * ( endBatch() )
     */
    void addCallEndBatch();
    
    /* 
     * the next incCall will be counted in the current frame
     * and after this incCall the frame will be considered ended
     * ( endFrame() )
     */
    void addCallEndFrame();

    /**
     * The next incCall will count the call and after that a
     * beginBatch() will be performed
     */
    void addCallBeginBatch();
    
    void dumpBatchStats( const char* file );

    bool dumpFrameStats( const char* file, bool vertical = false, int splitLevel = 0 );    

    void dumpTotalStats( const char* file );

};

#endif // GLISTATSMANAGER_H