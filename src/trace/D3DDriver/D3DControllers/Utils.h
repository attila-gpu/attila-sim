#ifndef D3DCONTROLLERS_UTILS_H
#define D3DCONTROLLERS_UTILS_H

#include "d3d9_port.h"
#include "GPUTypes.h"

// Misc utility stuff that doesn't fit in other places
///@todo Find properly places for the code, don't get this file grow too much

/// Return the size of a volume
UINT getVolumeSize(UINT Width, UINT Height, UINT Depth, D3DFORMAT Format);

/// Return the size of a surface
UINT getSurfaceSize(UINT Width, UINT Height, D3DFORMAT Format);

/// Return the size of a row
UINT getSurfacePitch(UINT Width, D3DFORMAT Format);

/// Return the size of a row
UINT getSurfacePitchACD(UINT Width, D3DFORMAT Format);

/// Swaps red and blue component
u32bit agbr2argb(u32bit agbr);

/// Returns number of vertices given a batch's description
UINT get_vertex_count(D3DPRIMITIVETYPE type, UINT primitive_count);

/// Returns size in bytes of a given index format
UINT get_index_size(D3DFORMAT IndexDataFormat);

/// Returns true if format is compressed
bool is_compressed(D3DFORMAT format);


/**
Returns length in texels of a level of a mipmap,
MaxLevel is maximum level allowed, 0 means all.
*/
unsigned int get1DMipLength(UINT Length, UINT Level, UINT MaxLevel, bool Compressed = false);

/**
Returns level count
MaxLevel is maximum level allowed, 0 means all.
*/
UINT level_count(UINT MaxLevel, UINT Width, UINT Height);

/**
Returns level count for a volume
MaxLevel is maximum level allowed, 0 means all.
*/
UINT level_count(UINT MaxLevel, UINT Width, UINT Height, UINT Depth);

///return true if x is a power of 2 (i. e. 256)
bool ispower2(unsigned int x);
///return integer part of log2(x)
unsigned int ilog2(unsigned int x);
///return x^y
unsigned int ipow(unsigned int x, unsigned int y);
///return log2(x) ceiled if it has fractional part.
unsigned int ceiledLog2(unsigned int x);

void d3dcolor2quadfloat(D3DCOLOR in, gpu3d::QuadFloat* out);

/// Return true if fvf defines given texcoord index
bool fvf_has_texcoord(BYTE index, DWORD fvf);
/// @todo Find out what this function is supposed to do
BYTE fvf_texture_format(BYTE index, DWORD fvf);

/** Returns size in bytes for a texel (or a block of texels,
    for compressed formats */
UINT texel_size(D3DFORMAT);

/** Returns size in bytes for a surface */
UINT get_volume_size(UINT width, UINT height, UINT depth, D3DFORMAT format);

/**  Translates texel coordinates to the morton address offset starting from the texture base address.  */
u32bit texel2MortonAddress(u32bit i, u32bit j, u32bit blockDim, u32bit sBlockDim, u32bit width);

#endif
