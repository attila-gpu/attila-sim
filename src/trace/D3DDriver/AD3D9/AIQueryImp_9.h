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

#ifndef AIQUERYIMP_9_H_INCLUDED
#define AIQUERYIMP_9_H_INCLUDED

class AIQueryImp9 : public IDirect3DQuery9{
public:
    static AIQueryImp9 &getInstance();
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    D3DQUERYTYPE D3D_CALL GetType ( );
    DWORD D3D_CALL GetDataSize ( );
    HRESULT D3D_CALL Issue (  DWORD dwIssueFlags );
    HRESULT D3D_CALL GetData (  void* pData , DWORD dwSize , DWORD dwGetDataFlags );
private:
    AIQueryImp9();
};


#endif

