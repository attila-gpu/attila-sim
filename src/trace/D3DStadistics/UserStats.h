#ifndef USER_STAT_H
    #define USER_STAT_H

#include <string>
#include <iostream>

namespace workloadStats
{

/**
 * Common interface for all user-defined stats
 */
class UserStat
{
protected:
    
    std::string name;  ///< User Stat name.

public:
	UserStat() {};
    UserStat(const std::string& name): name(name) {};
	void setName(const std::string &n) {name = n;}
	
    virtual void endBatch()=0;
    virtual void endFrame()=0;
    virtual void endTrace()=0;

    virtual void printTraceValue(std::ostream& os) const = 0;
    virtual void printFrameValue(std::ostream& os, unsigned int frameIndex) const = 0;
    virtual void printBatchValue(std::ostream& os, unsigned int frameIndex, unsigned int batchIndex) const = 0;

    virtual const std::string& getName() const { return name; }

    virtual ~UserStat() {};
};

} // namespace workloadStats

#endif // USER_STAT_H
