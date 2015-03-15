////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXLogger.h"
#include "DXTraceManagerHeaders.h"
#include "DXIntPluginManager.h"
#include "GeometryTypes.h"
#include "DXInterceptorBanner.h"
#include "DXInterceptorOptions.h"
#include "main.h"

using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

extern const IID IID_DXInterceptorWrapper;
extern const IID IID_IDirect3D9InterceptorWrapper;
extern const IID IID_IDirect3DDevice9InterceptorWrapper;
extern const IID IID_IDirect3DVertexDeclaration9InterceptorWrapper;
extern const IID IID_IDirect3DVertexBuffer9InterceptorWrapper;
extern const IID IID_IDirect3DIndexBuffer9InterceptorWrapper;
extern const IID IID_IDirect3DTexture9InterceptorWrapper;
extern const IID IID_IDirect3DCubeTexture9InterceptorWrapper;
extern const IID IID_IDirect3DVolume9InterceptorWrapper;
extern const IID IID_IDirect3DVolumeTexture9InterceptorWrapper;
extern const IID IID_IDirect3DSurface9InterceptorWrapper;
extern const IID IID_IDirect3DStateBlock9InterceptorWrapper;
extern const IID IID_IDirect3DPixelShader9InterceptorWrapper;
extern const IID IID_IDirect3DVertexShader9InterceptorWrapper;
extern const IID IID_IDirect3DQuery9InterceptorWrapper;
extern const IID IID_IDirect3DSwapChain9InterceptorWrapper;

////////////////////////////////////////////////////////////////////////////////

interface IDirect3D9InterceptorWrapper;
interface IDirect3DDevice9InterceptorWrapper;
interface IDirect3DVertexDeclaration9InterceptorWrapper;
interface IDirect3DVertexBuffer9InterceptorWrapper;
interface IDirect3DIndexBuffer9InterceptorWrapper;
interface IDirect3DTexture9InterceptorWrapper;
interface IDirect3DCubeTexture9InterceptorWrapper;
interface IDirect3DVolume9InterceptorWrapper;
interface IDirect3DVolumeTexture9InterceptorWrapper;
interface IDirect3DSurface9InterceptorWrapper;
interface IDirect3DStateBlock9InterceptorWrapper;
interface IDirect3DPixelShader9InterceptorWrapper;
interface IDirect3DVertexShader9InterceptorWrapper;
interface IDirect3DQuery9InterceptorWrapper;
interface IDirect3DSwapChain9InterceptorWrapper;

////////////////////////////////////////////////////////////////////////////////

class DXInterceptorWrapper
{
public:

	static DWORD GetNewObjID();	
	
	DXInterceptorWrapper(REFIID iid);
	virtual ~DXInterceptorWrapper();
	
  REFIID GetOwnerIID() const;
  DWORD GetObjectID() const;

protected:

  static DWORD ms_objectIDCounter;

  REFIID m_ownerIID;
  DWORD m_objectID;

};

////////////////////////////////////////////////////////////////////////////////
