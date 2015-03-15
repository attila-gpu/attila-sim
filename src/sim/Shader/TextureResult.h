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
 * $RCSfile: TextureResult.h,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2006-01-31 12:54:40 $
 *
 * Texture Result definition file.
 *
 */

/**
 *
 *  @file TextureResult.h
 *
 *  Defines the Texture Result class.  This class carries texture access sample results from
 *  the Texture Unit to the Shader Decode Execute unit.
 *
 */

#include "GPUTypes.h"
#include "DynamicObject.h"

#ifndef _TEXTURERESULT_

#define _TEXTURERESULT_

namespace gpu3d
{

/**
 *
 *  Defines a texture access sample result from the Texture Unit to the Shader Decode Execute Unit.
 *
 *  This class inherits from the DynamicObject class which offers dynamic memory
 *  management and tracing support.
 *
 *
 */


class TextureResult : public DynamicObject
{

private:

    u32bit id;                  /**<  Texture Access identifier.  */
    QuadFloat *textSamples;     /**<  The result texture samples.  */
    u64bit startCycle;          /**<  Cycle at which the texture access was issued to the Texture Unit.  */

public:

    /**
     *
     *  Class constructor.
     *
     *  Creates and initializes a new Texture Result object.
     *
     *  @param id Identifier of the Texture Access.
     *  @param samples Pointer to a QuadFloat array with the result texture samples
     *  for the fragments in a stamp.
     *  @param cycle Cycle at which the texture access that produced the texture result was
     *  issued to the Texture Unit.
     *
     *  @return A new Texture Result object.
     *
     */

    TextureResult(u32bit id, QuadFloat *sample, u32bit stampFrags, u64bit cycle);

    /**
     *
     *  Class destructor.
     *
     */

    ~TextureResult();

    /**
     *
     *  Returns the texture access identifier.
     *
     *  @return The texture access id.
     *
     */

    u32bit getTextAccessID();

    /**
     *
     *  Returns the result texture samples.
     *
     *  @return A pointer to the texture samples for the fragments in a stamp.
     *
     */

    QuadFloat *getTextSamples();

    /**
     *
     *  Returns the cycle at which the texture access was started.
     *
     *  @return The cycle at which the texture access was issued to the Texture Unit.
     *
     */

    u64bit getStartCycle();


};

} // namespace gpu3d

#endif
