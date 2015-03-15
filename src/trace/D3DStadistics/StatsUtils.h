#ifndef STATS_UTILS_H
    #define STATS_UTILS_H

#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <functional>

#ifndef WIN32
    #include <function.h>
#endif 

namespace workloadStats
{
/****************************************************************************************************
 *                                      workloadStats NameSpace                                     *
 ****************************************************************************************************
 *                                                                                                  *
 *      This namespace contains the classes and structures used to get Workload statistics          *
 *  for the traced OpenGL application.                                                              *
 *                                                                                                  *
 *      The statistical mode of GLInterceptor is enabled specifying any name different to null      *
 *  in the batchStats, frameStats and traceStats fields of GLIConfig.ini file.                      *
 *                                                                                                  *
 *      After execution, in each of these files, the related and resumed statistics per batch, per  *
 *  per frame and for the entire trace are available.                                               *
 *                                                                                                  *
 ****************************************************************************************************
 */
 
/****************************************************************************************************
 *                                      StatsUtils.h classes                                        *
 ****************************************************************************************************
 *                                                                                                  *
 *      StatsUtils.h defines a collection of classes and collectors that can be used to construct	*
 *	several different types of accounting statistics taking or summarizing information for each		*
 *	batch, for each frame and the entire trace.                                                     *
 *                                                                                                  *
 *      At the end of this file, a definition of the UserStat interface is given to derive all the  *
 *  user statistics.                                                                                *
 ****************************************************************************************************

/**
 *  Aggregator class generalizes the idea of an aggregation operation over a vector of generic data.
 *
 *  This interface is implemented using templates to allow working with different basic types of data,
 *  either for the vector data type and the result type.
 */

template<class T1, class T2>
class Aggregator
{
public:
    
    virtual T2 getAggregatedValue(const std::vector<T1>& values) const = 0;

    /* Virtual destructor to call the derived class destructors. */
    virtual ~Aggregator() {};
};

/**
 *  The TraceDataCollector class is the main base class of data collectors to construct statistics.
 *
 *      It generalizes the idea of an object that carries out accounting using
 *  a frame Aggregator to resume batch statistic values into the frame statistic 
 *  value (when the endFrame() method is invoked) and a trace Aggregator to resume 
 *  this frame statistic values to a final trace statistic value (when the endTrace())
 *  method is invoked.
 *
 *      The statistic batch data type (BT), the frame resumed data type (FT) and the 
 *  trace data type (TT) can be different, and they are parametrizable because of the 
 *  template based implementation of this class. The corresponding aggregators are forced 
 *  to use the same data types. An example of this can be a TraceDataCollector subclass that uses
 *  an integer type to hold batch values, a float type for frame and trace values that 
 *  allows to use an average aggregator to hold frame and trace info.
 *  
 *  class ConcreteTraceDataCollector: public TraceDataCollector<unsigned int,float,float> // <BT, FT, TT>
 *  {
 *      ....
 *
 *      /**
 *       * The inlined constructor
 *       *
 *      ConcreteTraceDataCollector(const std::string& name)
 *      : TraceDataCollector<unsigned int,float,float>(name)
 *      {
 *          setAggregators(new ConcreteAggregator1<unsigned int, float>, // Frame aggregator
 *                         new ConcreteAggregator2<float, float>);     // Trace aggregator
 *      }
 *    ....
 *  };
 *
 *
 *      The batch values are stored in a vector holding vectors of batch values for each frame.
 *  The frame resumed values are stored in a separated vector and, in the same way, the final trace 
 *  statistic value is stored separately in another attribute.
 *
 *      The methods endBatch(), endFrame(), endTrace() are fully implemented and they
 *  perform and store the current batch or frame value computation. They also perform the 
 *  corresponding switching to the next batch or frame.
 *
 *      The endBatch() and setBatchValue() are declared virtual to allow GLIStat subclasses
 *  to redefine the behaviour and structures used for batch process and switching.
 *
 *      The endFrame() is also declared virtual to allow statistics that only do a special
 *  management at the frame level, for example the Average Pixel Depth Complexity which uses
 *  the frame depth complexity map of the frame to extract this parameter.
 *
 */

template<class BT, class FT, class TT>
class TraceDataCollector
{
protected:

