#ifdef WORKLOAD_STATS

#include "UserStats.h"
#include "StatsManager.h"
#include <fstream>
#include <iostream>
#include "support.h"
#include "Log.h"

using namespace std;
using namespace workloadStats;

/* Create singleton */
StatsManager& StatsManager::instance()
{

	/**
	 * @fix
	 * 
	 * Previous code caused a memory leak. The changes applied follow this
	 * article:
	 *
	 *      http://gethelp.devx.com/techtips/cpp_pro/10min/10min0200.asp
	 *
	 * Previous code:
	 *
	 * if ( !sm )
	 *     sm = new StatsManager(); 
	 * return *sm;
	 **/

	static StatsManager sm;
	return sm;
}

StatsManager::StatsManager(bool ownership) : 
maxFrame(0), 
minFrame(-1),
currentFrame(1),
currentBatch(0),
nBatches(0),
perBatchStats(true),
perFrameStats(true),
perTraceStats(true),
stillInFrame(false),
ownership(ownership),
pos(0)
{
    int i;

    batchesInFrameCount.reserve(1000); // optimization
}


StatsManager::~StatsManager()
{
    if (ownership)
	{
		map<string, UserStat*>::iterator it = userStats.begin();
		for ( ; it != userStats.end(); it++ )
		{   
			delete (*it).second;
		}
	}
};


UserStat* StatsManager::addUserStat(UserStat* stat)
{
    if(userStats.find(stat->getName()) == userStats.end())
    {
        userStats[stat->getName()] = stat;
        return stat;    
    }
    else
        panic("StatsManager.cpp","addUserStat","Already exists another User Stat with the same name");
}

UserStat* StatsManager::getUserStat(string name)
{
    map<string, UserStat*>::iterator returnStat;
    
    if((returnStat = userStats.find(name)) != userStats.end())
    {
        return returnStat->second;
    }
    else
        panic("StatsManager.cpp","getUserStat","User Stats does not exists");    
}

void StatsManager::setPerTraceStats( bool mode )
{
    perTraceStats = mode;
}

bool StatsManager::isPerTraceStats() const
{
    return perTraceStats;
}

void StatsManager::setPerBatchStats( bool mode )
{
    perBatchStats = mode;
}

bool StatsManager::isPerBatchStats() const
{
    return perBatchStats;
}


void StatsManager::setPerFrameStats( bool mode )
{
    perFrameStats = mode;
}

bool StatsManager::isPerFrameStats() const
{
    return perFrameStats;
}

void StatsManager::init( int startFrame )
{
    currentFrame = startFrame-1;
    pos = 0;
    stillInFrame = false;
}

void StatsManager::beginBatch()
{
    //cout << "ENTERING BEGIN BATCH" << endl;
	if ( pos == 0 )
    {
        if ( perBatchStats )
            pos++;
    }
    else
    {
        panic("StatsManager", "startBatch()", "Error. We are already in a batch");
    }
    
    if (!stillInFrame)
        stillInFrame = true;
    
}


void StatsManager::endBatch()
{   
    //cout << "ENTERING END BATCH" << endl;
    
    vector<int> newVect;
    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {   
        (*it).second->endBatch();
    }
    
    if ( pos == 1 )
        pos--;
    else
    {
        // pos != 1
        if ( perBatchStats )
            panic("StatsManager", "endBatch()", "Error. We are not in a batch");

        // else: counting is already in current frame
        // We do not have to add current batch counting to current frame counting

        currentBatch++;
        nBatches++;
         
        return ;
    }
    
    /* increase frame counters with batch counting */
    /* Can be optimized reducing the number of functions supported */
    /* Reduce the code generator functions generated */

    currentBatch ++; /* total batches in current frame */
    nBatches++; /* total batches */ 
}


void StatsManager::endFrame()
{
    //cout << "ENTERING END FRAME" << endl;
    
    if ( pos != 0 )
        panic("StatsManager","endFrame()", "Error. We are not in a frame");

    stillInFrame = false;
    
    int i;

    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        (*it).second->endFrame();
    }

    currentFrame++;
    
    batchesInFrameCount.push_back(currentBatch);
    currentBatch = 0; /* reset batch index */   
}

void StatsManager::endTrace()
{   
    if (stillInFrame)
        endFrame();
 
    //cout << "ENTERING END TRACE" << endl;
           
    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
        (*it).second->endTrace();
        
    dumpBatchStats("statsPerBatch.csv", true, 0);
    dumpFrameStats("statsPerFrame.csv", true, 0);
    dumpTraceStats("statsTraceFile.csv", true);
}


