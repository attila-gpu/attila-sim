////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXInterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

// {413A97F7-36A0-464f-A0A6-9C9A0BB72A50}
const IID IID_DXInterceptorWrapper = 
{ 0x413a97f7, 0x36a0, 0x464f, { 0xa0, 0xa6, 0x9c, 0x9a, 0xb, 0xb7, 0x2a, 0x50 } };

// {377183BA-FC63-4cfa-B799-10664DA43B73}
const IID IID_IDirect3D9InterceptorWrapper = 
{ 0x377183ba, 0xfc63, 0x4cfa, { 0xb7, 0x99, 0x10, 0x66, 0x4d, 0xa4, 0x3b, 0x73 } };

// {830BCD49-EF6D-46e2-978D-E8002346DEF5}
const IID IID_IDirect3DDevice9InterceptorWrapper = 
{ 0x830bcd49, 0xef6d, 0x46e2, { 0x97, 0x8d, 0xe8, 0x0, 0x23, 0x46, 0xde, 0xf5 } };

// {E62755E6-D813-4b5f-8AC0-C9A6555C3514}
const IID IID_IDirect3DVertexDeclaration9InterceptorWrapper = 
{ 0xe62755e6, 0xd813, 0x4b5f, { 0x8a, 0xc0, 0xc9, 0xa6, 0x55, 0x5c, 0x35, 0x14 } };

// {2CDD1794-1209-458c-BAF4-0EC2DDC65613}
const IID IID_IDirect3DVertexBuffer9InterceptorWrapper = 
{ 0x2cdd1794, 0x1209, 0x458c, { 0xba, 0xf4, 0xe, 0xc2, 0xdd, 0xc6, 0x56, 0x13 } };

// {29F240EC-525D-4995-999F-307251BDB936}
const IID IID_IDirect3DIndexBuffer9InterceptorWrapper = 
{ 0x29f240ec, 0x525d, 0x4995, { 0x99, 0x9f, 0x30, 0x72, 0x51, 0xbd, 0xb9, 0x36 } };

// {53101CED-8A45-4f0a-84B1-4117DBD30096}
const IID IID_IDirect3DTexture9InterceptorWrapper = 
{ 0x53101ced, 0x8a45, 0x4f0a, { 0x84, 0xb1, 0x41, 0x17, 0xdb, 0xd3, 0x0, 0x96 } };

// {BCBDA2DB-E1AF-4352-B510-D5FCF164846F}
const IID IID_IDirect3DCubeTexture9InterceptorWrapper = 
{ 0xbcbda2db, 0xe1af, 0x4352, { 0xb5, 0x10, 0xd5, 0xfc, 0xf1, 0x64, 0x84, 0x6f } };

// {534D9EC5-11AA-4ef4-8455-A1943090DEBC}
const IID IID_IDirect3DVolume9InterceptorWrapper = 
{ 0x534d9ec5, 0x11aa, 0x4ef4, { 0x84, 0x55, 0xa1, 0x94, 0x30, 0x90, 0xde, 0xbc } };

// {5E9CB985-7BE7-4afe-8C7A-5FFC1AF24126}
const IID IID_IDirect3DVolumeTexture9InterceptorWrapper = 
{ 0x5e9cb985, 0x7be7, 0x4afe, { 0x8c, 0x7a, 0x5f, 0xfc, 0x1a, 0xf2, 0x41, 0x26 } };

// {989A7F2F-B362-4cc4-9C32-3562D48C14AE}
const IID IID_IDirect3DSurface9InterceptorWrapper = 
{ 0x989a7f2f, 0xb362, 0x4cc4, { 0x9c, 0x32, 0x35, 0x62, 0xd4, 0x8c, 0x14, 0xae } };

// {364C8AC4-A3AD-4861-9827-9E41F63206E8}
const IID IID_IDirect3DStateBlock9InterceptorWrapper = 
{ 0x364c8ac4, 0xa3ad, 0x4861, { 0x98, 0x27, 0x9e, 0x41, 0xf6, 0x32, 0x6, 0xe8 } };

// {08E9BFE7-06F6-405f-A69A-A42A914B4CAB}
const IID IID_IDirect3DPixelShader9InterceptorWrapper = 
{ 0x8e9bfe7, 0x6f6, 0x405f, { 0xa6, 0x9a, 0xa4, 0x2a, 0x91, 0x4b, 0x4c, 0xab } };

// {6782FC32-AFF7-4c76-B659-9CC000349D19}
const IID IID_IDirect3DVertexShader9InterceptorWrapper = 
{ 0x6782fc32, 0xaff7, 0x4c76, { 0xb6, 0x59, 0x9c, 0xc0, 0x0, 0x34, 0x9d, 0x19 } };

// {6B974ACE-0468-4d9a-9E1F-D55CAC7469C3}
const IID IID_IDirect3DQuery9InterceptorWrapper = 
{ 0x6b974ace, 0x468, 0x4d9a, { 0x9e, 0x1f, 0xd5, 0x5c, 0xac, 0x74, 0x69, 0xc3 } };

// {8C34998F-7299-49d6-BD1F-68267E7344EC}
const IID IID_IDirect3DSwapChain9InterceptorWrapper = 
{ 0x8c34998f, 0x7299, 0x49d6, { 0xbd, 0x1f, 0x68, 0x26, 0x7e, 0x73, 0x44, 0xec } };


////////////////////////////////////////////////////////////////////////////////

DWORD DXInterceptorWrapper::ms_objectIDCounter = 1;

////////////////////////////////////////////////////////////////////////////////

DXInterceptorWrapper::DXInterceptorWrapper(REFIID iid) :
m_ownerIID(iid)
{
	m_objectID = DXInterceptorWrapper::GetNewObjID();
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorWrapper::~DXInterceptorWrapper()
{
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXInterceptorWrapper::GetNewObjID()
{
	return DXInterceptorWrapper::ms_objectIDCounter++;
}

////////////////////////////////////////////////////////////////////////////////

REFIID DXInterceptorWrapper::GetOwnerIID() const
{
  return m_ownerIID;
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXInterceptorWrapper::GetObjectID() const
{
	return m_objectID;
}

////////////////////////////////////////////////////////////////////////////////