    const Aggregator<BT,FT>*       frameAggregator;
    const Aggregator<FT,TT>*       traceAggregator;

    std::vector<std::vector<BT> >  batchValues;  ///< To store individually batch values over the entire trace. At the end of the frame, the Frame
                                                 ///  aggregator will resume the last filled batch vector in a new frame value.

    mutable std::vector<FT>        frameValues;  ///< To store the frame statistic value over the entire trace. At the end of the trace, the Trace
                                                         ///  aggregator will resume the frame vector in a final trace value.

    mutable TT                     traceValue;   ///< The final trace value.
    
    BT                             neutralValue; ///< Needed to make template-based data collectors.

    /**
     * Must be called in each subclass. To be under the "protected:" section prevents from direct instantiation
     * of this class.
     */
    TraceDataCollector(bool deprecatedAggregation = true);

    /**
     * Must be called in each subclass constructor to initialize the aggregators
     */
    void setAggregators( const Aggregator<BT,FT>* _frameAggregator, const Aggregator<FT,TT>* _traceAggregator );

    bool deprecatedAggregation; ///< Tells if frame/trace values will be computed when getting values or at frame change/trace end events.
    
    mutable bool frameAggregationComputed;  ///< Tells if deprecated frame aggregation has been alreadly computed.
    mutable bool traceAggregationComputed;  ///< Tells if deprecated trace aggregation has been alreadly computed.
    
public:

    /**
     * Destructor. It deletes Aggregator classes allocated dynamically.
     */
    virtual ~TraceDataCollector();

    /**
     * This method sets the batch value and marks the end of the batch. 
     *
     */
    void endBatch(BT value);

    /**
     * This method marks the end of a frame. 
     *
     * @note It calls the frame aggregator to resume the frame value based on the batch values stored for
     *       the frame up to now.
     *
     */
    void endFrame();

    /**
     * This method marks the end of the trace.
     *
     * @note It calls the trace aggregator to compute the final trace value over the entire frame vector
     *       and stores it in the trace value.
     *
     */
    void endTrace();

    /**
     * Methods used by GLIStatsManager to query statistics results
     *
     */
    BT getBatchValue(unsigned int frameIndex, unsigned int batchIndex) const;
    FT getFrameValue(unsigned int frameIndex) const;

	std::vector< BT > getFrameBatchesValues(unsigned int frameIndex) const;

    TT getTraceValue() const;

    unsigned int getFrameCount() const;

};

/****************************************************************************************
 *                      Implemented Aggregator subclasses                               *
 ****************************************************************************************
 */

/**
 *  Count Aggregator
 *
 *  Returns the number of elements of the aggregated data
 */
template<class T>
class CountAggregator: public Aggregator<T,unsigned int>
{
public:
    
    virtual unsigned int getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  Different Count Aggregator
 *
 *  Returns the number of different elements of the aggregated data. 
 */
template<class T>
class DifferentCountAggregator: public Aggregator<T, unsigned int>
{
public:
    
    virtual unsigned int getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  Sum Aggregator
 *
 *  Returns the addition of all the elements of the aggregated data.
 */
template<class T>
class SumAggregator: public Aggregator<T, T>
{
public:
    
    virtual T getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  Max Aggregator
 *
 *  Returns the maximum value of the aggregated data.
 */
template<class T>
class MaxAggregator: public Aggregator<T,T>
{
public:
    
    virtual T getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  Min Aggregator
 *
 *  Returns the minimum value of the aggregated data.
 */
template<class T>
class MinAggregator: public Aggregator<T,T>
{
public:
    
    virtual T getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  Average Aggregator
 *
 *  Returns the average float value of the aggregated data values.
 */
template<class T>
class AverageAggregator: public Aggregator<T,float>
{
public:

