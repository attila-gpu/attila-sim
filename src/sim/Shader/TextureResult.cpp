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
 * $RCSfile: TextureResult.cpp,v $
 * $Revision: 1.7 $
 * $Author: christip $
 * $Date: 2007-09-05 16:21:14 $
 *
 * Texture Result implementation file.
 *
 */


/**
 *
 *  @file TextureResult.cpp
 *
 *  This file implements the Texture Result class.
 *
 *  The Texture Result class carries texture sample results from the Texture Unit to
 *  the Shader Decode Execute Unit.
 *
 */

#include "TextureResult.h"

using namespace gpu3d;

/*  Texture Result constructor.  */
TextureResult::TextureResult(u32bit ident, QuadFloat *samples, u32bit stampFrags, u64bit cycle)
{
    u32bit i;

    /*  Set texture access identifier.  */
    id = ident;

    /*  Set texture access start cycle.  */
    startCycle = cycle;

    /*  Allocate space for the result.  */
    textSamples = new QuadFloat[stampFrags];

    /*  Copy result texture samples.  */
    for(i = 0; i < stampFrags; i++)
        textSamples[i] = samples[i];

    setTag("TexRes");
}

/*  Gets the texture access id for the result.  */
u32bit TextureResult::getTextAccessID()
{
    return id;
}

/*  Gets the texture samples.  */
QuadFloat *TextureResult::getTextSamples()
{
    return textSamples;
}

/*  Texture Result destructor.  */
TextureResult::~TextureResult()
{
    delete[] textSamples;
}

/*  Returns the texture access start cycle for the texture result.  */
u64bit TextureResult::getStartCycle()
{
        return startCycle;
}
