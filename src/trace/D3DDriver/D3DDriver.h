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

#ifndef D3D_DRIVER_H_INCLUDED
#define D3D_DRIVER_H_INCLUDED

class D3DTrace;

class D3DDriver 
{
public:

    static void initialize(D3DTrace *trace);
    static void finalize();

private:

    D3DDriver();
};

#endif