    virtual float getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  Weighted Average Aggregator
 *
 *  Returns the weighted average float value of the aggregated data values.
 */
template<class T>
class WeightedAverageAggregator: public Aggregator<T,float>
{
public:

    virtual float getAggregatedValue(const std::vector<std::pair<T,unsigned int> >& values) const;

};

/**
 *  Predicated Average Aggregator
 *
 *  Returns the average float value of all the elements of the aggregated data different from
 *  the concrete value.
 */
template<class T>
class PredicatedAverageAggregator: public Aggregator<T,float>
{
private:
    
    T targetValue;

public:

    PredicatedAverageAggregator(T targetValue): targetValue(targetValue) {};

    virtual float getAggregatedValue(const std::vector<T>& values) const;

};

/**
 *  List Different Count Aggregator
 *
 *  Returns the count of all the different elements in the aggregated data sets.
 */
template<class T>
class ListDifferentCountAggregator: public Aggregator<std::set<T>, unsigned int>
{
public:
    
    virtual unsigned int getAggregatedValue(const std::vector<std::set<T> >& values) const;

};

/**
 *  List Different Aggregator
 *
 *  Returns a new set with unique elements of the vector sets.
 */
template<class T>
class ListDifferentAggregator: public Aggregator<std::set<T>, std::set<T> >
{
public:
    
    virtual std::set<T> getAggregatedValue(const std::vector<std::set<T> >& values) const;

};

/**
 *  Frequency Aggregator
 *
 *  Returns the percent of time that a concrete value appears in the data set.
 */
template<class T>
class FrequencyAggregator: public Aggregator<T,float>
{
private:

    T concreteValue;

public:

    FrequencyAggregator(T concreteValue): concreteValue(concreteValue) {};

    virtual float getAggregatedValue(const std::vector<T>& values) const;
};

/********************************************************************************************
 *          TraceDataCollector concrete subclasses used by the user statistics              *
 ********************************************************************************************
 */

template<class T>
class SumPerFrameSumTotal: public TraceDataCollector<T,T,T>
{
public:

    SumPerFrameSumTotal()
    : TraceDataCollector<T,T,T>()
    {
        setAggregators(new SumAggregator<T>, new SumAggregator<T>);
    }
};

template<class T>
class SumPerFrameAverageTotal: public TraceDataCollector<T,T,float>
{
public:

    SumPerFrameAverageTotal()
    : TraceDataCollector<T,T,float>()
    {
        setAggregators(new SumAggregator<T>, new AverageAggregator<T>);
    }
};

template<class T>
class AveragePerFrameAverageTotal: public TraceDataCollector<T,float,float>
{
public:

    AveragePerFrameAverageTotal()
    : TraceDataCollector<T,float,float>()
    {
        setAggregators(new AverageAggregator<T>, new AverageAggregator<float>);
    }
};

template<class T>
class PredicatedAveragePerFrameAverageTotal: public TraceDataCollector<T,float,float>
{
public:

    PredicatedAveragePerFrameAverageTotal(T targetValue)
    : TraceDataCollector<T,float,float>()
    {
        setAggregators(new PredicatedAverageAggregator<T>(targetValue), new AverageAggregator<float>);
    }
};

template<class T>
class MaxPerFrameMaxTotal: public TraceDataCollector<T,T,T>
{
public:

    MaxPerFrameMaxTotal()
    : TraceDataCollector<T,T,T>()
    {
        setAggregators(new MaxAggregator<T>, new MaxAggregator<T>);
    }
};

template<class T>
class DifferentCountPerFrameAverageTotal: public TraceDataCollector<T,unsigned int,float>
{
public:

    DifferentCountPerFrameAverageTotal()
    : TraceDataCollector<T,unsigned int, float>()
    {
        setAggregators(new DifferentCountAggregator<T>, new AverageAggregator<unsigned int>);
    }
};

template<class T>
class DifferentCountPerFrameSumTotal: public TraceDataCollector<T,unsigned int,unsigned int>
{
public:

