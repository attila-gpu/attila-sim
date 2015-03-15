/********************************************************************
Dependencies: UserStats.h
********************************************************************/

#ifndef STATSMANAGER_H
    #define STATSMANAGER_H

#include <vector>
#include <map>
#include <string>


namespace workloadStats
{

class StatsManager {

private:

    StatsManager(bool ownership = false);
    
    unsigned int minFrame;
    unsigned int maxFrame;

    /** 
     * selector: 0 means outside of a batch
     *           1 means inside of a batch
     */
    int pos;

    /**
     * New batches specified for the current frame and not ended frame yet. 
     */
    bool stillInFrame;
    
    /**
     * Dump information selection
     */
    bool perBatchStats;
    bool perFrameStats;
    bool perTraceStats;

    std::vector<unsigned int> batchesInFrameCount; // Batches per frame is a built-in stat

    std::map<std::string, UserStat*> userStats;

    unsigned int currentFrame;
    unsigned int currentBatch; /* current batch of current frame */
    unsigned int nBatches; /* number of batches */

	bool ownership; ///< Deleting UserStats is responsibility of StatsManager

    bool dumpFrameStatsHorizontal( const char* file, int firstFrame, int lastFrame );

    bool dumpFrameStatsVertical( const char* file, int firstFrame, int lastFrame );

    bool dumpBatchStatsHorizontal( const char* file , int firstFrame, int firstBatch, int lastFrame, int lastBatch);

    bool dumpBatchStatsVertical( const char* file , int firstFrame, int firstBatch, int lastFrame, int lastBatch);
    
public:
    
    static StatsManager& StatsManager::instance();

    ~StatsManager();
    /**
     * Adds a new user stat 
     *
     * @see StatsUtils.h
     */
    UserStat* addUserStat(UserStat* userStat);

    /**
     * Gets a user stat by the name
     */
    UserStat* getUserStat(std::string name);
    
    void setPerBatchStats( bool mode );
    bool isPerBatchStats() const;

    void setPerFrameStats( bool mode );
    bool isPerFrameStats() const;

    void setPerTraceStats( bool mode );
    bool isPerTraceStats() const;

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

    /* mark trace end */
    void endTrace();

    void dumpBatchStats(const char* file, bool vertical = false, int splitLevel = 0 );

    void dumpFrameStats( const char* file, bool vertical = false, int splitLevel = 0 ); 

    void dumpTraceStats( const char* file, bool vertical = false );

};

} // namespace workloadStats

#endif // STATSMANAGER_H
