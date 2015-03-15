/**************************************************************************
 *
 * Copyright (c) 2002 - 2004 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ColorWriteStatusInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Color Write Status Info implementation file.
 *
 */

/**
 *
 *  @file ColorWriteStatusInfo.cpp
 *
 *  This file implements the Color Write Status Info class.
 *
 *  This class implements carries state information between Color Write
 *  and Z Test boxes.
 *
 */

#include "ColorWriteStatusInfo.h"

using namespace gpu3d;

/*  Color Write Status Info constructor.  */
ColorWriteStatusInfo::ColorWriteStatusInfo(ColorWriteState stat)
{
    /*  Set carried state.  */
    state = stat;

    /*  Set object color.  */
    setColor(state);

    setTag("CWStIn");
}


/*  Get state info carried by the object.  */
ColorWriteState ColorWriteStatusInfo::getState()
{
    return state;
}