    DifferentCountPerFrameSumTotal()
    : TraceDataCollector<T,unsigned int, unsigned int>()
    {
        setAggregators(new DifferentCountAggregator<T>, new SumAggregator<unsigned int>);
    }
};

template<class T>
class ListDifferentCountPerFrameAverageTotal: public TraceDataCollector<std::set<T>,unsigned int,float>
{
public:

    ListDifferentCountPerFrameAverageTotal()
    : TraceDataCollector<std::set<T>,unsigned int, float>()
    {
        setAggregators(new ListDifferentCountAggregator<T>, new AverageAggregator<unsigned int>);
    }
};

template<class T>
class ListDifferentPerFrameListDifferentTotal: public TraceDataCollector<std::set<T>, std::set<T>, std::set<T> >
{
public:
    ListDifferentPerFrameListDifferentTotal()
    : TraceDataCollector<std::set<T>, std::set<T>, std::set<T> >()
    {
        setAggregators(new ListDifferentAggregator<T>, new ListDifferentAggregator<T>);
    }
};

template<class T>
class FrequencyPerFrameAverageTotal: public TraceDataCollector<T,float,float>
{
public:

    FrequencyPerFrameAverageTotal(T concreteValue)
    : TraceDataCollector<T,float,float>()
    {
        setAggregators(new FrequencyAggregator<T>(concreteValue), new AverageAggregator<float>);
    }
};

/****************************************************************************************
 *  TraceDataCollector implementation:  Because of the template based implementation,   *
 *  this part is required to be in the header file.                                     *
 ****************************************************************************************
 */

template<class BT, class FT, class TT>
TraceDataCollector<BT,FT,TT>::TraceDataCollector(bool deprecatedAggregation)
: batchValues(0), frameValues(0), frameAggregator(0), traceAggregator(0), 
  deprecatedAggregation(deprecatedAggregation), frameAggregationComputed(false), 
  traceAggregationComputed(false)
{
    batchValues.push_back(std::vector<BT>(0));
};

template<class BT, class FT, class TT>
TraceDataCollector<BT,FT,TT>::~TraceDataCollector()
{
    if (frameAggregator) delete frameAggregator;
    if (traceAggregator) delete traceAggregator;
};

template<class BT, class FT, class TT>
void TraceDataCollector<BT,FT,TT>::setAggregators( const Aggregator<BT,FT>* _frameAggregator, 
                                                   const Aggregator<FT,TT>* _traceAggregator )
{
    frameAggregator = _frameAggregator;
    traceAggregator = _traceAggregator;
};

template<class BT, class FT, class TT>
void TraceDataCollector<BT,FT,TT>::endBatch(BT value)
{
    std::vector<BT>& batchValuesVector = batchValues.back();

    batchValuesVector.push_back(value);
};

template<class BT, class FT, class TT>
void TraceDataCollector<BT,FT,TT>::endFrame()
{
    if (!deprecatedAggregation)
    {
        const std::vector<BT>& currentValuesVector = batchValues.back();
        
        if (!frameAggregator)
            panic("workloadStats::TraceDataCollector","endFrame","Frame Aggregator not initialized");
    
        // Call frameAggregator to compute frame value.
        FT frameResumeValue = frameAggregator->getAggregatedValue(currentValuesVector);
    
        frameValues.push_back(frameResumeValue);
    };
    // Reserve a new batch vector for the following frame.
    batchValues.push_back(std::vector<BT>(0));
};

template<class BT, class FT, class TT>
void TraceDataCollector<BT,FT,TT>::endTrace()
{
    if (!deprecatedAggregation)
    {
        if (!traceAggregator)
            panic("workloadStats::TraceDataCollector","endTrace","Trace Aggregator not initialized");

        traceValue = traceAggregator->getAggregatedValue(frameValues);
    }
};

template<class BT, class FT, class TT>
BT TraceDataCollector<BT,FT,TT>::getBatchValue(unsigned int frameIndex, unsigned int batchIndex) const
{
    if (frameIndex >= batchValues.size()-1)
        panic("workloadStats::TraceDataCollector","getBatchValue","frame index out of bounds");

    if (batchIndex >= batchValues[frameIndex].size())
        panic("workloadStats::TraceDataCollector","getBatchValue","batch index out of bounds");

    return batchValues[frameIndex][batchIndex];
};

template<class BT, class FT, class TT>
FT TraceDataCollector<BT,FT,TT>::getFrameValue(unsigned int frameIndex) const
{
    if (deprecatedAggregation && !frameAggregationComputed)
    {
        typename std::vector<std::vector<BT> >::const_iterator iter = batchValues.begin();
        
        while ( iter != batchValues.end() )
        {
            if ( (iter + 1) != batchValues.end() ) // Skip last vector of batches (added automatically at endFrame)
            {
                if (!frameAggregator)
                    panic("workloadStats::TraceDataCollector","getFrameValue","Frame Aggregator not initialized");
                
                // Call frameAggregator to compute frame value.
                FT frameResumeValue = frameAggregator->getAggregatedValue((*iter));
                
                frameValues.push_back(frameResumeValue);
            }            
            iter++;
        }
        
        frameAggregationComputed = true;
    }
    
    if (frameIndex >= frameValues.size())
        panic("workloadStats::TraceDataCollector","getFrameValue","frame index out of bounds");

    return frameValues[frameIndex];
};

template<class BT, class FT, class TT>
TT TraceDataCollector<BT,FT,TT>::getTraceValue() const
{
    if (deprecatedAggregation && !traceAggregationComputed)
    {
        if (!traceAggregator)
            panic("workloadStats::TraceDataCollector","getTraceValue","Trace Aggregator not initialized");

        traceValue = traceAggregator->getAggregatedValue(frameValues);
        
        traceAggregationComputed = true;
    }
    
    return traceValue;
};

template<class BT, class FT, class TT>
std::vector< BT > TraceDataCollector<BT,FT,TT>::getFrameBatchesValues(unsigned int frameIndex) const
{
    if (frameIndex >= batchValues.size())
        panic("workloadStats::TraceDataCollector","getFrameBatchesValues","frame index out of bounds");
	
	return batchValues.at(frameIndex);
}


template<class BT, class FT, class TT>
unsigned int TraceDataCollector<BT,FT,TT>::getFrameCount() const
{
    if (deprecatedAggregation && !frameAggregationComputed)
    {
        typename std::vector<std::vector<BT> >::const_iterator iter = batchValues.begin();
        
        while ( iter != batchValues.end() )
        {
            if ( (iter + 1) != batchValues.end() ) // Skip last vector of batches (added automatically at endFrame)
            {
                if (!frameAggregator)
                    panic("workloadStats::TraceDataCollector","getFrameValue","Frame Aggregator not initialized");
                
                // Call frameAggregator to compute frame value.
                FT frameResumeValue = frameAggregator->getAggregatedValue((*iter));
                
                frameValues.push_back(frameResumeValue);
            }            
            iter++;
        }
        
        frameAggregationComputed = true;
    }
    return frameValues.size();
};

/**********************************************************************************************
 *  Aggregator subclasses methods implementation: Wherever is possible, the C++ STL algorithm *
                                                  functions are used.                         *
 **********************************************************************************************
 */
template<class T>
unsigned int CountAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    return values.size();
};

template<class T>
unsigned int DifferentCountAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    typename std::vector<T>::const_iterator iter = values.begin();