bool StatsManager::dumpBatchStatsVertical( const char* file , int firstFrame, int firstBatch, int lastFrame, int lastBatch)
{   
    firstFrame--;
    firstBatch--;

    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        return false; // the file could not be opened to write

    unsigned int i, j, k, nB;

    f << "BATCH STATS";

    // Prints user stats name

    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        f << ";" << (*it).second->getName();
    }
    f << "\n";

    for ( i = firstFrame; i < lastFrame; i++ )
    {
        nB = batchesInFrameCount[i];
        
        if (i == firstFrame && nB > firstBatch)
            j = firstBatch;
        else
            j = 0;

        if (i == lastFrame - 1 && nB > lastBatch)
            nB = lastBatch;

        for (; j < nB; j++ )
        {
            
            f << "Batch " << j+1 << " (F:" << i+1 << ");";

            // Write Stats
            
            map<string, UserStat*>::iterator it2 = userStats.begin();
            for ( ; it2 != userStats.end(); it2++ )
            {
                if (it2 != userStats.begin())
                    f << ";";
    
                (*it2).second->printBatchValue(f,i,j);
            }
            f << "\n";
            
        }
    }

    f.close();
    return true;
}

bool StatsManager::dumpBatchStatsHorizontal( const char* file , int firstFrame, int firstBatch, int lastFrame, int lastBatch)
{   
    firstFrame--;
    firstBatch--;
    
    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        return false; // the file could not be opened to write

    int i, j, k, nB;

    f << "BATCH STATS";
    
    for ( i = firstFrame; i < lastFrame; i++ )
    {
        nB = batchesInFrameCount[i];
        
        if (i == firstFrame && nB > firstBatch)
            j = firstBatch;
        else
            j = 0;

        if (i == lastFrame - 1 && nB > lastBatch)
            nB = lastBatch;

        for (; j < nB; j++ )
            f << ";Batch " << j+1 << " (F:" << i+1 << ")";
    }

    f << endl;

    // User Stats

    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        f << (*it).second->getName();
        
        for ( i = firstFrame; i < lastFrame; i++ )
        {
            nB = batchesInFrameCount[i];
            
            if (i == firstFrame && nB > firstBatch)
                j = firstBatch;
            else
                j = 0;

            if (i == lastFrame - 1 && nB > lastBatch)
                nB = lastBatch;

            for (; j < nB; j++ )
            {
                f << ";" ; (*it).second->printBatchValue(f,i,j);
            }
        }

        f << endl;
    }

    f << endl;
    
    f.close();

    return true;
}

void StatsManager::dumpBatchStats(const char* file, bool vertical, int splitLevel)
{
    if ( !perBatchStats )
    {
        LOG( 0, Log::log() << "Skipped (perBatchStats = DISABLED)\n")
        return;
    }

    bool ok = true;

    if ( splitLevel <= 0 )
    {
        if ( vertical )
            ok = dumpBatchStatsVertical(file, 1, 1, currentFrame-1, currentBatch-1);
        else
            ok = dumpBatchStatsHorizontal(file, 1, 1, currentFrame-1, currentBatch-1);
    }
    else
    {
        unsigned int total_batches = 0;

        for (unsigned int i=0; i < currentFrame; i++)
            total_batches += batchesInFrameCount[i];

        unsigned int iFile, nFiles;
        char fileName[256];

        unsigned int frameCount = 0;
        unsigned int batchesCount = 0;
        unsigned int frameBatchesCount = 0;
        unsigned int firstFrame, firstBatch, lastFrame, lastBatch;

        nFiles = (total_batches / (splitLevel+1)) +1; // splitLevel columns with data + 1 column with labels

        while ( batchesCount < total_batches && ok )
        {
            firstFrame = frameCount;
            firstBatch = frameBatchesCount;
            
            frameBatchesCount += splitLevel;
            
            while (frameBatchesCount >= batchesInFrameCount[frameCount])
            {
                frameBatchesCount =- batchesInFrameCount[frameCount];
                frameCount++;
            }

            batchesCount += splitLevel;

            lastFrame = frameCount;
            lastBatch = frameBatchesCount;

            sprintf(fileName,"%s_%d-%d_%d-%d.csv", file, firstFrame + 1, firstBatch + 1, lastFrame + 1, lastBatch + 1);
            if ( vertical )
                ok = dumpBatchStatsVertical(fileName, firstFrame + 1, firstBatch + 1, lastFrame, lastBatch);
            else
                ok = dumpBatchStatsHorizontal(fileName, firstFrame + 1, firstBatch + 1, lastFrame, lastBatch);

        }
    }
    if ( ok )
    {
        LOG(0, Log::log() << " OK\n";)
    }
    else
    {
        LOG(0, Log::log() << " File could not be opened for writing (maybe was already opened)\n";)
    }
}

