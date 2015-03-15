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

#ifndef ACDXSHADERCACHE_H
    #define ACDXSHADERCACHE_H

#include <map>
#include <list>
#include <vector>
#include "ACDTypes.h"
#include "ACDXTLShader.h"
#include "ACDXTLState.h"
#include "ACDXFixedPipelineSettings.h"

namespace acdlib
{

class ACDXShaderCache
{
public:

    ACDXShaderCache();

    /*
     * Try to find if a previous program is in the cache, if it is found the program is returned,
     * otherwise, it is return a false
     */
    acd_bool isCached(const ACDX_FIXED_PIPELINE_SETTINGS shaderSettings, ACDShaderProgram*& cachedProgram, ACDXConstantBindingList*& bindingList);

    /*
     * Add a new entry to the cache. In case the cache is full, it is clear first
     */
    void addCacheEntry(ACDX_FIXED_PIPELINE_SETTINGS shaderSettings, ACDShaderProgram* cachedProgram, ACDXConstantBindingList* bindingList);
    
    /**
     * Clear the program object from cache
     * the cleared programs are returned in a std::vector structure
     */
    void clear();
     
    /**
     * Returns if the cache is full
     */
    acd_bool full() const;
    
    /**
     * Returns the amount of programs stored currently in the cache
     */
    acd_uint size() const;    
    
    /**
     * Dump stadistics obtained from the use of the shader cache
     *
     */
    void dumpStatistics() const;
    
private:

    acd_uint computeChecksum(const ACDX_FIXED_PIPELINE_SETTINGS fpSettings) const;

	bool compareShaderSettings(ACDX_FIXED_PIPELINE_SETTINGS cachedShaderSet, ACDX_FIXED_PIPELINE_SETTINGS checkingShaderSet);

    acd_uint maxPrograms;

    struct cachedDataType
	{
		ACDX_FIXED_PIPELINE_SETTINGS _shaderSettings;
		ACDShaderProgram* shaderProgram;
        ACDXConstantBindingList* constantBinding;

		cachedDataType(ACDX_FIXED_PIPELINE_SETTINGS shaderSettings, ACDShaderProgram* cachedProgram, ACDXConstantBindingList* bindingList)
		{
            shaderProgram = cachedProgram;
            constantBinding = bindingList;

            _shaderSettings.alphaTestEnabled = shaderSettings.alphaTestEnabled;
            _shaderSettings.alphaTestFunction = shaderSettings.alphaTestFunction;
            _shaderSettings.colorMaterialEnabledFace = shaderSettings.colorMaterialEnabledFace;
            _shaderSettings.colorMaterialMode = shaderSettings.colorMaterialMode;
            _shaderSettings.cullEnabled = shaderSettings.cullEnabled;
            _shaderSettings.cullMode = shaderSettings.cullMode;
            _shaderSettings.fogCoordinateSource = shaderSettings.fogCoordinateSource;
            _shaderSettings.fogEnabled = shaderSettings.fogEnabled;
            _shaderSettings.fogMode = shaderSettings.fogMode;
            _shaderSettings.lightingEnabled = shaderSettings.lightingEnabled;

			for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS; i++ )
			{
                _shaderSettings.lights[i].enabled = shaderSettings.lights[i].enabled;
                _shaderSettings.lights[i].lightType = shaderSettings.lights[i].lightType;
			}

            _shaderSettings.localViewer = shaderSettings.localViewer;
            _shaderSettings.normalizeNormals = shaderSettings.normalizeNormals;
            _shaderSettings.separateSpecular = shaderSettings.separateSpecular;

			for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++ )
			{
                _shaderSettings.textureCoordinates[i].coordQ = shaderSettings.textureCoordinates[i].coordQ;
                _shaderSettings.textureCoordinates[i].coordR = shaderSettings.textureCoordinates[i].coordR;
                _shaderSettings.textureCoordinates[i].coordS = shaderSettings.textureCoordinates[i].coordS;
                _shaderSettings.textureCoordinates[i].coordT = shaderSettings.textureCoordinates[i].coordR;
                _shaderSettings.textureCoordinates[i].textureMatrixIsIdentity = shaderSettings.textureCoordinates[i].textureMatrixIsIdentity;
			}

			for (acd_uint i = 0; i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES; i++ )
			{
                _shaderSettings.textureStages[i].activeTextureTarget = shaderSettings.textureStages[i].activeTextureTarget;
                _shaderSettings.textureStages[i].baseInternalFormat = shaderSettings.textureStages[i].baseInternalFormat;
                // shaderSettings.textureStages[i].combineSettings = shaderSettings.textureStages[i].combineSettings;
                _shaderSettings.textureStages[i].enabled = shaderSettings.textureStages[i].enabled;
                _shaderSettings.textureStages[i].textureStageFunction = shaderSettings.textureStages[i].textureStageFunction;
			}

			_shaderSettings.twoSidedLighting = shaderSettings.twoSidedLighting;

		}
	};


    multimap<acd_uint, cachedDataType* >  cache;
    
    acd_uint hits;
    acd_uint misses;
    acd_uint clears;
    
};

} // namespace acdlib

#endif // ACDXSHADERCACHE_H
