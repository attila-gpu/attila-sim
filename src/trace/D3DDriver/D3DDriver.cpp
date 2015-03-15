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

#include "D3DDriver.h"
#include "D3DInterface.h"

void D3DDriver::initialize(D3DTrace *trace)
{
    D3DInterface::initialize(trace);
}

void D3DDriver::finalize()
{

    D3DInterface::finalize();
}

D3DDriver::D3DDriver()
{
}

