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

#include "ACDXShaderCache.h"
#include "InternalConstantBinding.h"


using namespace std;
using namespace acdlib;

ACDXShaderCache::ACDXShaderCache()
{
    maxPrograms = 100;
}

acd_bool ACDXShaderCache::isCached(const ACDX_FIXED_PIPELINE_SETTINGS shaderSettings, ACDShaderProgram*& cachedProgram, ACDXConstantBindingList*& bindingList)
{

    acd_uint crc = computeChecksum(shaderSettings);    // CRC creation of the entry shader settings

    pair<multimap<acd_uint, cachedDataType* >::iterator, 
         multimap<acd_uint, cachedDataType* >::iterator> range = cache.equal_range(crc);    // check if the resultant CRC is inside the cache

    
    if ( range.first != range.second )  // For every match with the same CRC check if any one have the entry shader setting
    {
        for ( multimap<acd_uint, cachedDataType* >::iterator matching = range.first; matching != range.second; matching++ )
        {
			if ( compareShaderSettings(matching->second->_shaderSettings, shaderSettings) ) // Compares the input and the cache entry settings struct
            {
                hits++; // Hit stats
				delete cachedProgram;
                cachedProgram = matching->second->shaderProgram;
                bindingList = matching->second->constantBinding;
                return true; // Return a hit
            }
        }
    }

    return false; // Return false in case of a miss*/
}

void ACDXShaderCache::addCacheEntry(ACDX_FIXED_PIPELINE_SETTINGS shaderSettings, ACDShaderProgram* cachedProgram, ACDXConstantBindingList* bindingList)
{

    acd_uint crc = computeChecksum(shaderSettings);    // CRC creation of the entry shader settings

    if ( cache.size() == maxPrograms ) // Check if the cache is full, in that case clear it
    {
        cout << "Attention: shader cache full, clearing it \n";
        clears++; // Clear stats
        clear();
    }
        
    misses++; // Miss stats
	
	cachedDataType* newEntry = new cachedDataType(shaderSettings, cachedProgram, bindingList);

    cache.insert(make_pair(crc, newEntry)); // Insert the new cache entry

}

bool ACDXShaderCache::compareShaderSettings(ACDX_FIXED_PIPELINE_SETTINGS cachedShaderSet, ACDX_FIXED_PIPELINE_SETTINGS checkingShaderSet)
{

    acd_bool equivalent = true;

    equivalent &= cachedShaderSet.alphaTestEnabled == checkingShaderSet.alphaTestEnabled;
    equivalent &= cachedShaderSet.alphaTestFunction == checkingShaderSet.alphaTestFunction;
    equivalent &= cachedShaderSet.colorMaterialEnabledFace == checkingShaderSet.colorMaterialEnabledFace;
    equivalent &= cachedShaderSet.colorMaterialMode == checkingShaderSet.colorMaterialMode;
    equivalent &= cachedShaderSet.cullEnabled == checkingShaderSet.cullEnabled;
    equivalent &= cachedShaderSet.cullMode == checkingShaderSet.cullMode;
    equivalent &= cachedShaderSet.fogCoordinateSource == checkingShaderSet.fogCoordinateSource;
    equivalent &= cachedShaderSet.fogEnabled == checkingShaderSet.fogEnabled;
    equivalent &= cachedShaderSet.fogMode == checkingShaderSet.fogMode;
    equivalent &= cachedShaderSet.lightingEnabled == checkingShaderSet.lightingEnabled;

    for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS; i++ )
    {
        equivalent &= cachedShaderSet.lights[i].enabled == checkingShaderSet.lights[i].enabled;
        equivalent &= cachedShaderSet.lights[i].lightType == checkingShaderSet.lights[i].lightType;
    }

    equivalent &= cachedShaderSet.localViewer == checkingShaderSet.localViewer;
    equivalent &= cachedShaderSet.normalizeNormals == checkingShaderSet.normalizeNormals;
    equivalent &= cachedShaderSet.separateSpecular == checkingShaderSet.separateSpecular;

    for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++ )
    {
        equivalent &= cachedShaderSet.textureCoordinates[i].coordQ == checkingShaderSet.textureCoordinates[i].coordQ;
        equivalent &= cachedShaderSet.textureCoordinates[i].coordR == checkingShaderSet.textureCoordinates[i].coordR;
        equivalent &= cachedShaderSet.textureCoordinates[i].coordS == checkingShaderSet.textureCoordinates[i].coordS;
        equivalent &= cachedShaderSet.textureCoordinates[i].coordT == checkingShaderSet.textureCoordinates[i].coordR;
        equivalent &= cachedShaderSet.textureCoordinates[i].textureMatrixIsIdentity == checkingShaderSet.textureCoordinates[i].textureMatrixIsIdentity;
    }

    for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++ )
    {
        equivalent &= cachedShaderSet.textureStages[i].activeTextureTarget == checkingShaderSet.textureStages[i].activeTextureTarget;
        equivalent &= cachedShaderSet.textureStages[i].baseInternalFormat == checkingShaderSet.textureStages[i].baseInternalFormat;
        //equivalent &= cachedShaderSet.textureStages[i].combineSettings == checkingShaderSet.textureStages[i].combineSettings;
        equivalent &= cachedShaderSet.textureStages[i].enabled == checkingShaderSet.textureStages[i].enabled;
        equivalent &= cachedShaderSet.textureStages[i].textureStageFunction == checkingShaderSet.textureStages[i].textureStageFunction;
    }

    equivalent &= cachedShaderSet.twoSidedLighting == checkingShaderSet.twoSidedLighting;

    return equivalent;

}

