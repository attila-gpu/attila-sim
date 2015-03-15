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
 * $RCSfile: BlendOperation.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:26 $
 *
 * Blend Operation implementation file.
 *
 */

/**
 *
 *  @file BlendOperation.cpp
 *
 *  This file implements the Blend Operation class.
 *
 *  This class implements the blend operations issued to the
 *  blend unit in the ColorWrite box.
 *
 */

#include "BlendOperation.h"

using namespace gpu3d;


/*  Blend Operation constructor.  */
BlendOperation::BlendOperation(u32bit opID)
{
    /*  Set operation parameters.  */
    id = opID;

    /*  Set object color.  */
    setColor(0);

    setTag("BlOp");
}

/*  Return blend operation id.  */
u32bit BlendOperation::getID()
{
    return id;
}