bool StatsManager::dumpFrameStatsVertical( const char* file, int firstFrame, int lastFrame )
{
    firstFrame--;

    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        return false; // the file could not be opened to write
    
    unsigned int i, j;

    /* Create label row */
    f << "FRAME STATS";

    /* API Calls accounting dump */

    f << ";BATCHES"; // It is a built-in stat

    // Prints user stats name
    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        f << ";" << (*it).second->getName();
    }
    f << "\n";

    /* Create data rows */
    for ( i = firstFrame; i < lastFrame; i++ )
    {
        /* Write frame */
        f << (i+1) << ";";

        /* Write Stats */
        
        f << batchesInFrameCount[i] << ";"; /* Batches */

        map<string, UserStat*>::iterator it2 = userStats.begin();
        for ( ; it2 != userStats.end(); it2++ )
        {
            if ( it2 != userStats.begin() )
                f << ";";
            (*it2).second->printFrameValue(f,i);
        }
        f << "\n";
    }

    f.close();
    
    return true;
}


bool StatsManager::dumpFrameStatsHorizontal( const char* file, int firstFrame, int lastFrame )
{
    firstFrame--;   

    ofstream f;
    f.open(file);
    if ( !f.is_open() )
        return false;

    int i, j;

    /* Create labels for frame columns */
    f << "FRAME STATS";

    j = 0;

    for ( i = firstFrame; i < lastFrame; i++ )
        f << ";Frame " << i+1;

    f << endl;
    
    // Built-in Stats
    
    f << "BATCHES"; 
    for ( j = firstFrame; j < lastFrame; j++ )
    {
        f << ";" << batchesInFrameCount[j];
    }
    f << endl;

    // The remain of built-in Stats
    map<string, UserStat*>::iterator it = userStats.begin();
    for ( ; it != userStats.end(); it++ )
    {
        f << (*it).second->getName();

        for ( j = firstFrame; j < lastFrame; j++ )
        {
            f << ";" ; (*it).second->printFrameValue(f,j);
        }

        f << endl;
    }
    f << "\n";

    f.close();

    return true;
}


void StatsManager::dumpFrameStats( const char* file, bool vertical, int splitLevel )
{   
    if ( !perFrameStats )
    {
        LOG( 0, Log::log() << "Skipped (perFrameStats = DISABLED)\n")
        return;
    }

    bool ok = true;

    if ( splitLevel <= 0 )
    {
        if ( vertical )
            ok = dumpFrameStatsVertical(file, 1, currentFrame-1);
        else
            ok = dumpFrameStatsHorizontal(file, 1, currentFrame-1);
    }
    else
    {
        int iFile, nFiles;
        char fileName[256];

        nFiles = (currentFrame / (splitLevel+1)) +1; /* splitLevel columns with data + 1 column with labels */

        
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
    }
    if ( ok )
    {
        LOG(0, Log::log() << " OK\n";)
    }
    else
    {
        LOG(0, Log::log() << " File could not be opened for writing (maybe was already opened)\n";)
    }
}

void StatsManager::dumpTraceStats( const char* file, bool vertical )
{
    if ( !perTraceStats )
    {
        LOG( 0, Log::log() << "Skipped (perTraceStats = DISABLED)\n")
        return;
    }
    
    ofstream f;
    f.open(file);
    if ( f.is_open() )
    {
        LOG(0, Log::log() << " OK\n";)
    }
    else
    {
        LOG(0, Log::log() << " File could not be opened for writing (maybe was already opened)\n";)
        return;
    }
    
    /* Write Stats */

    int i, j;

    f << "TRACE STATS";
            
    if (vertical)
    {
        f << ";FRAMES;BATCHES;";

        map<string, UserStat*>::iterator it = userStats.begin();
        for ( ; it != userStats.end(); it++ )
        {
            f << (*it).second->getName() << ";";
        }
        
        f << "\n;";

         /* Print number of frames */
        f << currentFrame-1 << ";";

        unsigned int totalBatches = 0;

        for(int i = 0; i < (currentFrame - 1); i++)
            totalBatches += batchesInFrameCount[i];

        f << totalBatches << ";";

        it = userStats.begin();

        for ( ; it != userStats.end(); it++ )
        {
            (*it).second->printTraceValue(f); f << ";";
        }
        f << "\n";
    }
    else    // Horizontal dump
    {
        f << "\n";

        f << "FRAMES;" << currentFrame-1 << "\n";
        
        unsigned int totalBatches = 0;

        for(int i = 0; i < (currentFrame - 1); i++)
            totalBatches += batchesInFrameCount[i];

        f << "BATCHES;" << totalBatches << "\n";

        map<string, UserStat*>::iterator it = userStats.begin();
        for ( ; it != userStats.end(); it++ )
        {
            f << (*it).second->getName() << ";"; (*it).second->printTraceValue(f); f << "\n";
        }
    }

    f.close();
    return;
};

#endif // WORKLOAD_STATS