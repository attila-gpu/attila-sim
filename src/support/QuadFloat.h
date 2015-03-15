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
 * $RCSfile: QuadFloat.h,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:55 $
 *
 * Quad Fload class definition file. 
 *
 */


#ifndef __QUAD_FLOAT__
    #define __QUAD_FLOAT__

#include "support.h"
#include "GPUTypes.h"
#include <ostream>

namespace gpu3d
{

//! Class QuadFloat defines a 4-component vector of floats
class QuadFloat {

private:
    
    f32bit component[4]; //!< Data

public:
    
    //! Constructor
    QuadFloat( f32bit x = 0, f32bit y = 0, f32bit z = 0, f32bit w = 0 );

    //!Allows indexed access & LH position ( implements set & get ) to individual components
    f32bit& operator[]( u32bit index );
    
    /*!
     * Modifies ALL components with new values, components without values are set to 0
     * i.e: qf.setComponents( 1.0, 3.0 );
     * The QuadFloat resultant is ( 1.0, 3.0, 0.0, 0.0 )
     */
    void setComponents( f32bit x = 0, f32bit y = 0, f32bit z = 0, f32bit w = 0 );

    /*!
     * Gets all the QuadFloat components
     */
    void getComponents( f32bit& x, f32bit& y, f32bit& z, f32bit& w );

    
    /**
     *  Returns the pointer to the vector of f32bit components
     *  for the QuadFloat.
     *
     */

    f32bit *getVector();

    /**
     *
     *  Assignment operator.  Copy from an array of float point values.
     *
     */
     
    QuadFloat& operator=(f32bit *source);
    
    //! Must be declared and defined inside the class
    friend std::ostream& operator<<( std::ostream& theStream, const QuadFloat& qf ) {
            
        theStream << "(" << qf.component[0] << "," << qf.component[1] << "," 
            << qf.component[2] << "," << qf.component[3] << ")";
    
        return theStream;
    }
    
};

} // namespace gpu3d

#endif
