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
#include "AIDeviceImp_9.h"
#include "AIPixelShaderImp_9.h"

#include "AD3D9State.h"

#include <cstring>

AIPixelShaderImp9::AIPixelShaderImp9(AIDeviceImp9* _i_parent, CONST DWORD* pFunction):
i_parent(_i_parent) 
{

    //AD3D9State::instance().addPixelShader(this, pFunction);

    UINT programLength;
    
    //  Build the shader intermediate representation and get the shader length.
    programLength = ShaderTranslator::get_instance().buildIR(pFunction, programIR);
    
    //  Allocate an array to store the program.
    program = new DWORD[programLength];

    // Copy the content of the program
    memcpy(program, pFunction, programLength * sizeof(DWORD));

    acdPixelShader = new acdlib::ACDShaderProgram* [8 * 2];
    for (UINT i = 0; i < (8 * 2); i++)
        acdPixelShader[i] = NULL;

    nativePixelShader = new NativeShader* [8 * 2];
    for (UINT i = 0; i < (8 * 2); i++)
        nativePixelShader[i] = NULL;

    refs = 0;
}


AIPixelShaderImp9::AIPixelShaderImp9() 
{
	///@note Used to differentiate when using as singleton cover
	i_parent = 0;
}

AIPixelShaderImp9& AIPixelShaderImp9::getInstance() 
{
    static AIPixelShaderImp9 instance;
    return instance;
}

HRESULT D3D_CALL AIPixelShaderImp9::QueryInterface(REFIID riid, void** ppvObj) 
{
    D3D9_CALL(false, "AIPixelShaderImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIPixelShaderImp9::AddRef() 
{
    D3D9_CALL(false, "AIPixelShaderImp9::AddRef")
	if(i_parent != 0) {
		refs ++;
    	return refs;
	}
	else return 0;
}

ULONG D3D_CALL AIPixelShaderImp9::Release() 
{
    D3D9_CALL(true, "AIPixelShaderImp9::Release")
    if(i_parent != 0) {
        refs--;
        if(refs == 0) {
            // Remove state
            /*StateDataNode* parent = state->get_parent();
            parent->remove_child(state);
            delete state;
            state = 0;*/
        }
        return refs;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

HRESULT D3D_CALL AIPixelShaderImp9::GetDevice(IDirect3DDevice9** ppDevice) 
{
    D3D9_CALL(true, "AIPixelShaderImp9::GetDevice")
    *ppDevice = i_parent;
    return D3D_OK;;
}

HRESULT D3D_CALL AIPixelShaderImp9::GetFunction(void* pData, UINT* pSizeOfData) 
{
    D3D9_CALL(false, "AIPixelShaderImp9::GetFunction")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

acdlib::ACDShaderProgram* AIPixelShaderImp9::getAcdPixelShader(D3DCMPFUNC alpha, bool fogEnable) 
{

    int i = (alpha - 1) * 2 + (fogEnable ? 1 : 0);

    if (acdPixelShader[i] == NULL)
    {
        if (nativePixelShader[i] == NULL)
            nativePixelShader[i] = ShaderTranslator::get_instance().translate(programIR, alpha, fogEnable);

        acdPixelShader[i] = AD3D9State::instance().createShaderProgram();

//printf("Native Shader :\n");
//printf("%s\n", nativePixelShader[i]->debug_assembler.c_str());
        
        acdPixelShader[i]->setCode(nativePixelShader[i]->bytecode, nativePixelShader[i]->lenght);
    }
    
    return acdPixelShader[i];

}

NativeShader* AIPixelShaderImp9::getNativePixelShader(D3DCMPFUNC alpha, bool fogEnable) 
{

    int i = (alpha - 1) * 2 + (fogEnable ? 1 : 0);

    if (nativePixelShader[i] == NULL)
        nativePixelShader[i] = ShaderTranslator::get_instance().translate(programIR, alpha, fogEnable);
        
    return nativePixelShader[i];

}
