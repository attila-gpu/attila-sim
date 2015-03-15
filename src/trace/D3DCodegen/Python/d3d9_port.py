import d3d9api
from d3d9api import *

api = getD3D9API("d3d9api.xml")

f = open("d3d9_port.h","w")

print >>f, """
/**
 * D3D9 portable header (windows/linux)
 *
 **/

#ifndef _D3D9_PORT
#define _D3D9_PORT

#ifdef WIN32

// Define call convention
#define D3D_CALL _stdcall

/**
 * Include sdk provided d3d9 header
 */
#include <d3d9.h>
#else
/**
 * Non windows systems use following d3d9 header.
 */

/*
 * Basic types
 */

typedef int INT;
typedef unsigned int UINT;
typedef int BOOL;
typedef long HRESULT;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef float FLOAT;

struct LARGE_INTEGER {
	DWORD LowPart;
	LONG HighPart;
};

/*
 * Windows GDI structs
 */

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT;

typedef struct tagRECT {
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT;

typedef struct tagPALETTEENTRY {
    BYTE        peRed;
    BYTE        peGreen;
    BYTE        peBlue;
    BYTE        peFlags;
} PALETTEENTRY;

typedef struct _RGNDATAHEADER {
    DWORD   dwSize;
    DWORD   iType;
    DWORD   nCount;
    DWORD   nRgnSize;
    RECT    rcBound;
} RGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER   rdh;
    char            Buffer[1];
} RGNDATA;

typedef struct _GUID{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

struct HDC__{
int unused;
};

struct HWND__{
int unused;
};

struct HMONITOR__{
int unused;
};

#define REFGUID const _GUID &
#define REFIID  const _GUID &
#define CONST const

typedef HDC__ *HDC;
typedef HWND__ *HWND;
typedef HMONITOR__ *HMONITOR;

typedef void *HANDLE;

/*
 * The ubiquitous IUnknown interface...
 * FUN Try to pronounce 'ubiquitous' fast and repeteadly with a friend.
 *     The first that makes a mistake pay a drink TO THE OTHER.
 *     Restart game
 *     The theory predicts (long term) equal scores.
 */

class IUnknown
{
public:
 virtual ULONG AddRef(void) = 0; 
 virtual HRESULT QueryInterface(REFIID riid, void **ppvObject) = 0;
 virtual ULONG Release(void) = 0; 
};

// Keep default call convention for methods
#define D3D_CALL


/*
 * Forward declarations 
 */

class IDirect3D9;
class IDirect3DDevice9;
class IDirect3DSwapChain9;
class IDirect3DTexture9;
class IDirect3DVolumeTexture9;
class IDirect3DCubeTexture9;
class IDirect3DVertexBuffer9;
class IDirect3DIndexBuffer9;
class IDirect3DSurface9;
class IDirect3DVolume9;
class IDirect3DVertexDeclaration9;
class IDirect3DVertexShader9;
class IDirect3DPixelShader9;
class IDirect3DStateBlock9;
class IDirect3DQuery9;
class IDirect3DBaseTexture9;
class IDirect3DResource9;

#include <d3d9types.h>
#include <d3d9caps.h>

"""

for i in api.interfaces:
    print >>f, "class ", i.name, " : public ", i.base, "{"
    print >>f, "public:"
    for m in i.methods:
        print >>f, "    virtual", m.returns, m.name, "(",
        comma = ""
        for p in m.parameters:
            print >>f, comma, p.type, p.name,
            comma = ","
        print >>f, ") = 0;"
    print >>f, "};"
    print >>f, ""

for ff in api.functions:
    # Some functions have empty return type and ISO C++ forbids this.
    # Void is generated in this case.
    returns = "void"
    if ff.returns != "":
        returns = ff.returns
    print >>f, returns, ff.name, "(",
    comma = ""
    for p in ff.parameters:
        print >>f, comma, p.type, p.name,
        comma = ","
    print >>f, ");"

print >> f, """
#endif // WIN32
#endif

"""

f.close();
