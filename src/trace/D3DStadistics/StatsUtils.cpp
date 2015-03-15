#include "stdafx.h"

#ifdef WORKLOAD_STATS

#include "StatsUtils.h"
#include <fstream>

using namespace std;
using namespace workloadStats;

/***********************************************************************************
 *                       Utility Functions Implementation                          *
 ***********************************************************************************/

void workloadStats::writeSquareMatrixToPPM(const char *fname, const float* matrix, unsigned int matrixDimension)
{
    ofstream imgFile;

    imgFile.open (fname, ofstream::out | ofstream::trunc);

    imgFile << "P3 " << endl;
    imgFile << "# Created by GLIStat " << endl;
    
    // Mida de la imatge

    imgFile << matrixDimension << " " << matrixDimension << endl;
    
    // Profunditat del color

    imgFile << "255" << endl; // max. component

    int R, G, B;

    int w = matrixDimension;

    for ( int i = 0 ; i < matrixDimension; i++ )
    {
        for ( int j = 0; j < matrixDimension; j++ )
        {
            R = (int) (255.0*matrix[((i*w)+j)]);
            G = (int) (255.0*matrix[((i*w)+j)]);
            B = (int) (255.0*matrix[((i*w)+j)]);

            imgFile << R << " " << G << " " << B << " ";
        }
    }

    imgFile.close();
};


void workloadStats::computeAverageReusePerWindowSize(vector<float>& reuseStripChart, const float* matrix, 
                                                unsigned int matrixDimension)
{
    /*
     * The algorithm traverses and makes the average of
     * the matrix diagonals values. For example, 
     * for a 4x4 matrix the diagonals are:
     *
     *      M = a[i,j]  i=0..3, j=0..3
     *
     *      0,0   1,1   2,2   3,3 <- window size = 1
     *      0,1   1,2   2,3       <- window size = 2
     *      0,2   1,3             <- window size = 3
     *      0,3                   <- window size = 4
     *
     */

    for(int i = 0; i < matrixDimension; i++)
    {
        float percent = 0.0f;

        for(int j = 0; j < (matrixDimension - i); j++)
        {   
            percent += matrix[j*matrixDimension + j + i];
        }
        
        percent = percent / (float)(matrixDimension - i);

        reuseStripChart.push_back(percent);
    }

};

/* Recursive function used by the function above to compute a single column of the matrix. */

void workloadStats::computeValueReuseForAColumn(const vector<set<unsigned int> >& diffTextsPerFrame, 
                                           unsigned int initFrame, unsigned int finFrame, unsigned int matDim, 
                                           float* reuseMat, set<unsigned int>& reusedVals, 
                                           set<unsigned int>& diffVals)
{
    set<unsigned int> dS = diffTextsPerFrame[initFrame];

    if (initFrame == finFrame) // Base case of recursion
    {
        reuseMat[initFrame * matDim + finFrame] = 1.0f;
        reusedVals = dS;
        diffVals = dS;
    }
    else // General case of recursion
    {
        workloadStats::computeValueReuseForAColumn(diffTextsPerFrame, initFrame + 1, finFrame, matDim, reuseMat, reusedVals, diffVals);
        
        set<unsigned int> newReusedVals;

        insert_iterator<set<unsigned int> > resIter(newReusedVals, newReusedVals.begin());

        set_intersection(reusedVals.begin(),reusedVals.end(),dS.begin(),dS.end(),resIter);

        set<unsigned int>::const_iterator dSiter = dS.begin();

        while (dSiter != dS.end())
        {
            diffVals.insert((*dSiter));
            dSiter++;
        }

        reuseMat[initFrame * matDim + finFrame] = (float)newReusedVals.size()/(float)diffVals.size();

        reusedVals = newReusedVals;
    }
};

void workloadStats::computeValueSimilarityMatrix(const vector<set<unsigned int> >& diffTextsPerFrame, 
                                            float* reuseMat)
{
	// TEST
	ofstream *log = new ofstream("stats.log");
	*log << "Computing similarity matrix" << endl;
	log->flush();

	int i, j;

    /* Compute the matrix dimension */
    unsigned int matDim = diffTextsPerFrame.size();

    /* Initialize matrix with zeroes */

    for(i=0; i < matDim * matDim; i++)
        reuseMat[i] = 0.0f;

    set<unsigned int> diffValsI;
    set<unsigned int> diffValsJ;

    /* Iterate over the matrix columns */
    for(i = 0; i < matDim; i++)
    {
        diffValsI = diffTextsPerFrame[i];
        
        for ( j = 0; j <= i; j ++)
        {
            diffValsJ = diffTextsPerFrame[j];
            set<unsigned int> intersecSet;
            set<unsigned int> unionSet;
            insert_iterator<set<unsigned int> > resIter(intersecSet, intersecSet.begin());
            insert_iterator<set<unsigned int> > resIter2(unionSet, unionSet.begin());
            set_intersection(diffValsI.begin(),diffValsI.end(),diffValsJ.begin(),diffValsJ.end(),resIter);
            set_union(diffValsI.begin(),diffValsI.end(),diffValsJ.begin(),diffValsJ.end(),resIter2);
            reuseMat[j * matDim + i] = (float)intersecSet.size()/(float)unionSet.size();
        }
        
		// TEST
		*log << "Completed frame " << i << " of " << matDim << endl;
		log->flush();
    }

	// TEST
	log->close();
};

void workloadStats::computeRangeValueReuseMatrix(const vector<set<unsigned int> >& diffTextsPerFrame, 
                                            float* reuseMat)
{
    int i;

    /* Compute the matrix dimension */
    unsigned int matDim = diffTextsPerFrame.size();

    /* Initialize matrix with zeroes */

    for(i=0; i < matDim * matDim; i++)
        reuseMat[i] = 0.0f;

    /* Set up initial sets for the recursive function */
    set<unsigned int> reusedVals;
    set<unsigned int> diffVals;

    /* Iterate over the matrix columns */
    for(i = 0; i < matDim; i++)
        workloadStats::computeValueReuseForAColumn(diffTextsPerFrame, 0, i, matDim, reuseMat, reusedVals, diffVals);

};

#endif // WORKLOAD_STATS