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
 * $RCSfile: ROPStatusInfo.cpp,v $
 * $Revision: 1.1 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:46 $
 *
 * ROP Status Info implementation file.
 *
 */

/**
 *
 *  @file RPStatusInfo.cpp
 *
 *  This file implements the ROP Status Info class.
 *
 *  This class implements an object that carries state information
 *  from a Generic ROP box to a producer stage that generates fragments for the
 *  Generic ROP box.
 *
 */

#include "ROPStatusInfo.h"

using namespace gpu3d;

/*  ROP Status Info constructor.  */
ROPStatusInfo::ROPStatusInfo(ROPState stat)
{
    /*  Set carried state.  */
    state = stat;

    /*  Set object color.  */
    setColor(state);

    setTag("ZSTStIn");
}


/*  Get state info carried by the object.  */
ROPState ROPStatusInfo::getState()
{
    return state;
}
