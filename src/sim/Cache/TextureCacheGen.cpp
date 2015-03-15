/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not b;e disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: TextureCacheGen.cpp,v $
 * $Revision: 1.1 $
 * $Author: vmoya $
 * $Date: 2006-05-11 18:10:40 $
 *
 * Texture Cache class implementation file.
 *
 */

/**
 *
 * @file TextureCacheGen.cpp
 *
 * Implements the Texture Cache class.  This class implements the cache used to
 * read texels from texture data.
 *
 */

#include "TextureCacheGen.h"
#include "GPUMath.h"

using namespace gpu3d;

/*  Texture cache constructor.  */
TextureCacheGen::TextureCacheGen()
{
}

