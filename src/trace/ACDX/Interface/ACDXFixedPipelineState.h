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

#ifndef ACDX_FIXED_PIPELINE_STATE_H
    #define ACDX_FIXED_PIPELINE_STATE_H

#include <list>
#include <string>

#include "ACDXGlobalTypeDefinitions.h"

/* Include the interfaces of the Fixed Pipeline stages */
#include "ACDXTransformAndLightingStage.h"
#include "ACDXTextCoordGenerationStage.h"
#include "ACDXFragmentShadingStage.h"
#include "ACDXPostShadingStage.h"

#include "ACDXStoredFPState.h"
#include "ACDXStoredFPItemID.h"


namespace acdlib
{
/**
 * Stored FP item list definition
 */
typedef std::list<ACDX_STORED_FP_ITEM_ID> ACDXStoredFPItemIDList;

/**
 * The ACDXFixedPipelineState interface gives access
 * to the state related with the Fixed Pipeline.
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @version 0.8
 * @date 03/12/2007
 */
class ACDXFixedPipelineState
{
public:
    
    /**
     * Gets an interface to the transform & lighting stages.
     */
    virtual ACDXTransformAndLightingStage& tl() = 0;

    /**
     * Gets an interface to the texture coordinate generation stage.
     */
    virtual ACDXTextCoordGenerationStage& txtcoord() = 0;

    /**
     * Gets an interface to the fragment shading and texturing stages.
     */
    virtual ACDXFragmentShadingStage& fshade() = 0;

    /**
     * Gets an interface to the post fragment shading stages (FOG and Alpha).
     */
    virtual ACDXPostShadingStage& postshade() = 0;

    /**
     * Save a single Fixed Pipeline state
     *
     * @param    stateId the identifier of the state to save
     * @returns The group of states saved
     */
    virtual ACDXStoredFPState* saveState(ACDX_STORED_FP_ITEM_ID stateId) const = 0;

    /**
     * Save a group of Fixed Pipeline states
     *
     * @param siIds List of state identifiers to save
     * @returns The group of states saved
     */
    virtual ACDXStoredFPState* saveState(ACDXStoredFPItemIDList siIds) const = 0;

    /**
     * Save the whole set of Fixed Pipeline states
     *
     * @returns The whole states group saved
     */
    virtual ACDXStoredFPState* saveAllState() const = 0;

    /**
     * Restores the value of a group of Fixed Pipeline states
     *
     * @param state a previously saved state group to be restored
     */
    virtual void restoreState(const ACDXStoredFPState* state) = 0;

    /**
     * Releases a ACDXStoredFPState interface object
     *
     * @param state The saved state object to be released
     */
    virtual void destroyState(ACDXStoredFPState* state) const = 0;

    //////////////////////////////////////////
    //      Debug/Persistence methods       //
    //////////////////////////////////////////

    /**
     * Dumps the ACDXFixedPipelineState state
     */
    virtual void DBG_dump(const acd_char* file, acd_enum flags) const = 0;

    /**
     * Saves a file with an image of the current ACDXFixedPipelineState state
     */
    virtual acd_bool DBG_save(const acd_char* file) const = 0;

    /**
     * Restores the ACDXFixedPipelineState state from a image file previously saved
     */
    virtual acd_bool DBG_load(const acd_char* file) = 0;

};

} // namespace acdlib

#endif // ACDX_FIXED_PIPELINE_STATE_H