    std::set<T> differentList;

    while ( iter != values.end() )
    {
        differentList.insert((*iter));          
        iter++;
    }

    return differentList.size();
};

template<class T>
T SumAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    return std::accumulate(values.begin(), values.end(),  T(0));
};

template<class T>
T MaxAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    typename std::vector<T>::const_iterator maxValue = std::max_element(values.begin(),values.end());
    return (*maxValue);
};

template<class T>
T MinAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    typename std::vector<T>::const_iterator minValue = std::min_element(values.begin(),values.end());
    return (*minValue);
};

template<class T>
float AverageAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    T accum = std::accumulate(values.begin(), values.end(), T(0));
    
    if (values.size() != 0)
    {
        return ((float) accum / (float) values.size());
    }
    else
        return (float)accum;
};

template<class T>
float WeightedAverageAggregator<T>::getAggregatedValue(const std::vector<std::pair<T,unsigned int> >& values) const
{
    std::vector<T> values2;
    std::vector<unsigned int> weights;
    typename std::vector<std::pair<T, unsigned int> >::const_iterator iter = values.begin();
    
    while ( iter != values.end() )
    {
        values2.push_back(iter->first);
        weights.push_back(iter->second);
        iter++;
    };
    
    T accum = inner_product( values2.begin(), values2.end(), weights.begin(), T(0) );

    unsigned int weightsAccum = std::accumulate(weights.begin(), weights.end(), T(0));
    
    if (values2.size() != 0)
    {
        return ((float) accum / (float) weightsAccum);
    }
    else
        return (float)accum;
    
};

template<class T>
struct pred_adder: public std::binary_function<T, T, T>
{
    T value;

