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

#ifndef TEXTUREMIPMAPCHAIN
    #define TEXTUREMIPMAPCHAIN

#include "TextureMipmap.h"
#include <map>
#include <vector>

class GPUDriver;

namespace acdlib
{

class TextureMipmapChain
{
public:

    TextureMipmap* create( acd_uint mipLevel );
    
    void destroy( acd_uint mipLevel );
    
    void destroyMipmaps();

    TextureMipmap* find(acd_uint mipmap);
    
    const TextureMipmap* find(acd_uint mipmap) const;

    std::vector<const TextureMipmap*> getMipmaps(acd_uint min, acd_uint max) const;
	
	acd_uint size();

private:

    std::map<acd_uint,TextureMipmap*> _mips;

};

}

#endif // TEXTUREMIPMAPCHAIN
