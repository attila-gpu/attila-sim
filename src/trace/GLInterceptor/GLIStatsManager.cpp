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

#include "GLIStatsManager.h"
#include "GLResolver.h"
#include "GLJumpTable.h"
#include <fstream>
#include "support.h"
#include "ShProgramInfo.h"
#include "includelog.h"

using includelog::logfile; // make log object visible
using namespace std;

bool GLIStatsManager::callPlayable[] =
{
    #include "PlayableCalls.gen"
};

GLIStatsManager::GLIStatsManager() : 
maxFrame(0), 
minFrame(-1),
currentFrame(1),
currentBatch(0),
perBatchStats(true),
acm(ADDCALL),
pos(0)
{
    int i;
    for ( i = 0; i < APICall_UNDECLARED; i++ )
    {
        statCallActive[i] = true; /* by default all calls will produce its counting */
        frames[i].first = NULL;
        frames[i].last = NULL;
        batches[i].first = NULL;
        batches[i].last = NULL;
    }
    
    memset(callCounter, 0, sizeof(callCounter));

    batchesInFrameCount.reserve(1000); // optimization

}

GLIStat* GLIStatsManager::addUserStat(GLIStat* stat)
{
    UserStat us(stat);
    us.batch.push_back(vector<int>()); // create one entry for batches of first frame
    userStats.push_back(us); // add the new user statistic
    return stat;
}

void GLIStatsManager::setPerBatchStats( bool mode )
{
    perBatchStats = mode;
}

bool GLIStatsManager::isPerBatchStats() const
{
    return perBatchStats;
}

void GLIStatsManager::setPerFrameStats( bool mode )
{
    perFrameStats = mode;
}

bool GLIStatsManager::isPerFrameStats() const
{
    return perFrameStats;
}

void GLIStatsManager::init( int startFrame )
{
    currentFrame = startFrame-1;
    memset(callCounter,0, sizeof(callCounter));
    pos = 0;
}


void GLIStatsManager::setStatCall( APICall call, bool active )
{
    statCallActive[call] = active;
}


bool GLIStatsManager::getStatCall( APICall call )
{
    return statCallActive[call];
}



void GLIStatsManager::addCallBeginBatch()
{
    acm = ADDCALL_BEGINBATCH;
}

void GLIStatsManager::addCallEndBatch()
{
    acm = ADDCALL_ENDBATCH;
}

void GLIStatsManager::addCallEndFrame()
{
    acm = ADDCALL_ENDFRAME;
}

void GLIStatsManager::incCall( APICall apicall, int incValue )
{
    if ( statCallActive[apicall] )
        callCounter[pos][apicall] += incValue;

    /* @efficiency -> Skip next 3 checkings */
    /* This is the common branch */
    if ( acm == ADDCALL )
        return ;

    if ( acm == ADDCALL_BEGINBATCH )
    {
        beginBatch();
        acm = ADDCALL;
        return ;
    }

    if ( acm == ADDCALL_ENDBATCH )
    {
        endBatch();
        acm = ADDCALL;
        return ;
    }
    
    if ( acm == ADDCALL_ENDFRAME )
    {
        endFrame();
        acm = ADDCALL;
        return ;
    }    
}


void GLIStatsManager::beginBatch()
{
    if ( pos == 0 )
    {
        if ( perBatchStats )
            pos++;
    }
    else
    {
        panic("GLIStatsManager", "startBatch()", "Error. We are already in a batch");
    }
    
    /* Reset counters done in endBatch */    
}


void GLIStatsManager::endBatch()
{    
    vector<int> newVect;
    vector<UserStat>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {    
        int batchCount = it->stat->getBatchCount();
        it->batch.back().push_back(batchCount);
    }
    
    if ( pos == 1 )
        pos--;
    else
    {
        /* pos != 1 */
        if ( perBatchStats )
            panic("GLIStatsManager", "endBatch()", "Error. We are not in a batch");

        /* else: counting is already in current frame */
        /* We do not have to add current batch counting to current frame counting */

        //as.getTrianglesPerBatch();
        currentBatch++;
        nBatches++;
        //as.resetBatchStats();
        
        return ;
    }
    
    /* increase frame counters with batch counting */
    /* Can be optimized reducing the number of functions supported */
    /* Reduce the code generator functions generated */

    /* This loop is a Bottleneck if a frame contains lots of small batches */
    /* Examples: viewperf & tenebrae */
    if ( !perBatchStats )
    {
        for ( int i = 0; i < APICall_UNDECLARED; i++ )
        {
            if ( callCounter[1][i] != 0 )
            {
                callCounter[0][i] += callCounter[1][i];
                /* Batch stats disabled for now */
                add((APICall)i, currentFrame, currentBatch, callCounter[1][i]); /* store in batch stats */
                callCounter[1][i] = 0; /* reset batch counter */
            }
        }
    }

    currentBatch ++; /* total batches in current frame */
    nBatches++; /* total batches */    
}


