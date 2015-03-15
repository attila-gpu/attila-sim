
#include "Utils.h"
#include "GPUMath.h"
#include <map>

using namespace std;
using namespace gpu3d;

#define max(a,b)            (((a) > (b)) ? (a) : (b))

UINT getVolumeSize(UINT Width, UINT Height, UINT Depth, D3DFORMAT Format) {

    UINT size;

    switch(Format)
    {
        case D3DFMT_DXT1:
        case D3DFMT_DXT3:
        case D3DFMT_DXT5:
        case D3DFMT_ATI2:
            // Until don't be found a volume texture that uses a compressed format, won't be supported.
            panic("","getVolumeSize", "Volumes with compressed formats not implemented.");
            break;
        default:
            size = Width * Height * Depth * texel_size(Format);
            break;
    }   

    return size;

}

UINT getSurfaceSize(UINT Width, UINT Height, D3DFORMAT Format)
{
    UINT size;

    switch(Format)
    {
        case D3DFMT_DXT1:
            size = max((UINT) 4, Width) * max((UINT) 4 , Height) / 2;
            break;
        case D3DFMT_DXT3:
        case D3DFMT_DXT5:
        case D3DFMT_ATI2:
            size = max((UINT) 4, Width) * max((UINT) 4, Height);
            break;
        default:
            size = Width * Height * texel_size(Format);
            break;
    }   

    return size;
}

UINT getSurfacePitch(UINT Width, D3DFORMAT Format)
{
    UINT pitch;
    
    switch(Format)
    {
        case D3DFMT_DXT1:
            pitch = max((UINT) 4, Width) * 2;
            break;
        case D3DFMT_DXT3:
        case D3DFMT_DXT5:
        case D3DFMT_ATI2:
            pitch = max((UINT) 4, Width) * 4;
            break;
        default:
            pitch = Width * texel_size(Format);
            break;            
    }
    
    return pitch;

}

UINT getSurfacePitchACD(UINT Width, D3DFORMAT Format)
{
    UINT pitch;
    
    switch(Format)
    {
        case D3DFMT_DXT1:
            pitch = max((UINT) 4, Width) / 2;
            break;
        case D3DFMT_DXT3:
        case D3DFMT_DXT5:
        case D3DFMT_ATI2:
            pitch = max((UINT) 4, Width);
            break;
        default:
            pitch = Width * texel_size(Format);
            break;            
    }
   

    return pitch;
}

u32bit agbr2argb(u32bit agbr) {
     return (agbr & 0xff00ff00) | ((agbr & 0x00ff0000) >> 16) |
        ((agbr & 0x000000ff) << 16);
}

UINT get_vertex_count(D3DPRIMITIVETYPE type, UINT primitive_count) 
{
    u32bit vertex_count = 0;
    switch(type) 
    {
        case D3DPT_TRIANGLELIST:
            vertex_count = 3 * primitive_count;
            break;
        case D3DPT_TRIANGLESTRIP:
            vertex_count = primitive_count + 2;
            break;
        case D3DPT_TRIANGLEFAN:
            vertex_count = primitive_count + 2;
            break;
    }
    return vertex_count;
}

UINT get_index_size(D3DFORMAT IndexDataFormat)
{
    u32bit  size = 0;
    switch(IndexDataFormat) 
    {
        case D3DFMT_INDEX16:
            size = 2;
            break;
        case D3DFMT_INDEX32:
            size = 4;
            break;
        default:
            panic("Utils","get_index_size","Index format unknown");
            break;
    }
    return size;
}


bool is_compressed(D3DFORMAT format)
{
    return (format == D3DFMT_DXT1) ||
            (format == D3DFMT_DXT2) ||
            (format == D3DFMT_DXT3) ||
            (format == D3DFMT_DXT4) ||
            (format == D3DFMT_DXT5) ||
            (format == D3DFMT_ATI2);
}

unsigned int get1DMipLength(UINT Length, UINT Level, UINT MaxLevel, bool Compressed) {
    // CeiledLog2length is the theoretical last level.
    UINT ceiledLog2Length = ceiledLog2(Length);
    // Ceiled length is the next power-of-2 greater or equal to length.
    UINT ceiledLength = ipow(2, ceiledLog2Length);

    // If level demanded is greater than maxlevel clamp it
    if((Level > (MaxLevel - 1)) & (MaxLevel != 0))
        Level = MaxLevel - 1;

    // The last two levels of a compressed mimpap have length 4
    if(Compressed & (Level > (ceiledLog2Length - 2))) {
        return 4;
    }
    // Check if demanded level is greater than the theoretical limit
    else if(Level > ceiledLog2Length) {
        return 1;
    }
    // Apply formula for calculate regular mipmap length
    else
        return ceiledLength / ipow(2, Level);
}

