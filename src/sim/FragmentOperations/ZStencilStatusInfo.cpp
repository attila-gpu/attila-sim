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
 * $RCSfile: ZStencilStatusInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Z Stencil Status Info implementation file.
 *
 */

/**
 *
 *  @file ZStencilStatusInfo.cpp
 *
 *  This file implements the Z Stencil Status Info class.
 *
 *  This class implements an object that carries state information
 *  from Z Stencil Test box to Fragment FIFO (Interpolator) box.
 *
 */

#include "ZStencilStatusInfo.h"

using namespace gpu3d;

/*  Z Stencil Status Info constructor.  */
ZStencilStatusInfo::ZStencilStatusInfo(ZStencilTestState stat)
{
    /*  Set carried state.  */
    state = stat;

    /*  Set object color.  */
    setColor(state);

    setTag("ZSTStIn");
}


/*  Get state info carried by the object.  */
ZStencilTestState ZStencilStatusInfo::getState()
{
    return state;
}
