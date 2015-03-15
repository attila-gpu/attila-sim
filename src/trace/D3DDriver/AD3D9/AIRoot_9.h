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

#ifndef AIROOT_9_H
#define AIROOT_9_H

#include "d3d9_port.h"
#include <set>

class AIDirect3DImp9;

/**
    Root interface for D3D9:
        Owner of IDirect3DImp9 objects.
        Implements some D3D9 functions.
*/
class AIRoot9 {
public:
    AIRoot9();
    ~AIRoot9();

    IDirect3D9* Direct3DCreate9(UINT SDKVersion);
private:
    std::set<AIDirect3DImp9*> i_childs;
};

#endif

