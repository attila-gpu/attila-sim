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

#ifndef ACDX_FIXED_PIPELINE_CONSTANT_LIMITS_H
    #define ACDX_FIXED_PIPELINE_CONSTANT_LIMITS_H

namespace acdlib
{
/**
 *    The maximum number of lights supported by the implementation
 */
#define ACDX_FP_MAX_LIGHTS_LIMIT 8                
/**
 *    The maximum number of texture stages supported by the implementation
 */
#define ACDX_FP_MAX_TEXTURE_STAGES_LIMIT 16
/**
 *  The maximum number of modelview matrices supported by the implementation
 */
#define ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT 4

} // namespace acdlib

#endif // ACDX_FIXED_PIPELINE_CONSTANT_LIMITS_H
