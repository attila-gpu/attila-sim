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
#include "AIVertexDeclarationImp_9.h"

#include "AD3D9State.h"

AIVertexDeclarationImp9::AIVertexDeclarationImp9(AIDeviceImp9* _i_parent, CONST D3DVERTEXELEMENT9* elements):
i_parent(_i_parent) 
{

    //AD3D9State::instance().addVertexDeclaration(this, elements);
    
    D3DVERTEXELEMENT9 end = D3DDECL_END();
    UINT numElements = 0;

    // Copy the vertex declaration to a more easy to use structure
    while(elements[numElements].Stream != end.Stream) {

        D3DVERTEXELEMENT9 tmpElem;

        tmpElem.Stream = elements[numElements].Stream;
        tmpElem.Offset = elements[numElements].Offset;
        tmpElem.Type = elements[numElements].Type;
        tmpElem.Method = elements[numElements].Method;
        tmpElem.Usage = elements[numElements].Usage;
        tmpElem.UsageIndex = elements[numElements].UsageIndex;

        vElements.push_back(tmpElem);
        numElements++;

    }

    refs = 0;

}

AIVertexDeclarationImp9::AIVertexDeclarationImp9() {}

AIVertexDeclarationImp9& AIVertexDeclarationImp9::getInstance() 
{
    D3D9_CALL(true, "AIVertexDeclarationImp9::getInstance")
    static AIVertexDeclarationImp9 instance;
    return instance;
}

vector< D3DVERTEXELEMENT9 >& AIVertexDeclarationImp9::getVertexElements() 
{
    D3D9_CALL(true, "AIVertexDeclarationImp9::getVertexElements")
    return vElements;
}

/*UINT AIVertexDeclarationImp9::getNumElements() {
    return numElements;
}*/

HRESULT D3D_CALL AIVertexDeclarationImp9::QueryInterface(REFIID riid, void** ppvObj) 
{
    D3D9_CALL(false, "AIVertexDeclarationImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIVertexDeclarationImp9::AddRef() 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexDeclaration9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;*/

    D3D9_CALL(false, "AIVertexDeclarationImp9::AddRef")
	if(i_parent != 0) {
		refs ++;
    	return refs;
	}
	else return 0;

}

ULONG D3D_CALL AIVertexDeclarationImp9::Release() 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexDeclaration9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;*/

    
    D3D9_CALL(true, "AIVertexDeclarationImp9::Release")
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

HRESULT D3D_CALL AIVertexDeclarationImp9::GetDevice(IDirect3DDevice9** ppDevice) 
{
    D3D9_CALL(true, "AIVertexDeclarationImp9::GetDevice")
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL AIVertexDeclarationImp9::GetDeclaration(D3DVERTEXELEMENT9* pElement, UINT* pNumElements) 
{
    D3D9_CALL(false, "AIVertexDeclarationImp9::GetDeclaration")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}