void GLIStatsManager::endFrame()
{
    if ( pos != 0 )
        panic("GLIStatsManager","endFrame()", "Error. We are not in a frame");

    int i;

    for ( i = 0; i < APICall_UNDECLARED; i++ )
    {
        if ( callCounter[0][i] != 0 )
        {
            add((APICall)i, currentFrame, callCounter[0][i]); /* store in frame stats */
            callCounter[0][i] = 0; /* reset counter */
        }
    }


/////// Error aquí... (sospecha, división por 0)
    vector<UserStat>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        int frameCount = it->stat->getFrameCount();
        it->frame.push_back(frameCount);
        it->batch.push_back(vector<int>()); // create a new entry for batches of next frame
    }
//////// Fin error

    currentFrame++;
    
    batchesInFrameCount.push_back(currentBatch);
    currentBatch = 0; /* reset batch index */    
}


void GLIStatsManager::add( APICall apicall, int frame, int batch, int count )
{
    CCount* cc = new CCount;
    
    cc->frame = frame;
    cc->count = count;
    cc->batch = batch;

    batches[apicall].add(cc);
}

void GLIStatsManager::add( APICall apicall, int frame, int count)
{
    CCount* cc = new CCount;
    
    cc->frame = frame;
    cc->count = count;
    cc->batch = -1; /* not included in any batch */

    frames[apicall].add(cc);
}


void GLIStatsManager::CCountList::add( CCount* cc )
{
    if ( first == NULL )
    {
        first = cc;
        last = cc;
        last->next = NULL;
    }
    else
    {
        last->next = cc;
        last = cc;
        last->next = NULL;
    }
}


bool GLIStatsManager::dumpFrameStatsVertical( const char* file, int firstFrame, int lastFrame )
{
    firstFrame--;

    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        return false; // the file could not be opened to write
    
    int i, j;

    vector<int> calls; /* store index calls used in this trace */
    vector<CCount *> ptrs; /* store current start frame (CCount object associated to initial frame) */

    /* Create label row */
    f << "FRAME STATS";

    for ( i = 0; i < APICall_UNDECLARED; i++ )
    {
        if ( frames[i].first != NULL )
        {
            f << ";" << GLResolver::getFunctionName((APICall)i);
            if ( !callPlayable[i] )
                f << "*"; /* Indicates call is not playable by GLPlayer */

            /* skip previous frames */
            CCount* ptr = frames[i].first;
            while ( ptr != NULL && ptr->frame < firstFrame )
                ptr = ptr->next;

            ptrs.push_back(ptr); /* store the next frame equal or greater than firstFrame */
            
            calls.push_back(i); /* this call has been done at least one time in the trace */
        }
    }

    f << ";BATCHES"; // It is a built-in stat

    // Prints user stats name
    vector<UserStat>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        f << ";" << it->stat->getName();
    }
    f << "\n";

    /* Create data rows */
    for ( i = firstFrame; i < lastFrame; i++ )
    {
        /* Write frame */
        f << (i+1) << ";";

        /* Write all call counting in this frame */
        for ( j = 0; j < calls.size(); j++ ) /* loop over all calls done in this trace */
        {
            CCount* ptr =  ptrs[j];
            if ( ptr != NULL && ptr->frame == i )
            {
                 f << ptr->count;
                 ptrs[j] = ptr->next;
            }
            f << ";";
        }

        /* Write Stats */
        //f << vertexBatchCount[i].size() << ";"; /* Batches */
        f << batchesInFrameCount[i] << ";"; /* Batches */

        vector<UserStat>::iterator it2 = userStats.begin();
        for ( ; it2 != userStats.end(); it2++ )
        {
            if ( it2 != userStats.begin() )
                f << ";";
            f << it2->frame[i];
        }
        f << "\n";
    }

    return true;
}