UINT level_count(UINT MaxLevel, UINT Width, UINT Height)
{
    // Calculate the highest level that can be reached
    unsigned int MaxCeiledLog2Length = max(ceiledLog2(Width), ceiledLog2(Height));
    unsigned int TopLevel;
    
    // 0 Maxlevel means all levels
    if(MaxLevel == 0)
    {
        TopLevel = MaxCeiledLog2Length;
    }        
    else 
    {
        TopLevel = MaxLevel - 1;
        
        // Limit to the highest if a greater is demanded
        if (TopLevel > MaxCeiledLog2Length)
            TopLevel = MaxCeiledLog2Length;
    }
    return TopLevel + 1;
}

UINT level_count(UINT MaxLevel, UINT Width, UINT Height, UINT Depth) {
    // Calculate the highest level that can be reached
    unsigned int MaxCeiledLog2Length = max(max(ceiledLog2(Width), ceiledLog2(Height)), ceiledLog2(Depth));
    unsigned int TopLevel;
    // 0 Maxlevel means all levels
    if(MaxLevel == 0)
        TopLevel = MaxCeiledLog2Length;
    else {
        TopLevel = MaxLevel - 1;
        // Limit to the highest if a greater is demanded
        if (TopLevel > MaxCeiledLog2Length)
            TopLevel = MaxCeiledLog2Length;
    }
    return TopLevel + 1;
}


bool ispower2(unsigned int x) {
// Divide x by 2 checking for a remainder
// if it's found, then x isn't power of 2
bool foundRemainder = false;
    while((x != 1) & !foundRemainder) {
        if ((x % 2) != 0)
            foundRemainder = true;
        else
            x /= 2;
    }
    return !foundRemainder;
}

unsigned int ilog2(unsigned int x) {
    unsigned int result = 0;
    while(x != 1) {
        result ++;
        x /= 2;
    }
    return result;
}

unsigned int ipow(unsigned int x, unsigned int y) {
    unsigned int result = 1;
    for(unsigned int i = 0; i < y; i ++)
        result *= x;
    return result;
}

unsigned int ceiledLog2(unsigned int x) {
    unsigned int result = 0;
    result = ilog2(x);
    // Ceil result if its not an exact power of 2
    if (!ispower2(x))
        result ++;
    return result;
}

void d3dcolor2quadfloat(D3DCOLOR in, QuadFloat* out) {
    ///@note reordering components, gpu uses another order
    (*out)[0] = float((in >> 16) & 0xff) / 255.0f;
    (*out)[1] = float((in >> 8)  & 0xff) / 255.0f;
    (*out)[2] = float((in)       & 0xff) / 255.0f;
    (*out)[3] = float((in >> 24) & 0xff) / 255.0f;
}

bool fvf_has_texcoord(BYTE index, DWORD fvf) {

    BYTE tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    return index < tex_count;
}

BYTE fvf_texture_format(BYTE index, DWORD fvf) {
    return 0xfc & (fvf >> (index * 2 + 16));
}

