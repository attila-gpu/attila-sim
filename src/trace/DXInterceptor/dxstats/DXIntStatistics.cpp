////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTraceManagerHeaders.h"
#define DXINTPLUGIN_IMPLEMENTATION
#include "DXIntPlugin.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#define DXINT_PLUGIN_STATS_VERSION 0x0100

////////////////////////////////////////////////////////////////////////////////
// Macro for determining the number of elements in an array

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#endif

////////////////////////////////////////////////////////////////////////////////

enum STATSCOUNTERID
{
  CTR_FRAMESPERSECOND,
  CTR_CALLSPERFRAME,
  CTR_BATCHESPERFRAME,
  CTR_VERTICESPERFRAME,
  CTR_TRIANGLESPERFRAME,
};

////////////////////////////////////////////////////////////////////////////////

DXINTCOUNTERINFO countersSet[] =
{
  {
    CTR_FRAMESPERSECOND,
      "FramesPerSecond",
      "Frames per second at this frame",
      DXICDT_FLOAT
  },  
  {
    CTR_CALLSPERFRAME,
    "CallsPerFrame",
    "# of processed calls at this frame",
    DXICDT_UINT32
  },
  {
    CTR_BATCHESPERFRAME,
    "BatchesPerFrame",
    "# of processed batches at this frame",
    DXICDT_UINT32
  },
  {
    CTR_VERTICESPERFRAME,
    "VerticesPerFrame",
    "# of processed vertices at this frame",
    DXICDT_UINT32
  },
  {
    CTR_TRIANGLESPERFRAME,
    "TrianglesPerFrame",
    "# of processed triangles at this frame",
    DXICDT_UINT32
  },
};

////////////////////////////////////////////////////////////////////////////////
// Frames Per Second

dx_float  g_FRAMESPERSECOND = 0.0;
dx_float  g_FRAMESPERSECOND_result = 0.0;
dx_uint32 g_lastTimeStamp = 0;
dx_uint32 g_numFramesFromLastSecond = 0;
dx_uint32 g_timeElapsedFromLastSecond = 0;

void FramesPerSecondUpdateCounters()
{
  g_numFramesFromLastSecond++;

  dx_uint32 currentTimeStamp = timeGetTime();
  dx_uint32 timeElapsed = currentTimeStamp - g_lastTimeStamp;
  g_lastTimeStamp = currentTimeStamp;
  g_timeElapsedFromLastSecond += timeElapsed;

  if (g_timeElapsedFromLastSecond >= 1000)
  {
    if (g_numFramesFromLastSecond)
    {
      g_FRAMESPERSECOND = g_numFramesFromLastSecond * 1000.0f / g_timeElapsedFromLastSecond;
    }
    else
    {
      g_FRAMESPERSECOND = 0.0;
    }

    g_numFramesFromLastSecond = 0;
    g_timeElapsedFromLastSecond = 0;
  }
}

BOOL FramesPerSecond_Begin()
{
  g_FRAMESPERSECOND = 0.0;
  g_lastTimeStamp = timeGetTime();
  g_numFramesFromLastSecond = 0;
  g_timeElapsedFromLastSecond = 0;
  return TRUE;
}

BOOL FramesPerSecond_ProcessCall(DXMethodCallPtr call)
{
  return TRUE;
}

