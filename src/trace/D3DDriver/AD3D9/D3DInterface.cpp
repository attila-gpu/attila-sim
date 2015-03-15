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

#include "Common.h"
#include "D3DInterface.h"
#include "AD3D9State.h"

AIRoot9* D3DInterface::ai_root_9 = NULL;
D3DTrace* D3DInterface::trace = NULL;

void D3DInterface::initialize(D3DTrace *d3dTrace)
{
    D3D_DEBUG( cout << "D3DInterface: Initializing" << endl; )

    ai_root_9 = new AIRoot9();

    trace = d3dTrace;
    
    AD3D9State::instance().setD3DTrace(trace);
}

void D3DInterface::finalize()
{
    D3D_DEBUG( cout << "D3DInterface: Finalizing" << endl; )

    delete ai_root_9;
}

AIRoot9* D3DInterface::get_acd_root_9()
{
    return ai_root_9;
}

D3DInterface::D3DInterface() {}

