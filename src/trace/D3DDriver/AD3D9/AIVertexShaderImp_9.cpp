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
#include "AIVertexShaderImp_9.h"

#include "AD3D9State.h"

#include <cstring>

AIVertexShaderImp9::AIVertexShaderImp9(AIDeviceImp9* _i_parent, CONST DWORD* pFunction):
i_parent(_i_parent) 
{

    //AD3D9State::instance().addVertexShader(this, pFunction);
    
    UINT programLength;
    
    //  Build the shader intermediate representation and get the shader length.
    programLength = ShaderTranslator::get_instance().buildIR(pFunction, programIR);
    
    //  Allocate an array to store the program.
    program = new DWORD[programLength];

    // Copy the content of the program
    memcpy(program, pFunction, programLength * sizeof(DWORD));

    acdVertexShader = NULL;
    nativeVertexShader = NULL;

    refs = 0;
}


AIVertexShaderImp9::AIVertexShaderImp9() 
{
	///@note Used to differentiate singleton cover
	i_parent = 0;
}

AIVertexShaderImp9& AIVertexShaderImp9::getInstance() 
{
    static AIVertexShaderImp9 instance;
    return instance;
}

HRESULT D3D_CALL AIVertexShaderImp9::QueryInterface(REFIID riid , void** ppvObj) 
{
    D3D9_CALL(false, "AIVertexShaderImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIVertexShaderImp9::AddRef() 
{
    D3D9_CALL(true, "AIVertexShaderImp9::AddRef")
	if(i_parent != 0) {
		refs ++;
    	return refs;
	}
	else return 0;
}

ULONG D3D_CALL AIVertexShaderImp9::Release() 
{
    D3D9_CALL(true, "AIVertexShaderImp9::Release")
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

HRESULT D3D_CALL AIVertexShaderImp9::GetDevice(IDirect3DDevice9** ppDevice) 
{
    D3D9_CALL(false, "AIVertexShaderImp9::GetDevice")
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL AIVertexShaderImp9::GetFunction(void* pData, UINT* pSizeOfData) 
{
    D3D9_CALL(false, "AIVertexShaderImp9::GetFunction")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}


acdlib::ACDShaderProgram* AIVertexShaderImp9::getAcdVertexShader() 
{

    if (acdVertexShader == NULL) {

        if (nativeVertexShader == NULL)
            nativeVertexShader = ShaderTranslator::get_instance().translate(programIR);

        acdVertexShader = AD3D9State::instance().createShaderProgram();
        
        acdVertexShader->setCode(nativeVertexShader->bytecode, nativeVertexShader->lenght);

    }
    
    return acdVertexShader;

}

NativeShader* AIVertexShaderImp9::getNativeVertexShader() 
{

    if (nativeVertexShader == NULL)
        nativeVertexShader = ShaderTranslator::get_instance().translate(programIR);
    
    return nativeVertexShader;

}