bool GLIStatsManager::dumpFrameStatsHorizontal( const char* file, int firstFrame, int lastFrame )
{
    return false;

    panic("GLIStatsManager", "dumpFrameStatsHorizontal", "Not implemented yet");

    /*
    firstFrame--;    

    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        panic("GLIStatsManager","dumpFrameStats()", "Error opening oputput file");

    int i, j;

    f << "FRAME STATS";

    j = 0;

    for ( i = firstFrame; i < lastFrame; i++ )
        f << ";Frame " << i+1;

    f << endl;
    
    for ( i = 0; i < APICall_UNDECLARED; i++ )
    {
        CCount* ptr = frames[i].first;
        if ( ptr != NULL )
        {
            // Skip previous frames 
            while ( ptr != NULL && ptr->frame < firstFrame )
                ptr = ptr->next;

            f << GLResolver::getFunctionName((APICall)i);
            if ( !callPlayable[i] )
                f << " *"; // indicates this call is not playable 
            f << ";";
            for ( j = firstFrame; j < lastFrame; j++ )
            {
                if ( ptr != NULL && ptr->frame == j )
                {
                    f << ptr->count;
                    ptr = ptr->next;
                }
                if ( j < lastFrame-1 )
                    f << ";";
            }
            f << endl;
        }
    }
    f << "BATCHES";
    for ( j = firstFrame; j < lastFrame; j++ )
    {
        //f << ";" << vertexBatchCount[j].size();
        f << ";" << batchesInFrameCount[j];
    }
    f << endl;

    f << "VERTEXES";

    int totalSum = 0;
    for ( j = firstFrame; j < lastFrame; j++ )
        f << ";" << vertexFrameCount[j];

    f << endl << "TRIANGLES";

    for ( j = firstFrame; j < lastFrame; j++ )
        f << ";" << triangleFrameCount[j];

    f << endl << "AVG. VSH INSTRS. PER VERTEX";
    for ( j = firstFrame; j < lastFrame; j++ )
        f << ";" << avgVshInstrPerFrame[j];

    f << endl << "AVG. FSH INSTR. PER FRAGMENT";
    for ( j = firstFrame; j < lastFrame; j++ )
        f << ";" << avgFshInstrPerFrame[j];

    f << endl << "AVG. FSH TEXTURE INSTRS. PER FRAGMENT";
    for ( j = firstFrame; j < lastFrame; j++ )
        f << ";" << avgFshTexInstrPerFrame[j];

    f.close();
    */
}


bool GLIStatsManager::dumpFrameStats( const char* file, bool vertical, int splitLevel )
{    
    if ( !perFrameStats )
    {
        logfile().pushInfo("GLIStatsManager::dumpFrameStats");
        logfile().write(includelog::Init, "Skipped (perFrameStats = DISABLED)\n");
        logfile().popInfo();
        return false;
    }

    if ( splitLevel <= 0 )
    {
        if ( vertical )
            return dumpFrameStatsVertical(file, 1, currentFrame-1);
        
        return dumpFrameStatsHorizontal(file, 1, currentFrame-1);
    }

    int iFile, nFiles;
    char fileName[256];

    nFiles = (currentFrame / (splitLevel+1)) +1; /* splitLevel columns with data + 1 column with labels */

    bool ok = true;
    for ( iFile = 0; iFile < nFiles && ok; iFile++ )
    {
        int firstFrame = iFile * splitLevel + 1;
        int lastFrame = ((iFile+1)*splitLevel > currentFrame ? currentFrame : (iFile+1)*splitLevel);
        sprintf(fileName,"%s_%d_%d.csv", file, firstFrame, lastFrame);
        if ( vertical )
            ok = dumpFrameStatsVertical(fileName, firstFrame, lastFrame);
        else
            ok = dumpFrameStatsHorizontal(fileName, firstFrame, lastFrame);
    }

    return ok;
}


void GLIStatsManager::dumpBatchStats( const char* file )
{    
    
    if ( !perBatchStats )
    {
        logfile().pushInfo("GLIStatsManager::dumpBatchStats");
        logfile().write(includelog::Init, "Skipped (perBatchStats = DISABLED)\n");
        logfile().popInfo();
        return ;
    }

    panic("GLIStatsManager", "dumpBatchStats", "Not implemented yet");

    /*
    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        panic("GLIStatsManager", "dumpBatchStats()", "Error opening output file");
    

    int i, j, k, nB;

    f << "BATCH STATS";
    
    j = 0;
    
    for ( i = 0; i < currentFrame; i++ )
    {
        nB = vertexBatchCount[i].size();
        for ( j = 0; j < nB; j++ )
            f << ";Batch " << j+1 << " (F:" << i+1 << ")";
    }

    f << endl;

    for ( i = 0; i < APICall_UNDECLARED; i++ )
    {
        CCount* ptr = batches[i].first;
        if ( batches[i].first != NULL )
        {
            f << GLResolver::getFunctionName((APICall)i) << ";";
            for ( j = 0; j < currentFrame; j++ )
            {
                nB = vertexBatchCount[j].size(); // obtain number of batches in 'j' frame
                for ( k = 0; k < nB; k++ )
                {
                    if ( ptr != NULL && ptr->batch == k )
                    {
                        f << ptr->count;
                        ptr = ptr->next;
                    }
                    if ( j < currentFrame-1  || k < nB-1)
                        f << ";";
                }
            }
            f << endl;
        }
        
    }

    f << "PRIMITIVE";
    for ( j = 0; j < nBatches; j++ )
    {
        f << ";" << GLResolver::getConstantName(batchPrimitives[j]);
    }



    f << endl << "VERTEXES";

    for ( j = 0; j < currentFrame; j++ )
    {
        nB = vertexBatchCount[j].size();
        for ( k = 0; k < nB; k++ )
            f << ";" << vertexBatchCount[j][k];
    }

    f << endl << "TRIANGLES";

    for ( j = 0; j < currentFrame; j++ )
    {
        nB = vertexBatchCount[j].size();
        for ( k = 0; k < nB; k++ )
            f << ";" << triangleBatchCount[j][k];
    }

    f.close();
    */
}

