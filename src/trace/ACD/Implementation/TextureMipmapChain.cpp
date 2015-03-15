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

#include "TextureMipmapChain.h"
#include <utility>

using namespace std;
using namespace acdlib;

vector<const TextureMipmap*> TextureMipmapChain::getMipmaps(acd_uint min, acd_uint max) const
{
    vector<const TextureMipmap*> mips;

    map<acd_uint, TextureMipmap*>::const_iterator it = _mips.lower_bound(min);
    map<acd_uint, TextureMipmap*>::const_iterator upperBound = _mips.upper_bound(max);
    for ( ; it != upperBound; ++it )
        mips.push_back(it->second);

    return mips;
}

TextureMipmap* TextureMipmapChain::create( acd_uint mipLevel )
{
    TextureMipmap* mip = find(mipLevel);
    
    if ( mip != 0 ) // previos mipmap definition exists, delete it
    {
        delete mip;
        _mips.erase(mipLevel);
    }

    mip = new TextureMipmap();
    _mips.insert(make_pair(mipLevel, mip));
    return mip;
}

void TextureMipmapChain::destroy( acd_uint mipLevel )
{
    map<acd_uint, TextureMipmap*>::iterator it = _mips.find(mipLevel);
    if ( it != _mips.end() )
        delete it->second;
}

void TextureMipmapChain::destroyMipmaps()
{
    map<acd_uint, TextureMipmap*>::iterator it = _mips.begin();
    for ( ; it != _mips.end(); ++it )
        delete it->second;
}

TextureMipmap* TextureMipmapChain::find(acd_uint mipLevel)
{
    map<acd_uint, TextureMipmap*>::iterator it = _mips.find(mipLevel);
    if ( it != _mips.end() )
        return it->second;
    return 0;
}

const TextureMipmap* TextureMipmapChain::find(acd_uint mipLevel) const
{
    map<acd_uint, TextureMipmap*>::const_iterator it = _mips.find(mipLevel);
    if ( it != _mips.end() )
        return it->second;
    return 0;
}

acd_uint TextureMipmapChain::size()
{
	return _mips.size();
}
