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
 * $RCSfile: ROPOperation.cpp,v $
 * $Revision: 1.1 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:46 $
 *
 * ROP Operation implementation file.
 *
 */

/**
 *
 *  @file ROPOperation.cpp
 *
 *  This file implements the ROP Operation class.
 *
 *  This class implements the ROP operations started in a Generic ROP box
 *  that are sent through an operation signal simulating the latency of
 *  the ROP operation unit.
 *
 */

#include "ROPOperation.h"

using namespace gpu3d;

/*  ROP Operation constructor.  */
ROPOperation::ROPOperation(ROPQueue *opStamp)
{
    /*  Set operation parameters.  */
    operatedStamp = opStamp;

    /*  Set object color.  */
    setColor(0);

    setTag("ROPOp");
}

/*  Return Z operation id.  */
ROPQueue *ROPOperation::getROPStamp()
{
    return operatedStamp;
}


