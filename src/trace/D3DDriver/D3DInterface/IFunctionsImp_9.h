#ifndef IFUNCTIONSIMP_H_INCLUDED
#define IFUNCTIONSIMP_H_INCLUDED

IDirect3D9 * D3D_CALL Direct3DCreate9 (
 UINT SDKVersion
);
void D3D_CALL DebugSetMute (
 UINT SDKVersion
);
void D3D_CALL Direct3DShaderValidatorCreate9 (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_BeginEvent (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_EndEvent (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_SetMarker (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_SetRegion (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_QueryRepeatFrame (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_SetOptions (
 UINT SDKVersion
);
void D3D_CALL D3DPERF_GetStatus (
 UINT SDKVersion
);

#endif