acd_uint ACDXShaderCache::computeChecksum(const ACDX_FIXED_PIPELINE_SETTINGS fpSettings) const
{

    acd_uint crc = 0;

    crc += acd_uint(fpSettings.alphaTestEnabled);
    crc += acd_uint(fpSettings.alphaTestFunction);
    crc += acd_uint(fpSettings.colorMaterialEnabledFace);
    crc += acd_uint(fpSettings.colorMaterialMode);
    crc += acd_uint(fpSettings.cullEnabled);
    crc += acd_uint(fpSettings.cullMode);
    crc += acd_uint(fpSettings.fogCoordinateSource);
    crc += acd_uint(fpSettings.fogEnabled);
    crc += acd_uint(fpSettings.fogMode);
    crc += acd_uint(fpSettings.lightingEnabled);

	for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS; i++ )
    {
        crc += acd_uint(fpSettings.lights[i].enabled);
        crc += acd_uint(fpSettings.lights[i].lightType);
    }

    crc += acd_uint(fpSettings.localViewer);
    crc += acd_uint(fpSettings.normalizeNormals);
    crc += acd_uint(fpSettings.separateSpecular);

    for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++ )
    {
        crc += acd_uint(fpSettings.textureCoordinates[i].coordQ);
        crc += acd_uint(fpSettings.textureCoordinates[i].coordR);
        crc += acd_uint(fpSettings.textureCoordinates[i].coordS);
        crc += acd_uint(fpSettings.textureCoordinates[i].coordT);
        crc += acd_uint(fpSettings.textureCoordinates[i].textureMatrixIsIdentity);
    }

    for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++ )
    {
        crc += acd_uint(fpSettings.textureStages[i].activeTextureTarget);
        crc += acd_uint(fpSettings.textureStages[i].baseInternalFormat);
        //crc += acd_uint(fpSettings.textureStages[i].combineSettings);
        crc += acd_uint(fpSettings.textureStages[i].enabled);
        crc += acd_uint(fpSettings.textureStages[i].textureStageFunction);
    }

    crc += acd_uint(fpSettings.twoSidedLighting);

    return crc;
}

void ACDXShaderCache::clear()
{    
    for (multimap<acd_uint, cachedDataType* >::iterator it = cache.begin() ; it != cache.end(); it++ )
    {
        ACDXConstantBindingList::iterator iter = it->second->constantBinding->begin();

        while (iter != it->second->constantBinding->end())
        {
            //Delete InternalConstantBinding object TO DO: CAUTION: Not all the cb are icb´s
            InternalConstantBinding* icb = static_cast<InternalConstantBinding*>(*iter);

            delete icb;

            iter++;
        }

        delete (it->second->constantBinding);

        delete (it->second);

    }

    cache.clear();

    clears++;
}

bool ACDXShaderCache::full() const
{
	return cache.size() == maxPrograms;
}

acd_uint ACDXShaderCache::size() const
{
	return (u32bit) cache.size();
}

void ACDXShaderCache::dumpStatistics() const
{ 
    cout << "hits: " << hits << " (" << (acd_float)(100.0f * ((acd_float)hits/(acd_float)(hits + misses))) << "%)\n";
    cout << "misses: " << misses << " (" << (acd_float)(100.0f * ((acd_float)misses/(acd_float)(hits + misses))) << "%)\n";
    cout << "clears: " << clears << "\n";
    cout << "programs in cache: " << cache.size() << endl;
}