BOOL FramesPerSecond_Frame(BYTE** data, UINT* size)
{
  FramesPerSecondUpdateCounters();

  g_FRAMESPERSECOND_result = g_FRAMESPERSECOND;

  *data = (BYTE*) &g_FRAMESPERSECOND_result;
  *size = sizeof(dx_float);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Calls Per Frame

dx_uint32 g_CALLSPERFRAME = 0;
dx_uint32 g_CALLSPERFRAME_result = 0;

BOOL CallsPerFrame_Begin()
{
  g_CALLSPERFRAME = 0;
  return TRUE;
}

BOOL CallsPerFrame_ProcessCall(DXMethodCallPtr call)
{
  g_CALLSPERFRAME++;
  return TRUE;
}

BOOL CallsPerFrame_Frame(BYTE** data, UINT* size)
{
  g_CALLSPERFRAME_result = g_CALLSPERFRAME;
  g_CALLSPERFRAME = 0;

  *data = (BYTE*) &g_CALLSPERFRAME_result;
  *size = sizeof(dx_uint32);
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Batches Per Frame

dx_uint32 g_BATCHESPERFRAME = 0;
dx_uint32 g_BATCHESPERFRAME_result = 0;

BOOL BatchesPerFrame_Begin()
{
  g_BATCHESPERFRAME = 0;
  return TRUE;
}

BOOL BatchesPerFrame_ProcessCall(DXMethodCallPtr call)
{
  if (call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_DrawIndexedPrimitive ||
      call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_DrawIndexedPrimitiveUP ||
      call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_DrawPrimitive ||
      call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_DrawPrimitiveUP)
  {
    g_BATCHESPERFRAME++;
  }
  return TRUE;
}

BOOL BatchesPerFrame_Frame(BYTE** data, UINT* size)
{
  g_BATCHESPERFRAME_result = g_BATCHESPERFRAME;
  g_BATCHESPERFRAME = 0;

  *data = (BYTE*) &g_BATCHESPERFRAME_result;
  *size = sizeof(dx_uint32);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Vertices Per Frame

dx_uint32 g_VERTICESPERFRAME = 0;
dx_uint32 g_VERTICESPERFRAME_result = 0;

BOOL VerticesPerFrame_Begin()
{
  g_VERTICESPERFRAME = 0;
  g_VERTICESPERFRAME_result = 0;
  return TRUE;
}

UINT VerticesPerFrame_CalculateNumTriangles(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount)
{
  switch (primitiveType)
  {
  case D3DPT_POINTLIST:
    return primitiveCount;

  case D3DPT_LINELIST:
    return primitiveCount*2;

  case D3DPT_LINESTRIP:
    return primitiveCount*2 + 1;

  case D3DPT_TRIANGLELIST:
    return primitiveCount*3;

  case D3DPT_TRIANGLESTRIP:
    return primitiveCount + 2;

  case D3DPT_TRIANGLEFAN:
    return primitiveCount + 2;
  }

  return 0;
}

BOOL VerticesPerFrame_ProcessCall(DXMethodCallPtr call)
{
  D3DPRIMITIVETYPE primitiveType;
  UINT primitiveCount;

  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawIndexedPrimitive:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(5, (char*) &primitiveCount);
    g_VERTICESPERFRAME += VerticesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawPrimitive:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(2, (char*) &primitiveCount);
    g_VERTICESPERFRAME += VerticesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawIndexedPrimitiveUP:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(3, (char*) &primitiveCount);
    g_VERTICESPERFRAME += VerticesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawPrimitiveUP:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(1, (char*) &primitiveCount);
    g_VERTICESPERFRAME += VerticesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;
  }

  return TRUE;
}

BOOL VerticesPerFrame_Frame(BYTE** data, UINT* size)
{
  g_VERTICESPERFRAME_result = g_VERTICESPERFRAME;
  g_VERTICESPERFRAME = 0;

  *data = (BYTE*) &g_VERTICESPERFRAME_result;
  *size = sizeof(dx_uint32);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// Triangles Per Frame

dx_uint32 g_TRIANGLESPERFRAME = 0;
dx_uint32 g_TRINAGLESPERFRAME_result = 0;

BOOL TrianglesPerFrame_Begin()
{
  g_TRIANGLESPERFRAME = 0;
  g_TRINAGLESPERFRAME_result = 0;
  return TRUE;
}

UINT TrianglesPerFrame_CalculateNumTriangles(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount)
{
  switch (primitiveType)
  {
  case D3DPT_POINTLIST:
  case D3DPT_LINELIST:
  case D3DPT_LINESTRIP:
    return 0;

  case D3DPT_TRIANGLELIST:
  case D3DPT_TRIANGLESTRIP:
  case D3DPT_TRIANGLEFAN:
    return primitiveCount;
  }

  return 0;
}

BOOL TrianglesPerFrame_ProcessCall(DXMethodCallPtr call)
{
  D3DPRIMITIVETYPE primitiveType;
  UINT primitiveCount;
  
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawIndexedPrimitive:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(5, (char*) &primitiveCount);
    g_TRIANGLESPERFRAME += TrianglesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawPrimitive:  
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(2, (char*) &primitiveCount);
    g_TRIANGLESPERFRAME += TrianglesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawIndexedPrimitiveUP:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(3, (char*) &primitiveCount);
    g_TRIANGLESPERFRAME += TrianglesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_DrawPrimitiveUP:
    call->GetParam(0, (char*) &primitiveType);
    call->GetParam(1, (char*) &primitiveCount);
    g_TRIANGLESPERFRAME += TrianglesPerFrame_CalculateNumTriangles(primitiveType, primitiveCount);
    break;
  }
  
  return TRUE;
}

BOOL TrianglesPerFrame_Frame(BYTE** data, UINT* size)
{
  g_TRINAGLESPERFRAME_result = g_TRIANGLESPERFRAME;
  g_TRIANGLESPERFRAME = 0;

  *data = (BYTE*) &g_TRINAGLESPERFRAME_result;
  *size = sizeof(dx_uint32);

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

DLLEXP BOOL DXIntPlugin_GetPluginInfo(DXINTPLUGININFO* info)
{
  info->Name = "Basic Statistics Plugin";
  info->Version = DXINT_PLUGIN_STATS_VERSION;
  info->SystemVersion = DXINT_PLUGIN_SYSTEM_VERSION;
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

DLLEXP BOOL DXIntPlugin_GetCounterInfo(DWORD* countersCount, DXINTCOUNTERINFO** countersArray)
{
  *countersCount = ARRAY_LENGTH(countersSet);
  *countersArray = &countersSet[0];
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

DLLEXP BOOL DXIntPlugin_BeginExperiment(DXINTCOUNTERID counterID)
{
  switch (counterID)
  {
  case CTR_FRAMESPERSECOND:   return FramesPerSecond_Begin();
  case CTR_CALLSPERFRAME:     return CallsPerFrame_Begin();
  case CTR_BATCHESPERFRAME:   return BatchesPerFrame_Begin();
  case CTR_VERTICESPERFRAME:  return VerticesPerFrame_Begin();
  case CTR_TRIANGLESPERFRAME: return TrianglesPerFrame_Begin();
  default: return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

DLLEXP BOOL DXIntPlugin_ProcessCall(DXINTCOUNTERID counterID, DXMethodCallPtr call)
{
  switch (counterID)
  {
  case CTR_FRAMESPERSECOND:   return FramesPerSecond_ProcessCall(call);
  case CTR_CALLSPERFRAME:     return CallsPerFrame_ProcessCall(call);
  case CTR_BATCHESPERFRAME:   return BatchesPerFrame_ProcessCall(call);
  case CTR_VERTICESPERFRAME:  return VerticesPerFrame_ProcessCall(call);
  case CTR_TRIANGLESPERFRAME: return TrianglesPerFrame_ProcessCall(call);
  default: return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

DLLEXP BOOL DXIntPlugin_EndFrame(DXINTCOUNTERID counterID, BYTE** data, UINT* size)
{
  switch (counterID)
  {
  case CTR_FRAMESPERSECOND:   return FramesPerSecond_Frame(data, size);
  case CTR_CALLSPERFRAME:     return CallsPerFrame_Frame(data, size);
  case CTR_BATCHESPERFRAME:   return BatchesPerFrame_Frame(data, size);
  case CTR_VERTICESPERFRAME:  return VerticesPerFrame_Frame(data, size);
  case CTR_TRIANGLESPERFRAME: return TrianglesPerFrame_Frame(data, size);
  default: return FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////

DLLEXP BOOL DXIntPlugin_EndExperiment(DXINTCOUNTERID counterID)
{
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
