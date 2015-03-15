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
 * $RCSfile: QuadInt.h,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:56 $
 *
 * Quad Int class definition file. 
 *
 */


#ifndef __QUADINT__
    #define __QUADINT__

#include "GPUTypes.h"
#include <ostream>

namespace gpu3d
{

//! Class QuadInt defines a 4-component 32 bit integer vector
class QuadInt {

private:
    
    s32bit component[4]; //!< Data

public:
    
    //! Constructor
    QuadInt( s32bit x = 0, s32bit y = 0, s32bit z = 0, s32bit w = 0 );

    //!Allows indexed access & LH position ( implements set & get ) to individual components
    s32bit& operator[]( u32bit index );
    
    /*!
     * Modifies ALL components with new values, components without values are set to 0
     * i.e: qi.setComponents( 1, 3 );
     * The QuadInt resultant is ( 1, 3, 0, 0 )
     */
    void setComponents( s32bit x = 0, s32bit y = 0, s32bit z = 0, s32bit w = 0 );

    /*!
     * Gets all the QuadInt components
     */
    void getComponents( s32bit& x, s32bit& y, s32bit& z, s32bit& w );


    /**
     *  Get the pointer to the s32bit vector for QuadInt.    
     *
     */
     
    s32bit *getVector();
    
    /**
     *
     *  Assignment operator.  Copy values from an array of integer values.
     *
     */
     
    QuadInt& operator=(s32bit *source);
         
     
    //! Must be declared and defined inside the class
    friend std::ostream& operator<<( std::ostream& theStream, const QuadInt& qi ) {
            
        theStream << "(" << qi.component[0] << "," << qi.component[1] << "," 
            << qi.component[2] << "," << qi.component[3] << ")";
    
        return theStream;
    }

};

} // namespace gpu3d

#endif
