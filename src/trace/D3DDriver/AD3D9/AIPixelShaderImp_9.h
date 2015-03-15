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

#ifndef AIPIXELSHADERIMP_9_H
#define AIPIXELSHADERIMP_9_H

#include "ACDShaderProgram.h"
#include "ShaderTranslator.h"
struct NativeShader;

class AIPixelShaderImp9 : public IDirect3DPixelShader9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AIPixelShaderImp9 &getInstance();

    AIPixelShaderImp9(AIDeviceImp9* i_parent, CONST DWORD* pFunction);

private:
    /// Singleton constructor method
    AIPixelShaderImp9();

    AIDeviceImp9* i_parent;

    ULONG refs;

    IR* programIR;
    DWORD* program;

    acdlib::ACDShaderProgram** acdPixelShader;

    NativeShader** nativePixelShader;

public:
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL GetFunction (  void* pData , UINT* pSizeOfData );

    acdlib::ACDShaderProgram* getAcdPixelShader(D3DCMPFUNC alpha, bool fogEnable);
    NativeShader* getNativePixelShader(D3DCMPFUNC alpha, bool fogEnable);

};

#endif