///@todo Implement this in a more elegant way
map<D3DFORMAT, UINT> texel_sizes;
UINT texel_size(D3DFORMAT format) {
    static bool initialized = false;
    if(!initialized) {
        texel_sizes[D3DFMT_A2R10G10B10] = 4;
        texel_sizes[D3DFMT_A8R8G8B8] = 4;
        texel_sizes[D3DFMT_X8R8G8B8] = 4;
        texel_sizes[D3DFMT_A1R5G5B5] = 2;
        texel_sizes[D3DFMT_X1R5G5B5] = 2;
        texel_sizes[D3DFMT_R5G6B5] = 2;
        texel_sizes[D3DFMT_D16_LOCKABLE] = 2;
        texel_sizes[D3DFMT_D32] = 4;
        texel_sizes[D3DFMT_D15S1] = 2;
        texel_sizes[D3DFMT_D24S8] = 4;
        texel_sizes[D3DFMT_D24X8] = 4;
        texel_sizes[D3DFMT_D24X4S4] = 4;
        texel_sizes[D3DFMT_D32F_LOCKABLE] = 4;
        texel_sizes[D3DFMT_D24FS8] = 4;
        texel_sizes[D3DFMT_D16] = 2;

        texel_sizes[D3DFMT_DXT1] = 8;
        texel_sizes[D3DFMT_DXT2] = 16;
        texel_sizes[D3DFMT_DXT3] = 16;
        texel_sizes[D3DFMT_DXT4] = 16;
        texel_sizes[D3DFMT_DXT5] = 16;
        texel_sizes[D3DFMT_ATI2] = 16;

        texel_sizes[D3DFMT_R16F] = 2;
        texel_sizes[D3DFMT_G16R16F] = 4;
        texel_sizes[D3DFMT_A16B16G16R16F] = 8;

        texel_sizes[D3DFMT_MULTI2_ARGB8] = 4; ///@todo this is correct?
        texel_sizes[D3DFMT_R8G8_B8G8]    = 2;
        texel_sizes[D3DFMT_UYVY] = 2;
        texel_sizes[D3DFMT_YUY2] = 2;

        texel_sizes[D3DFMT_INDEX32] = 4;
        texel_sizes[D3DFMT_INDEX16] = 2;
        texel_sizes[D3DFMT_VERTEXDATA] = 0;

        texel_sizes[D3DFMT_R32F] = 4;
        texel_sizes[D3DFMT_G32R32F] = 8;
        texel_sizes[D3DFMT_A32B32G32R32F] = 16;

        texel_sizes[D3DFMT_L6V5U5] = 2;
        texel_sizes[D3DFMT_X8L8V8U8] = 4;
        texel_sizes[D3DFMT_A2W10V10U10] = 4;

        texel_sizes[D3DFMT_V8U8] = 2;
        texel_sizes[D3DFMT_Q8W8V8U8] = 4;
        texel_sizes[D3DFMT_V16U16] = 4;
        texel_sizes[D3DFMT_Q16W16V16U16] = 8;
        texel_sizes[D3DFMT_CxV8U8] = 2;

        texel_sizes[D3DFMT_R8G8B8] = 3;
        texel_sizes[D3DFMT_A8R8G8B8] = 4;
        texel_sizes[D3DFMT_X8R8G8B8] = 4;
        texel_sizes[D3DFMT_R5G6B5] = 2;
        texel_sizes[D3DFMT_X1R5G5B5] = 2;
        texel_sizes[D3DFMT_A1R5G5B5] = 2;
        texel_sizes[D3DFMT_A4R4G4B4] = 2;
        texel_sizes[D3DFMT_R3G3B2] = 1;
        texel_sizes[D3DFMT_A8] = 1;
        texel_sizes[D3DFMT_A8R3G3B2] = 2;
        texel_sizes[D3DFMT_X4R4G4B4] = 2;
        texel_sizes[D3DFMT_A2B10G10R10] = 4;
        texel_sizes[D3DFMT_A8B8G8R8] = 4;
        texel_sizes[D3DFMT_X8B8G8R8] = 4;
        texel_sizes[D3DFMT_G16R16] = 4;
        texel_sizes[D3DFMT_A2R10G10B10] = 4;
        texel_sizes[D3DFMT_A16B16G16R16] = 8;
        texel_sizes[D3DFMT_A8P8] = 2; ///@todo this is correct?
        texel_sizes[D3DFMT_P8] = 1;
        texel_sizes[D3DFMT_L8] = 1;
        texel_sizes[D3DFMT_L16] = 2;
        texel_sizes[D3DFMT_A8L8] = 2;
        texel_sizes[D3DFMT_A4L4] = 1;
        initialized = true;
    }
    return texel_sizes[format];
}

UINT get_volume_size(UINT width, UINT height, UINT depth, D3DFORMAT format) {
    // Volume slices are mortonized, so
    // its size must be rounded to the next squared power of two
    UINT w = ipow(2, ceiledLog2(width));
    UINT h = ipow(2, ceiledLog2(height));
    UINT largest = (w > h) ? w : h;

    if(is_compressed(format)) {
        // Calculate how many 4x4 blocks needed
        largest = largest/ 4 + (((largest % 4) == 0) ? 0 : 1);
    }

    return depth * largest * largest * texel_size(format);
}




/*  Translates texel coordinates to the morton address offset starting from the texture base address.  */
u32bit texel2MortonAddress(u32bit i, u32bit j, u32bit blockDim, u32bit sBlockDim, u32bit width)
{
    u32bit address;
    u32bit texelAddr;
    u32bit blockAddr;
    u32bit sBlockAddr;

    /*  Compute the address of the texel inside the block using Morton order.  */
    texelAddr = GPUMath::morton(blockDim, i, j);

    /*  Compute the address of the block inside the superblock using Morton order.  */
    blockAddr = GPUMath::morton(sBlockDim, i >> blockDim, j >> blockDim);

    /*  Compute the address of the superblock inside the cache.  */
    sBlockAddr = ((j >> (sBlockDim + blockDim)) << GPU_MAX(s32bit(width - (sBlockDim + blockDim)), s32bit(0))) + (i >> (sBlockDim + blockDim));

    /*  Compute the final address.  */
    address = (((sBlockAddr << (2 * sBlockDim)) + blockAddr) << (2 * blockDim)) + texelAddr;

    return address;
}

