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
 * $RCSfile: ZOperation.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Z Operation implementation file.
 *
 */

/**
 *
 *  @file ZOperation.cpp
 *
 *  This file implements the Z Operation class.
 *
 *  This class implements the Z and stencil tests issued to the
 *  test unit in the Z Stencil Test box.
 *
 */

#include "ZOperation.h"

using namespace gpu3d;

/*  Z Operation constructor.  */
ZOperation::ZOperation(u32bit opID)
{
    /*  Set operation parameters.  */
    id = opID;

    /*  Set object color.  */
    setColor(0);

    setTag("ZOp");
}

/*  Return Z operation id.  */
u32bit ZOperation::getID()
{
    return id;
}


