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

#ifndef AISTATEBLOCKIMP_9_H_INCLUDED
#define AISTATEBLOCKIMP_9_H_INCLUDED

class AIStateBlockImp9 : public IDirect3DStateBlock9
{

public:

    AIStateBlockImp9(AIDeviceImp9* parent, map<D3DRENDERSTATETYPE, DWORD> renderState);
    static AIStateBlockImp9 &getInstance();
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL Capture ( );
    HRESULT D3D_CALL Apply ( );

private:

    AIStateBlockImp9();

    AIDeviceImp9* parentDevice;
    
    ULONG referenceCount;

    map<D3DRENDERSTATETYPE, DWORD> blockRenderState;
};



#endif