    pred_adder(T value): value(value) {};

    T operator()(T x, T y) { if (y != value) return (x + y); else return x; }
};

template<class T>
float PredicatedAverageAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{   
    workloadStats::pred_adder<T> plus(targetValue);

    T accum = std::accumulate(values.begin(), values.end(), T(0), plus);

    unsigned int num_items = count_if( values.begin(), values.end(), bind2nd(not_equal_to<T>(), targetValue));
    if (num_items != 0)
        return (float) accum / (float) num_items;
    else
        return (float)accum;
};

template<class T>
unsigned int ListDifferentCountAggregator<T>::getAggregatedValue(const std::vector<std::set<T> >& values) const
{
    typename std::vector<std::set<T> >::const_iterator iter = values.begin();

    std::set<T> differentList;

    while ( iter != values.end() )
    {
        typename std::set<T>::const_iterator iter2 = iter->begin();

        while ( iter2 != iter->end() )
        {
            differentList.insert((*iter2));     
            iter2++;
        }
        iter++;
    }

    return differentList.size();
};

template<class T>
std::set<T> ListDifferentAggregator<T>::getAggregatedValue(const std::vector<std::set<T> >& values) const
{
    typename std::vector<std::set<T> >::const_iterator iter = values.begin();

    std::set<T> differentList;

    while ( iter != values.end() )
    {
        typename std::set<T>::const_iterator iter2 = iter->begin();

        while ( iter2 != iter->end() )
        {
            differentList.insert((*iter2));     
            iter2++;
        }
        iter++;
    }

    return differentList;
};

template<class T>
float FrequencyAggregator<T>::getAggregatedValue(const std::vector<T>& values) const
{
    unsigned int times = std::count(values.begin(), values.end(), concreteValue);
    return (float)times / values.size();
};

/*************************************************************
 *                  Utility functions                        *
 *************************************************************/

void computeRangeValueReuseMatrix(const std::vector<std::set<unsigned int> >& diffTextsPerFrame, float* reuseMat);

void computeValueSimilarityMatrix(const std::vector<std::set<unsigned int> >& diffTextsPerFrame, float* reuseMat);

void computeValueReuseForAColumn(const std::vector<std::set<unsigned int> >& diffTextsPerFrame, unsigned int initFrame, 
                                    unsigned int finFrame, unsigned int matDim, float* reuseMat, 
                                    std::set<unsigned int>& reusedVals, std::set<unsigned int>& diffVals);

void writeSquareMatrixToPPM(const char *fname, const float* matrix, unsigned int matrixDimension);

void computeAverageReusePerWindowSize(std::vector<float>& reuseStripChart, const float* matrix, unsigned int matrixDimension);

} // namespace workloadStats

#endif // STATS_UTILS_H
