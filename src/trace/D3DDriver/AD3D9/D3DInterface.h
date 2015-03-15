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

#ifndef D3DINTERFACE_H_INCLUDED
#define D3DINTERFACE_H_INCLUDED

#include "AIRoot_9.h"

class D3DTrace;

class D3DInterface
{
public:

    static void initialize(D3DTrace *trace);
    static void finalize();
    static AIRoot9 *get_acd_root_9();
    static D3DTrace *getD3DTrace();
    
private:

    static AIRoot9 *ai_root_9;
    static D3DTrace *trace;
    
    D3DInterface();
};

#endif

