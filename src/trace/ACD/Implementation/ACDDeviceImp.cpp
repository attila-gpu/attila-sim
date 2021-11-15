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

#include "ACDDeviceImp.h"
#include "ACDBlendingStageImp.h"
#include "ACDRasterizationStageImp.h"
#include "ACDZStencilStageImp.h"
#include "ACDRenderTargetImp.h"
#include "ACDShaderProgramImp.h"
#include "ACDStoredStateImp.h"

#include "ACDShaderProgramImp.h"
#include "ACDBufferImp.h"
#include "ACDStreamImp.h"
#include "ACDSamplerImp.h"

#include "ACDMath.h"
#include "ACDMacros.h"
#include "TextureMipmap.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#include "ACDTexture2DImp.h"
#include "ACDTextureCubeMapImp.h"
#include "TextureAdapter.h"

#include "GlobalProfiler.h"

using namespace std;

using namespace acdlib;

acd_ubyte ACDDeviceImp::defaultVSh[] = "mov o0, i0\n"
                                       "mov o1, i1\n";

acd_ubyte ACDDeviceImp::defaultFSh[] = "mov o1, i1\n";

ACDDeviceImp::ACDDeviceImp(GPUDriver* driver) :
    _driver(driver),
    _requiredSync(true),
    _MAX_STREAMS(gpu3d::MAX_STREAM_BUFFERS - 1),
    _MAX_SAMPLES(16),
    //_MAX_STREAMS(3),
    _indexedMode(false),
    _clearColorBuffer(0),
    _primitive(ACD_POINTS),
    _vaMap(gpu3d::MAX_VERTEX_ATTRIBUTES, gpu3d::ST_INACTIVE_ATTRIBUTE),
    _gsh(0), _gpuMemGshTrack(0),
    _vsh(0), _gpuMemVshTrack(0),
    _fsh(0), _gpuMemFshTrack(0),
    _vshOutputs(gpu3d::MAX_VERTEX_ATTRIBUTES, false),
    _fshInputs(gpu3d::MAX_FRAGMENT_ATTRIBUTES, false),
    _vshResources(1),
    _fshResources(1),
    _shOptimizer(acdlib_opt::SHADER_ARCH_PARAMS()),
    //_stream(gpu3d::MAX_STREAM_BUFFERS - 1),
    _stream(_MAX_STREAMS),
    _zClearValue(1.0f),
    _stencilClearValue(0x00ffffff),
    _hzActivated(false),
    _hzBufferValid(false),
    _defaultBackBuffer(0),
    _defaultFrontBuffer(0),
    _defaultZStencilBuffer(0),
    _currentZStencilBuffer(0),
    _defaultRenderBuffers(false),
    _defaultZStencilBufferInUse(false),
    _colorSRGBWrite(false),
    _startFrame(0),
    _currentFrame(0),
    _currentBatch(0),
    _alphaTest(false),
    _streamStart(0),
    _streamCount(0),
    _streamInstances(1),
    _earlyZ(true),
    _currentColor(acd_float(0.0))
{
    _driver->setContext(this);

    _moa = new MemoryObjectAllocator(driver);

    acd_uint tileLevel1Sz;
    acd_uint tileLevel2Sz;
    _driver->getTextureTilingParameters(tileLevel1Sz, tileLevel2Sz);

    // configure the texture tiling used in texture mipmaps
    TextureMipmap::setTextureTiling(tileLevel1Sz, tileLevel2Sz);
    TextureMipmap::setDriver(_driver);

    // Create stream objects
    for ( acd_uint i = 0; i < _MAX_STREAMS; ++i )
        _stream[i] = new ACDStreamImp(this, _driver, i);

    // The last available stream in attila is reserved to stream indices when indexedMode is enabled
    _indexStream = new ACDStreamImp(this, _driver, _MAX_STREAMS);

    // Init register with the stream of indices
    gpu3d::GPURegData data;
    data.uintVal = _MAX_STREAMS;
    _driver->writeGPURegister(gpu3d::GPU_INDEX_STREAM, 0, data);

    _sampler = new ACDSamplerImp*[gpu3d::MAX_TEXTURES];
    for ( acd_uint i = 0; i < gpu3d::MAX_TEXTURES; ++i )
        _sampler[i] = new ACDSamplerImp(this, _driver, i);

    // Create rasterization stage object
    _rast = new ACDRasterizationStageImp(this, _driver);

    // Create ZStencil stage object
    _zStencil = new ACDZStencilStageImp(this, _driver);

    // Create Blending stage object
    _blending = new ACDBlendingStageImp(this, _driver);

    _syncRegisters();

    // Start the deferred dump structures
    _nextDumpEvent.valid = false;

    data.uintVal = _MAX_STREAMS;
    _driver->writeGPURegister(gpu3d::GPU_INDEX_STREAM, 0, data);


    _defaultVshProgram = (ACDShaderProgramImp*)(createShaderProgram());
    _defaultFshProgram = (ACDShaderProgramImp*)(createShaderProgram());

    _defaultVshProgram->setProgram(defaultVSh);
    _defaultFshProgram->setProgram(defaultFSh);

    for ( acd_uint i = 0; i < _vshOutputs.size(); ++i )
        _defaultVshProgram->setOutputWritten(i, false);

    for ( acd_uint i = 0; i < _fshInputs.size(); ++i )
        _defaultFshProgram->setInputRead(i, false);

    _defaultVshProgram->setOutputWritten(gpu3d::POSITION_ATTRIBUTE, true);
    _defaultVshProgram->setOutputWritten(3, true);

    _defaultVshProgram->setInputRead(gpu3d::POSITION_ATTRIBUTE, true);
    _defaultVshProgram->setInputRead(gpu3d::COLOR_ATTRIBUTE, true);

    _vsh = _defaultVshProgram;
    _fsh = _defaultFshProgram;

    _syncVertexShader();
    _syncFragmentShader();

    zValuePartialClear = 0;

    //  Allocate buffer with the quad positions.
    acd_float vertexPositions[4 * 4] =
    {
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

        
    vertexBufferPartialClear = createBuffer(4*16, (acd_ubyte*)vertexPositions);

    for (acd_uint i = 0; i < ACD_MAX_RENDER_TARGETS; i++)
        _currentRenderTarget[i] = NULL;
}

ACDDeviceImp::~ACDDeviceImp()
{
 
}

void ACDDeviceImp::setOptions(const ACD_CONFIG_OPTIONS& configOptions)
{
    panic("ACDDeviceImp", "setOptions", "Method not implemented yet");
}

void ACDDeviceImp::_syncRegisters()
{
    gpu3d::GPURegData data;

    if ( _indexedMode )
        data.uintVal = 1;
    else
        data.uintVal = 0;
    _driver->writeGPURegister(gpu3d::GPU_INDEX_MODE, 0, data); // Init INDEX MODE

    _translatePrimitive(_primitive, &data);
    _driver->writeGPURegister(gpu3d::GPU_PRIMITIVE, data); // Init primitive topology

    for ( acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; ++i ) {
        data.uintVal = _vaMap[i];
        _driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_MAP, i, data);
    }

    data.qfVal[0] =  float(_clearColorBuffer         & 0xff) / 255.0f;
    data.qfVal[1] =  float((_clearColorBuffer >> 8)  & 0xff) / 255.0f;
    data.qfVal[2] =  float((_clearColorBuffer >> 16) & 0xff) / 255.0f;
    data.qfVal[3] =  float((_clearColorBuffer >> 24) & 0xff) / 255.0f;
    _driver->writeGPURegister(gpu3d::GPU_COLOR_BUFFER_CLEAR, data);

    data.uintVal = static_cast<acd_uint>(static_cast<acd_float>(_zClearValue) * static_cast<acd_float>((1 << 24) - 1));
    _driver->writeGPURegister(gpu3d::GPU_Z_BUFFER_CLEAR, data);

    data.intVal = _stencilClearValue;
    _driver->writeGPURegister(gpu3d::GPU_STENCIL_BUFFER_CLEAR, data);

    data.booleanVal = _hzActivated;
    //_driver->writeGPURegister(gpu3d::GPU_HIERARCHICALZ, data);

    gpu3d::GPURegData bValue;

    for ( acd_uint i = 0; i < _vshOutputs.size(); ++i ) {
        bValue.booleanVal = _vshOutputs[i];
        _driver->writeGPURegister(gpu3d::GPU_VERTEX_OUTPUT_ATTRIBUTE, i, bValue);
        _vshOutputs[i].restart();
    }

    for ( acd_uint i = 0; i < _fshInputs.size(); ++i ) {
        bValue.booleanVal = _fshInputs[i];
        _driver->writeGPURegister(gpu3d::GPU_FRAGMENT_INPUT_ATTRIBUTES, i, bValue);
        _fshInputs[i].restart();
    }

    /*
    cout << "CDDeviceImp::_syncRegisters() -> "
            "Temporary initializing VERTEX_THREAD_RESOURCES & FRAGMENT_THREAD_RESOURCES statically to 5"  << endl;
    data.uintVal = 5;
    _driver->writeGPURegister(gpu3d::GPU_VERTEX_THREAD_RESOURCES, data);
    _driver->writeGPURegister(gpu3d::GPU_FRAGMENT_THREAD_RESOURCES, data);
    */

    data.uintVal = _vshResources;
    _driver->writeGPURegister(gpu3d::GPU_VERTEX_THREAD_RESOURCES, data);

    data.uintVal = _fshResources;
    _driver->writeGPURegister(gpu3d::GPU_FRAGMENT_THREAD_RESOURCES, data);

    // Note: If more registers must be initialized by the device, put the initialization code here
}

MemoryObjectAllocator& ACDDeviceImp::allocator() const
{
    return *_moa;
}

GPUDriver& ACDDeviceImp::driver() const
{
    return *_driver;
}

void ACDDeviceImp::setPrimitive(ACD_PRIMITIVE primitive)
{
    _primitive = primitive;
}

ACDBlendingStage& ACDDeviceImp::blending()
{
    return *_blending;
}

ACDRasterizationStage& ACDDeviceImp::rast()
{
    return *_rast;
}

ACDZStencilStage& ACDDeviceImp::zStencil()
{
    return *_zStencil;
}

ACDTexture2D* ACDDeviceImp::createTexture2D()
{
    return new ACDTexture2DImp();
}

ACDTexture3D* ACDDeviceImp::createTexture3D()
{
    return new ACDTexture3DImp();
}

ACDTextureCubeMap* ACDDeviceImp::createTextureCubeMap()
{
    return new ACDTextureCubeMapImp();
}

ACDBuffer* ACDDeviceImp::createBuffer(acd_uint size, const acd_ubyte* data)
{
    return new ACDBufferImp(size, data);
}

acd_bool ACDDeviceImp::destroy(ACDResource* resourcePtr)
{
    switch ( resourcePtr->getType() ) {
        case ACD_RESOURCE_BUFFER:
            {
                ACDBufferImp* bufImp = static_cast<ACDBufferImp*>(resourcePtr);
                _moa->deallocate(bufImp); // Deallocate from GPU memory if required
                //cout << "I'm going to delete " << bufImp << "\n";
                delete bufImp; // Release buffer interface
                return true;
            }
            break;
        default:
            panic("ACDDeviceImp", "destroy", "Destroy is only implemented for buffer resources only [sorry for the inconvenience]");
    }
    return false;
}

acd_bool ACDDeviceImp::destroy(ACDShaderProgram* shProgram)
{
    if ( shProgram == 0 )
        panic("ACDDevice", "destroy", "Trying to destroy a NULL shader program interface");

    if ( shProgram == _vsh )
        panic("ACDDevice", "destroy", "Destroying a shader program used as the currently bound vertex program");
    else if ( shProgram == _fsh )
        panic("ACDDevice", "destroy", "Destroying a shader program used as the currently bound fragment program");
    else if ( shProgram == _gsh )
        panic("ACDDevice", "destroy", "Destroying a shader program used as the currently bound geometry program");

    ACDShaderProgramImp* shProgramImp = static_cast<ACDShaderProgramImp*>(shProgram);

    _moa->deallocate(shProgramImp); // Deallocate from GPU memory if required

    delete shProgramImp; // Destroy the interface

    return true;
}



void ACDDeviceImp::setResolution(acd_uint width, acd_uint height)
{
    /*if ( _driver->isResolutionDefined() )
        panic("ACDDeviceImp", "setResolution", "Resolution is already defined");*/

    _driver->setResolution(width, height);

    acd_uint mdFront, mdBack, mdZStencil;

    _driver->initBuffers(&mdFront, &mdBack, &mdZStencil);

    bool multisampling;
    acd_uint samples;
    bool fp16color;

    _driver->getFrameBufferParameters(multisampling, samples, fp16color);

    ACD_FORMAT format;

    if (fp16color)
        format = ACD_FORMAT_RGBA16F;
    else
        format = ACD_FORMAT_ARGB_8888;

    //  Create the render targets for the default render buffers.
    _defaultFrontBuffer = new ACDRenderTargetImp(this, width, height, multisampling, samples, format, true, mdFront);
    _defaultBackBuffer = new ACDRenderTargetImp(this, width, height, multisampling, samples, format, true, mdBack);
    _defaultZStencilBuffer = new ACDRenderTargetImp(this, width, height, multisampling, samples, ACD_FORMAT_S8D24, true, mdZStencil);

    //  Set the default back buffer and z stencil buffer as the current render targets.
    _currentRenderTarget[0] = _defaultBackBuffer;
    _currentZStencilBuffer = _defaultZStencilBuffer;

    _currentRenderTarget[0].restart();
    _currentZStencilBuffer.restart();

    //  Set as using the default render buffers.
    _defaultRenderBuffers = true;
    _defaultZStencilBufferInUse = true;

    //  Create a buffer for saving the color block state data on render target switch.
    //
    //  NOTE:  Current implementation uses a fixed size, this should change to take into
    //         account the resolution and the GPU parameters.
    //
    _mdColorBufferSavedState = _driver->obtainMemory(512 * 1024);

    //  Set the gpu register that points to the color state buffer save area.
    _driver->writeGPUAddrRegister(gpu3d::GPU_COLOR_STATE_BUFFER_MEM_ADDR, 0, _mdColorBufferSavedState);

    //  Create a buffer for saving the z stencil block state data on z stencil buffer switch.
    //
    //  NOTE:  Current implementation uses a fixed size, this should change to take into
    //         account the resolution and the GPU parameters.
    //
    _mdZStencilBufferSavedState = _driver->obtainMemory(512 * 1024);

    //  Set the gpu register that points to the z stencil state buffer save area.
    _driver->writeGPUAddrRegister(gpu3d::GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR, 0, _mdZStencilBufferSavedState);

    //  Set the z stencil buffer as defined.
    _zStencil->setZStencilBufferDefined(true);
}

acd_bool ACDDeviceImp::getResolution(acd_uint& width, acd_uint& height)
{
    _driver->getResolution(width, height);
    return _driver->isResolutionDefined();
}

ACDStream& ACDDeviceImp::stream(acd_uint streamID)
{
    return *_stream[streamID];
}


void ACDDeviceImp::enableVertexAttribute(acd_uint vaIndex, acd_uint streamID)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( vaIndex >= gpu3d::MAX_VERTEX_ATTRIBUTES )
            panic("ACDDeviceImp", "enableVertexAttributes", "Vertex attribute index too high");

        if ( _usedStreams.count(streamID) != 0 ) {
            stringstream ss;
            ss << "Stream " << vaIndex << " is already enabled with another vertex attribute";
            panic("ACDDeviceImp", "enableVertexAttribute", ss.str().c_str());
        }
    )

    _vaMap[vaIndex] = streamID;
    _usedStreams.insert(streamID);
    GLOBALPROFILER_EXITREGION()    
}

void ACDDeviceImp::disableVertexAttribute(acd_uint vaIndex)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( vaIndex >= gpu3d::MAX_VERTEX_ATTRIBUTES )
            panic("ACDDeviceImp", "disableVertexAttributes", "Vertex attribute index too high");
    )

    acd_uint stream = _vaMap[vaIndex];

    if ( stream != gpu3d::ST_INACTIVE_ATTRIBUTE )  { // Mark this stream as free
        _usedStreams.erase(stream);
        _vaMap[vaIndex] = gpu3d::ST_INACTIVE_ATTRIBUTE;
    }
    GLOBALPROFILER_EXITREGION()    
}

void ACDDeviceImp::disableVertexAttributes()
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _usedStreams.clear();
    //_vaMap.assign(gpu3d::MAX_VERTEX_ATTRIBUTES, gpu3d::ST_INACTIVE_ATTRIBUTE);
    for(u32bit a = 0; a < _vaMap.size(); a++)
        _vaMap[a] = gpu3d::ST_INACTIVE_ATTRIBUTE;
    GLOBALPROFILER_EXITREGION()    
}

void ACDDeviceImp::_syncStreamerState()
{

#ifdef ACD_DUMP_STREAMS
    acd_ubyte filename[30];
    sprintf((char*)filename, "ACDDumpStream.txt");
    remove((char*)filename);
#endif

    // Create a sorted map
    gpu3d::GPURegData data;
    map<acd_uint,acd_uint> m;
    for ( acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++ ) {
        m.insert(make_pair(i, _vaMap[i]));
    }

    // Use map to sort by stream unit (to match all legacy code)
    map<acd_uint, acd_uint>::iterator it = m.begin();
    while( it != m.end()) {
        acd_uint stream = it->second;
        acd_uint attrib = it->first;
		if ( _vaMap[attrib].changed() ) {
			data.uintVal = _vaMap[attrib];
			_driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_MAP, attrib, data);
			_vaMap[attrib].restart();
		}

		if ( _vaMap[attrib] != gpu3d::ST_INACTIVE_ATTRIBUTE) // Sync stream if the vertex attribute is enabled
			_stream[_vaMap[attrib]]->sync();
        it++ ;
    }
}

void ACDDeviceImp::_syncSamplerState()
{

#ifdef ACD_DUMP_SAMPLERS
    for (acd_uint i = 0; i < gpu3d::MAX_TEXTURES; i++)
        for (acd_uint j = 0; j < 6; j++)
            for (acd_uint k = 0; k < gpu3d::MAX_TEXTURE_SIZE; k++)
            {
                    acd_ubyte filename[30];
                    sprintf((char*)filename, "Sampler%d_Face%d_Mipmap%d.ppm", i,j,k);
                    remove((char*)filename);
            }
#endif

    ACDShaderProgramImp* fProgramImp = _fsh;

    for ( acd_uint i = 0; i < gpu3d::MAX_TEXTURES; ++i )
    {
        //if (!fProgramImp->getTextureUnitsUsage(i)) // If the texture is not used in the shader, there is no need to sync it
        //    _sampler[i]->setEnabled(false);

        _sampler[i]->sync();
    }
}


acd_uint ACDDeviceImp::availableStreams() const
{
    // the stream buffer (MAX_STREAM_BUFFERS-1) is reserved to stream indexes
    return _MAX_STREAMS;
}

acd_uint ACDDeviceImp::availableVertexAttributes() const
{
    return gpu3d::MAX_VERTEX_ATTRIBUTES;
}

ACDSampler& ACDDeviceImp::sampler(acd_uint samplerID)
{
    //panic("ACDDeviceImp", "sampler", "Samplers not available yet");
    return *_sampler[samplerID];
}

void ACDDeviceImp::setIndexBuffer(ACDBuffer* ib, acd_uint offset, ACD_STREAM_DATA indicesType)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    // Configure stream of indices
    ACD_STREAM_DESC desc;

    desc.components = 1;
    desc.frequency = 0;
    desc.offset = offset;
    desc.type = indicesType;
    desc.stride = 0;

    _indexStream->set(ib, desc);

    GLOBALPROFILER_EXITREGION()    
}

void ACDDeviceImp::_syncStreamingMode(acd_uint start, acd_uint count, acd_uint instances, acd_uint, acd_uint)
{
    // min & max params ignored currently

    gpu3d::GPURegData data;

    // Update GPU registers with the start vertex/index and the vertex/index count

    _streamStart = start;
    if(_streamStart.changed())
    {
        data.uintVal = _streamStart;
        _driver->writeGPURegister(gpu3d::GPU_STREAM_START, 0, data);
        _streamStart.restart();
    }

    _streamCount = count;
    if(_streamCount.changed())
    {
        data.uintVal = _streamCount;
        _driver->writeGPURegister(gpu3d::GPU_STREAM_COUNT, 0, data);
        _streamCount.restart();
    }

    _streamInstances = instances;
    if(_streamInstances.changed())
    {
        data.uintVal = _streamInstances;
        _driver->writeGPURegister(gpu3d::GPU_STREAM_INSTANCES, 0, data);
        _streamInstances.restart();
    }

    if ( _indexedMode ) {
        if ( _indexedMode.changed() ) {
            // Changing from sequential to indexed mode (enable GPU indexed mode)
            data.uintVal = 1;
            _driver->writeGPURegister(gpu3d::GPU_INDEX_MODE, 0, data);
            _indexedMode.restart();
        }
        // Synchronize the stream of indices
        _indexStream->sync();
    }
    else {
        if ( _indexedMode.changed() ) {
            // Changing from indexed to sequential mode (disable GPU indexed mode)
            data.uintVal = 0;
            _driver->writeGPURegister(gpu3d::GPU_INDEX_MODE, 0, data);
            _indexedMode.restart();
        }
        // else (nothing to do)
    }
}

ACDRenderTarget* ACDDeviceImp::createRenderTarget( ACDTexture* resource, const ACD_RT_DIMENSION rtdimension, ACD_CUBEMAP_FACE face, acd_uint mipmap )
{
    return new ACDRenderTargetImp (this, resource, rtdimension, face, mipmap);
}


acd_bool ACDDeviceImp::setRenderTargets( acd_uint numRenderTargets,
                                         const ACDRenderTarget* const * renderTargets,
                                         const ACDZStencilTarget* zStencilTarget )
{
    cout << "ACDDeviceImp::setRenderTargets(" << numRenderTargets << ",RTargets**,ZStencilTargets*) -> Not implemented yet" << endl;
    return true;
}

acd_bool ACDDeviceImp::setRenderTarget(acd_uint indexRenderTarget, ACDRenderTarget *renderTarget)
{
    if (renderTarget == NULL)
    {
        //  Disable drawing.
        _blending->disableColorWrite(indexRenderTarget);
        _currentRenderTarget[indexRenderTarget] = NULL;
        return true;
    }

    //  Check the render target format.
    if ((renderTarget->getFormat() != ACD_FORMAT_ARGB_8888) &&
        (renderTarget->getFormat() != ACD_FORMAT_XRGB_8888) &&
        (renderTarget->getFormat() != ACD_FORMAT_RG16F) && 
        (renderTarget->getFormat() != ACD_FORMAT_R32F) &&
        (renderTarget->getFormat() != ACD_FORMAT_RGBA16F) &&
        (renderTarget->getFormat() != ACD_FORMAT_ABGR_161616) &&
        (renderTarget->getFormat() != ACD_FORMAT_S8D24))
    {
        panic("ACDDeviceImp","setRenderTarget","Format not supported");
        return false;
    }

    ACDRenderTargetImp *rt = reinterpret_cast<ACDRenderTargetImp *> (renderTarget);

    //  Check if setting the already defined render target.
    if (rt == _currentRenderTarget[indexRenderTarget])
        return true;

    if ((_currentRenderTarget[indexRenderTarget] == NULL) && (rt != NULL))
    {
        // Enable drawing.
        _blending->enableColorWrite(indexRenderTarget);
    }
    
    //  Check if using the default render buffers
    if (_defaultRenderBuffers)
    {
        //  Check if changing to the front buffer (swap).
        if (rt == _defaultFrontBuffer)
        {
            _currentRenderTarget[indexRenderTarget] = rt;
            _defaultFrontBuffer = _defaultBackBuffer;
            _defaultBackBuffer = rt;

            return true;
        }

        // Ensures any pending drawing is done on previous RT.
        _driver->sendCommand(gpu3d::GPU_FLUSHCOLOR);

        //  Save default render buffer state.
        _driver->sendCommand(gpu3d::GPU_SAVE_COLOR_STATE);

        _currentRenderTarget[indexRenderTarget] = rt;

        _defaultRenderBuffers = false;
        
        //  Reset viewport.
        _rast->setViewport(0, 0, rt->getWidth(), rt->getHeight());

        //  Reset scissor.
        _rast->enableScissor(false);
        _rast->setScissor(0, 0, rt->getWidth(), rt->getHeight());

        return true;
    }
    else
    {
        // Ensures any pending drawing is done on previous RT.
        _driver->sendCommand(gpu3d::GPU_FLUSHCOLOR);

        //  Check if changing to the default render buffers.
        if (rt == _defaultFrontBuffer)
        {
            _defaultFrontBuffer = _defaultBackBuffer;
            _defaultBackBuffer = rt;
            _currentRenderTarget[indexRenderTarget] = rt;

            _defaultRenderBuffers = true;

            //  Restore the color block state data from the save area.
            _driver->sendCommand(gpu3d::GPU_RESTORE_COLOR_STATE);

            //  Reset viewport.
            _rast->setViewport(0, 0, rt->getWidth(), rt->getHeight());

            //  Reset scissor.
            _rast->enableScissor(false);
            _rast->setScissor(0, 0, rt->getWidth(), rt->getHeight());

            return true;
        }

        if (rt == _defaultBackBuffer)
        {
            _currentRenderTarget[indexRenderTarget] = rt;

            _defaultRenderBuffers = true;

            //  Restore the color block state data from the save area.
            _driver->sendCommand(gpu3d::GPU_RESTORE_COLOR_STATE);

            //  Reset viewport.
            _rast->setViewport(0, 0, rt->getWidth(), rt->getHeight());

            //  Reset scissor.
            _rast->enableScissor(false);
            _rast->setScissor(0, 0, rt->getWidth(), rt->getHeight());

            return true;
        }

        _currentRenderTarget[indexRenderTarget] = rt;

        //  Reset viewport.
        if (indexRenderTarget == 0)
            _rast->setViewport(0, 0, rt->getWidth(), rt->getHeight());

        //  Reset scissor.
        _rast->enableScissor(false);
        _rast->setScissor(0, 0, rt->getWidth(), rt->getHeight());

        return true;
    }
}

bool ACDDeviceImp::setZStencilBuffer(ACDRenderTarget *zstencilBuffer)
{

//ACDRenderTargetImp *currentZSB = _currentZStencilBuffer;
//ACDRenderTargetImp *defaultZSB = _defaultZStencilBuffer;
//printf("ACDDeviceImp::setZStencilBuffer => >> Setting zStencilBuffer = %p | currentZStencilBuffer = %p | defaultZStencilBuffer = %p | defaultZStencilBufferInUse =%s \n",
//    zstencilBuffer, currentZSB, defaultZSB, _defaultZStencilBufferInUse ? "T" : "F");

    if (zstencilBuffer == NULL)
    {
        // Disable depth stencil operation
        _currentZStencilBuffer = NULL;
        return true;
    }

    //  Check the z stencil buffer format.
    if (zstencilBuffer->getFormat() != ACD_FORMAT_S8D24)
    {
        return false;
    }

    ACDRenderTargetImp *zstencil = reinterpret_cast<ACDRenderTargetImp *> (zstencilBuffer);

    //  Check if setting the already defined render target.
    if (zstencil == _currentZStencilBuffer)
        return true;
    
    if (_defaultZStencilBufferInUse)
    {   
        //  Check if setting the default z stencil buffer.  This happens with the default -> NULL -> default transition.
        if (zstencil == _defaultZStencilBuffer)
        {
            _currentZStencilBuffer = zstencil;
            return true;
        }
        
        // Ensures any pending drawing is done on previous RT.
        _driver->sendCommand(gpu3d::GPU_FLUSHZSTENCIL);

        //  Save default render buffer state.
        _driver->sendCommand(gpu3d::GPU_SAVE_ZSTENCIL_STATE);

        _defaultZStencilBufferInUse = false;
    }
    else
    {
        // Ensures any pending drawing is done on previous RT.
        _driver->sendCommand(gpu3d::GPU_FLUSHZSTENCIL);

        if (zstencil == _defaultZStencilBuffer)
        {
            //  Restore the color block state data from the save area.
            _driver->sendCommand(gpu3d::GPU_RESTORE_ZSTENCIL_STATE);

            _defaultZStencilBufferInUse = true;
        }
    }
    
    _currentZStencilBuffer = zstencil;

    return true;
}

ACDRenderTarget *ACDDeviceImp::getRenderTarget(acd_uint indexRenderTarget)
{
    return _currentRenderTarget[indexRenderTarget];
}

ACDRenderTarget *ACDDeviceImp::getZStencilBuffer()
{
    return _currentZStencilBuffer;
}

bool ACDDeviceImp::_multipleRenderTargetsSet() {

    for (acd_uint i = 1; i < ACD_MAX_RENDER_TARGETS; i++) {

        if (_currentRenderTarget[i] != NULL)
            return true;

    }

    return false;

}

void ACDDeviceImp::_syncRenderBuffers()
{

    gpu3d::GPURegData data;
    ACDRenderTargetImp *zstencil = _currentZStencilBuffer;

    bool resolutionSet = false;
    u32bit resWidth;
    u32bit resHeight;
    
    //  Check if the z stencil buffer changed and was not set to null.
    if (_currentZStencilBuffer.changed() && (zstencil != NULL))
    {
        //  Get the initial resolution from the Z Stencil buffer dimensions if z or stencil tests are currently enabled.   
        resolutionSet = zStencil().isStencilEnabled() || zStencil().isZEnabled();
        resWidth = zstencil->getWidth();
        resHeight = zstencil->getHeight();
    }
        
    for (acd_uint i = 0; i < ACD_MAX_RENDER_TARGETS; i++) {

        ACDRenderTargetImp *rt = _currentRenderTarget[i];

        //  Check that the render target and the z stencil buffer have the same parameters.
        if (_currentRenderTarget[i].changed() || _currentZStencilBuffer.changed())
        {
            //  Check if resolution was already set.
            if (resolutionSet)
            {
                //  Check if the render target is active.
                if (blending().getColorEnable(i))
                {
                    //  Check if the render target dimension matches the resolution set.
                    /*if ((rt->getWidth() != resWidth) || (rt->getHeight() != resHeight))
                    {
                        panic("ACDDeviceImp", "_syncRenderBuffers", "Render target resolution doesn't match current resolution.");
                    }*/
                }
            }
            else
            {
                //  Check if the render target is set.
                if (rt != NULL)
                {
                    //  Set the resolution from the render target dimensions..
                    resolutionSet = true;
                    resWidth = rt->getWidth();
                    resHeight = rt->getHeight();
                }
            }
        }

        if (_currentRenderTarget[i].changed() && rt != NULL)
        {
            //  Set the resolution of the new render target.
            //_driver->setResolution(rt->getWidth(), rt->getHeight());

            data.booleanVal = true;
            _driver->writeGPURegister(gpu3d::GPU_RENDER_TARGET_ENABLE, i, data);

            //  Set compression.
            data.booleanVal = rt->allowsCompression() && !_multipleRenderTargetsSet();

            _driver->writeGPURegister(gpu3d::GPU_COLOR_COMPRESSION, 0, data);

            //  Set render target format.
            switch (rt->getFormat())
            {
                case ACD_FORMAT_XRGB_8888:
                case ACD_FORMAT_ARGB_8888:
                    data.txFormat = gpu3d::GPU_RGBA8888;
                    break;
                case ACD_FORMAT_RG16F:
                    data.txFormat = gpu3d::GPU_RG16F;
                    break;
                case ACD_FORMAT_R32F:
                    data.txFormat = gpu3d::GPU_R32F;
                    break;                
                case ACD_FORMAT_ABGR_161616:
                    data.txFormat = gpu3d::GPU_RGBA16;
                    break;
                case ACD_FORMAT_RGBA16F:
                    data.txFormat = gpu3d::GPU_RGBA16F;
                    break;
                case ACD_FORMAT_S8D24:
                    data.txFormat = gpu3d::GPU_DEPTH_COMPONENT24;
                    break;
                default:
                    panic("ACDDeviceImp", "_syncRenderBuffers", "Render target format unknown.");
                    break;
            }
            //_driver->writeGPURegister(gpu3d::GPU_COLOR_BUFFER_FORMAT, 0, data);
            _driver->writeGPURegister(gpu3d::GPU_RENDER_TARGET_FORMAT, i, data);

            //  Set multisampling parameters
            //  TODO

            //  Set render target address.
            if (_defaultRenderBuffers)
            {
                ACDRenderTargetImp *rtTemp = _defaultFrontBuffer;

                if (i == 0)
                    _driver->writeGPUAddrRegister(gpu3d::GPU_FRONTBUFFER_ADDR, 0, rtTemp->md());

                rtTemp = _defaultBackBuffer;
                //_driver->writeGPUAddrRegister(gpu3d::GPU_BACKBUFFER_ADDR , 0, rtTemp->md());
                _driver->writeGPUAddrRegister(gpu3d::GPU_RENDER_TARGET_ADDRESS , i, rtTemp->md());

            }
            else
            {
                // This cleans block state bits in the color caches
                _driver->sendCommand(gpu3d::GPU_RESET_COLOR_STATE);

                if (i == 0)
                    _driver->writeGPUAddrRegister(gpu3d::GPU_FRONTBUFFER_ADDR, 0, rt->md());

                //_driver->writeGPUAddrRegister(gpu3d::GPU_BACKBUFFER_ADDR , 0, rt->md());
                _driver->writeGPUAddrRegister(gpu3d::GPU_RENDER_TARGET_ADDRESS , i, rt->md());
            }

            _currentRenderTarget[i].restart();
        }
        else if (_currentRenderTarget[i].changed() && rt == NULL && i != 0){

            data.booleanVal = false;
            _driver->writeGPURegister(gpu3d::GPU_RENDER_TARGET_ENABLE, i, data);

        }

    }

    if (_currentZStencilBuffer.changed())
    {
        //  Check if the depth stencil buffer is not defined.
        if (zstencil == NULL)
        {
            //  Sets the z and stencil buffer as not defined.  Disables z and stencil test.
            _zStencil->setZStencilBufferDefined(false);
        }
        else
        {
            if (!_defaultZStencilBufferInUse)
            {
                // This cleans block state bits in the color caches
                _driver->sendCommand(gpu3d::GPU_RESET_ZSTENCIL_STATE);
            }

            //  Set the z stencil buffer address.
            _driver->writeGPUAddrRegister(gpu3d::GPU_ZSTENCILBUFFER_ADDR, 0, zstencil->md());

            //  Sets the z and stencil buffer as defined.
            _zStencil->setZStencilBufferDefined(true);

            //  Set compression.
            data.booleanVal = zstencil->allowsCompression();
            _driver->writeGPURegister(gpu3d::GPU_ZSTENCIL_COMPRESSION, 0, data);
        }

        _currentZStencilBuffer.restart();
    }

    //  Check if the resolution was set.
    if (resolutionSet)
    {
        //  Set the new resolution.
        _driver->setResolution(resWidth, resHeight);
    }        
    else
    {
        //  No resolution was set.
    }
    
    //  Check if color linear to sRGB conversion on color write flag has changed.
    if (_colorSRGBWrite.changed())
    {
        //  Set convert color from linear to sRGB space on color write.
        data.booleanVal = _colorSRGBWrite;
        _driver->writeGPURegister(gpu3d::GPU_COLOR_SRGB_WRITE, 0, data);
        _colorSRGBWrite.restart();
    }
}

void ACDDeviceImp::draw(acd_uint start, acd_uint count, acd_uint instances)
{
    _indexedMode = false;

    if ((_primitive == ACD_TRIANGLES) || (_primitive == ACD_TRIANGLE_FAN) || (_primitive == ACD_TRIANGLE_STRIP) ||
        (_primitive == ACD_QUADS) || (_primitive == ACD_QUAD_STRIP))
    {
        _draw(start, count, 0, 0, 0, instances);
    }
    else
    {
        printf("ACDDeviceImp::draw() => WARNING : Primitive ");
        switch(_primitive)
        {
            case ACD_POINTS : printf("ACD_POINTS"); break;
            case ACD_LINES :  printf("ACD_LINES"); break;
            case ACD_LINE_LOOP : printf("ACD_LINE_LOOP"); break;
            case ACD_LINE_STRIP : printf("ACD_LINE_STRIP"); break;
            default :
                panic("ACDDeviceImp", "draw", "Undefined draw primitive.");
                break;
        }

        printf(" not implemented.  Ignoring draw call.\n");
        
        //panic("ACDDeviceImp", "draw", "Unsupported draw primitive.");
    }
}

void ACDDeviceImp::drawIndexed(acd_uint startIndex, acd_uint indexCount, acd_uint minIndex, acd_uint maxIndex,
                               acd_int baseVertexIndex, acd_uint instances)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    if ((_primitive == ACD_TRIANGLES) || (_primitive == ACD_TRIANGLE_FAN) || (_primitive == ACD_TRIANGLE_STRIP) ||
        (_primitive == ACD_QUADS) || (_primitive == ACD_QUAD_STRIP))
    {
        GLOBALPROFILER_ENTERREGION("ACD", "", "")
        
        _indexedMode = true;
        _draw(startIndex, indexCount, minIndex, maxIndex, baseVertexIndex, instances);

        GLOBALPROFILER_EXITREGION()
    }
    else
    {
        printf("ACDDeviceImp::drawIndexed() => WARNING : Primitive ");
        switch(_primitive)
        {
            case ACD_POINTS : printf("ACD_POINTS"); break;
            case ACD_LINES :  printf("ACD_LINES"); break;
            case ACD_LINE_LOOP : printf("ACD_LINE_LOOP"); break;
            case ACD_LINE_STRIP : printf("ACD_LINE_STRIP"); break;
            default :
                panic("ACDDeviceImp", "drawIndexed", "Undefined draw primitive.");
                break;
        }

        printf(" not implemented.  Ignoring draw call.\n");
        
        //panic("ACDDeviceImp", "drawIndexed", "Unsupported draw primitive.");
    }
}

void ACDDeviceImp::_translatePrimitive(ACD_PRIMITIVE primitive, gpu3d::GPURegData* data)
{
    switch ( primitive ) {
        case ACD_TRIANGLES:
            data->primitive = gpu3d::TRIANGLE;
            break;
        case ACD_TRIANGLE_STRIP:
            data->primitive = gpu3d::TRIANGLE_STRIP;
            break;
        case ACD_TRIANGLE_FAN:
            data->primitive = gpu3d::TRIANGLE_FAN;
            break;
        case ACD_QUADS:
            data->primitive = gpu3d::QUAD;
            break;
        case ACD_QUAD_STRIP:
            data->primitive = gpu3d::QUAD_STRIP;
            break;
        case ACD_LINES:
            data->primitive = gpu3d::LINE;
            break;
        case ACD_LINE_STRIP:
            data->primitive = gpu3d::LINE_STRIP;
            break;
        case ACD_LINE_LOOP:
            data->primitive = gpu3d::LINE_FAN;
            break;
        case ACD_POINTS:
            data->primitive = gpu3d::POINT;
            break;
        default:
            panic("ACDDeviceImp", "_translatePrimitive", "ACD Unsupported primitive");
    }
}

void ACDDeviceImp::_dump(const acd_char* file, acd_enum flags)
{
    // cout << "ACDDeviceImp::DBG_dump(" << file << ", 0x" << hex << flags << dec << ")" << endl;
    ofstream out;

    BoolPrint boolPrint;

    out.open(file);

    if ( !out.is_open() )
        panic("ACDDeviceImp", "DBG_dump", "Dump failed (output file could not be opened)");

    out << "####################" << endl;
    out << "## ACD STATE DUMP ##" << endl;
    out << "####################" << endl;
    out << "# FRAME: " << _currentFrame << endl;
    out << "# BATCH: " << _currentBatch << endl << endl;

    // Output internal data

    out << stateItemString(_primitive,"PRIMITIVE", false, &primitivePrint);
    out << stateItemString(_indexedMode,"INDEXED_MODE", false, &boolPrint);

    if (_indexedMode)
        out << "INDEX_STREAM:" << _indexStream->getInternalState();

    std::vector<StateItem<acd_uint> >::const_iterator iter = _vaMap.begin();

    acd_uint i = 0;

    while ( iter != _vaMap.end() )
    {
        out << "VERTEX_ATTR" << i << "_STREAM";

        if ((*iter) != gpu3d::ST_INACTIVE_ATTRIBUTE)
            out << " = " << (*iter);
        else
            out << " = INACTIVE";

        if ( (*iter).changed() )
        {
            out << " NotSync = ";
            if ((*iter).initial() != gpu3d::ST_INACTIVE_ATTRIBUTE)
            {
                out << (*iter).initial() << "\n";
            }
            else
                out << "INACTIVE\n";
        }
        else
            out << " Sync\n";

        if ((*iter) != gpu3d::ST_INACTIVE_ATTRIBUTE && (*iter) < _MAX_STREAMS)
        {
            out << _stream[(*iter)]->getInternalState();
        }
        iter++;
        i++;
    }

    // Output stages states
    out << _rast->getInternalState();
    out << _zStencil->getInternalState();
    out << _blending->getInternalState();

    out << stateItemString(_clearColorBuffer, "CLEAR_COLOR_BUFFER",false);
    out << stateItemString(_zClearValue,"Z_CLEAR_VALUE",false);
    out << stateItemString(_stencilClearValue,"STENCIL_CLEAR_VALUE",false);

    out << stateItemString(_hzActivated,"HZ_ACTIVE",false, &boolPrint);

    out << "GEOMETRY_SHADER_DEFINED = ";
    if (_gsh)
    {
        out << "TRUE" << endl;
        ACDShaderProgramImp* gProgramImp = _gsh;
        gProgramImp->printASM(out);
        gProgramImp->printConstants(out);
    }
    else
        out << "FALSE" << endl;

    out << "VERTEX_SHADER_DEFINED = ";
    if (_vsh)
    {
        out << "TRUE" << endl;
        ACDShaderProgramImp* vProgramImp = _vsh;
        vProgramImp->printASM(out);
        vProgramImp->printConstants(out);

        const acd_uint vshOutputsCount = _vshOutputs.size();
        for ( acd_uint i = 0; i < vshOutputsCount; ++i )
        {
            out << "VERTEX_SHADER_OUTPUT" << i << "_WRITTEN";
            out << stateItemString(_vshOutputs[i],"",false,&boolPrint);
        }

        out << stateItemString(_vshResources,"VERTEX_SHADER_THREAD_RESOURCES",false);
    }
    else
        out << "FALSE" << endl;


    out << "FRAGMENT_SHADER_DEFINED = ";
    if (_vsh)
    {
        out << "TRUE" << endl;
        ACDShaderProgramImp* fProgramImp = _fsh;
        fProgramImp->printASM(out);
        fProgramImp->printConstants(out);

        const acd_uint fshOutputsCount = _fshInputs.size();
        for ( acd_uint i = 0; i < fshOutputsCount; ++i )
        {
            out << "FRAGMENT_SHADER_INPUT" << i << "_WRITTEN";
            out << stateItemString(_fshInputs[i],"",false,&boolPrint);
        }

        out << stateItemString(_fshResources,"FRAGMENT_SHADER_THREAD_RESOURCES",false);
    }
    else
        out << "FALSE" << endl;


    for ( acd_uint i = 0; i < gpu3d::MAX_TEXTURES; ++i )
        out << _sampler[i]->getInternalState();

    out.close();
}

void ACDDeviceImp::_check_deferred_dump()
{
    if (!_nextDumpEvent.valid)
        return;

    if (_currentFrame > _nextDumpEvent.frame)
        panic("ACDDeviceImp","_check_deferred_dump","A deferred dump was skipped and this shouldnt happen");

    if (_currentFrame == _nextDumpEvent.frame)
    {
        if (_currentBatch > _nextDumpEvent.batch)
            panic("ACDDeviceImp","_check_deferred_dump","A deferred dump was skipped and this shouldnt happen");

        if (_currentBatch == _nextDumpEvent.batch)
        {
            // There is an event for this time.

            // Dump the state.
            _dump(_nextDumpEvent.fileName.c_str(), _nextDumpEvent.flags);

            // Remove the event from list
            _dumpEventList.remove(_nextDumpEvent);

            // Get the next dump event in time

            // First, get the frame and batch of the closest event.
            acd_uint minFrame, minBatch;
            minFrame = minBatch = acd_uint(-1);

            std::list<DumpEventInfo>::const_iterator iter = _dumpEventList.begin();

            for (; iter != _dumpEventList.end(); iter++)
            {
                if ((*iter).frame <= minFrame)
                {
                    minFrame = (*iter).frame;
                    if ((*iter).batch < minBatch)
                        minBatch = (*iter).batch;
                }
            };

            // Now, find the closest event struct and copy it into the
            // _nextDumpEvent;

            iter = _dumpEventList.begin();

            acd_bool found = false;

            while ( iter != _dumpEventList.end() && !found)
            {
                if ((*iter).frame == minFrame)
                    if ((*iter).batch == minBatch)
                    {
                        _nextDumpEvent = (*iter);
                        found = true;
                    }
                iter++;
            }

            if (!found)
                _nextDumpEvent.valid = false;

        }
    }
};

void ACDDeviceImp::_addBaseVertexIndex(acd_uint baseVertexIndex, acd_uint start, acd_uint count)
{

    /*if ( _vaMap[31] != gpu3d::ST_INACTIVE_ATTRIBUTE)
        panic("ACDDeviceImp", "_addBaseVertexIndex", "Trying to perform a draw indexed call without any stream of indexes setted");
    else
    {*/

        ACDBuffer* original = _indexStream->getBuffer(); // Stream 31 is where Indexed Stream is stored by default

        acd_ubyte* originalUbyte = (acd_ubyte* )original->getData();
        acd_ushort* originalUshort = (acd_ushort*) originalUbyte;
        acd_uint* originalUint = (acd_uint*) originalUbyte;
        acd_float* originalFloat = (acd_float*) originalUbyte;

        ACDBuffer* copy = createBuffer(0,0);

        acd_uint last = count + start;

        switch(_indexStream->getType())
        {
            case ACD_SD_UINT8:
            case ACD_SD_INV_UINT8:
                for(int i = start; i < last; i++)
                {
                    /*acd_ubyte newValue = originalUbyte[i] + baseVertexIndex;
                    copy->pushData(&newValue, sizeof(acd_ubyte));*/
                    acd_uint newValue = (acd_uint)(originalUbyte[i]) + baseVertexIndex;
                    copy->pushData(&newValue, sizeof(acd_uint));
                }
                _indexStream->setType(ACD_SD_UINT32);
                break;

            case ACD_SD_UINT16:
            case ACD_SD_INV_UINT16:
                for(int i = start; i < last; i++)
                {
                    /*acd_ushort newValue = originalUshort[i] + baseVertexIndex;
                    copy->pushData(&newValue, sizeof(acd_ushort));*/
                    acd_uint newValue = (acd_uint)(originalUshort[i]) + baseVertexIndex;
                    copy->pushData(&newValue, sizeof(acd_uint));
                }
                _indexStream->setType(ACD_SD_UINT32);
                break;

            case ACD_SD_UINT32:
            case ACD_SD_INV_UINT32:
                for(int i = start; i < last; i++)
                {
                    acd_uint newValue = originalUint[i] + baseVertexIndex;
                    copy->pushData(&newValue, sizeof(acd_uint));
                }
                break;

            case ACD_SD_FLOAT32:
            case ACD_SD_INV_FLOAT32:
                for(int i = start; i < last; i++)
                {
                    acd_float newValue = originalFloat[i] + baseVertexIndex;
                    copy->pushData(&newValue, sizeof(acd_float));
                }
                break;

            default:
                panic("ACDDeviceImp", "_addBaseVertexIndex", "Buffer content type not available");
        }

        _indexStream->setBuffer(copy);
    //}

}

void ACDDeviceImp::_draw(acd_uint start, acd_uint count, acd_uint min, acd_uint max, acd_uint baseVertexIndex, acd_uint instances)
{
    ///////////////////////////////////////////////////////////////
    /// Synchronize render buffers                              ///
    ///////////////////////////////////////////////////////////////
    _syncRenderBuffers();

    ///////////////////////////////////////////////////////////////
    /// Synchronize stages Rasterization, ZStencil and Blending ///
    ///////////////////////////////////////////////////////////////
    _rast->sync();
    _zStencil->sync();
    _blending->sync();

    //////////////////////////////////
    /// Synchronize primitive mode ///
    //////////////////////////////////
    if ( _primitive.changed() )
    {
        gpu3d::GPURegData data;
        _translatePrimitive(_primitive, &data);
        _driver->writeGPURegister(gpu3d::GPU_PRIMITIVE, data);
        _primitive.restart();
    }

    //////////////////////////////////
    /// Synchronize bound textures ///
    //////////////////////////////////
    _syncSamplerState();

    ////////////////////
    /// Sync shaders ///
    ////////////////////
    _syncVertexShader();
    _syncFragmentShader();

    ////////////////////////////////////////////////////////////////////////////////
    /// Enable or disable GPU Hierarchical Z depending on current ZStencil state ///
    ////////////////////////////////////////////////////////////////////////////////
    _syncHZRegister();

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// If a baseVertexIndex is setted, baseVertexIndex is added to each Index of the Indexed Stream ///
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    if (_indexedMode && baseVertexIndex != 0) {
        _addBaseVertexIndex(baseVertexIndex, start, count);
        start = 0;
    }

    ///////////////////////////////////////////////////////////////////////////////
    /// Synchronize streams state ((buffers attached automatically synchronized ///
    ///////////////////////////////////////////////////////////////////////////////
    _syncStreamerState();

    //////////////////////////////////
    /// Synchronize streaming mode ///
    //////////////////////////////////
    _syncStreamingMode(start, count, instances, min, max);

    //////////////////////////////////////////////////////////////////////////////
    /// Enable or disable VERTEX_OUTPUT_ATTRIBUTES & FRAGMENT_INPUT_ATTRIBUTES ///
    //////////////////////////////////////////////////////////////////////////////
    gpu3d::GPURegData bValue;

    const acd_uint vshOutputsCount = _vshOutputs.size();
    for ( acd_uint i = 0; i < vshOutputsCount; ++i ) 
    {
        if ( _vshOutputs[i].changed() ) 
        {
            bValue.booleanVal = _vshOutputs[i];
            _driver->writeGPURegister(gpu3d::GPU_VERTEX_OUTPUT_ATTRIBUTE, i, bValue);
            _vshOutputs[i].restart();
        }
    }

    acd_bool oneFragmentInput = false;

    const acd_uint fshOutputsCount = _fshInputs.size();
    for ( acd_uint i = 0; i < fshOutputsCount; ++i ) 
    {
        if ( _fshInputs[i].changed() ) 
        {
            bValue.booleanVal = _fshInputs[i];
            _driver->writeGPURegister(gpu3d::GPU_FRAGMENT_INPUT_ATTRIBUTES, i, bValue);
            _fshInputs[i].restart();
        }

        oneFragmentInput = oneFragmentInput || _fshInputs[i];
    }

    if ( !oneFragmentInput )
    {
        _fshInputs[gpu3d::COLOR_ATTRIBUTE] = true;
        bValue.booleanVal = true;
        _driver->writeGPURegister(gpu3d::GPU_FRAGMENT_INPUT_ATTRIBUTES, gpu3d::COLOR_ATTRIBUTE, bValue);
        _fshInputs[gpu3d::COLOR_ATTRIBUTE].restart();
    }

    // EarlyZ can be actived if Z is not modified by the fragment shader
    // If fragment depth is modified by the fragment program disable Early Z
    ACDShaderProgramImp* fsh = _fsh;

    if (fsh->getOutputWritten(0) || fsh->getKillInstructions())
    {
        _earlyZ = false;
        
        if (_earlyZ.changed())
        {
            bValue.booleanVal = false;
            _driver->writeGPURegister(gpu3d::GPU_EARLYZ, bValue);
            _alphaTest.restart();
            _earlyZ.restart();
        }
    }
    else
    {
        _earlyZ = true;
        if (_earlyZ.changed())
        {
            bValue.booleanVal = true;
            _driver->writeGPURegister(gpu3d::GPU_EARLYZ, bValue);
            _alphaTest.restart();
            _earlyZ.restart();
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    ///                 Synchronize Shader Thread Resources                    ///
    //////////////////////////////////////////////////////////////////////////////

    gpu3d::GPURegData data;

    if ( _vshResources.changed() )
    {
        data.uintVal = _vshResources;
        _driver->writeGPURegister(gpu3d::GPU_VERTEX_THREAD_RESOURCES, data);
        _vshResources.restart();
    }

    if ( _fshResources.changed() )
    {
        data.uintVal = _fshResources;
        _driver->writeGPURegister(gpu3d::GPU_FRAGMENT_THREAD_RESOURCES, data);
        _fshResources.restart();
    }

    // Check if deferred ACD state dump is required
    _check_deferred_dump();

    ////////////
    /// DRAW ///
    ////////////
    _driver->sendCommand( gpu3d::GPU_DRAW );

    _moa->realeaseLockedMemoryRegions();

    _currentBatch++;
}

void ACDDeviceImp::updateResource( const ACDResource* destResource,
                                   acd_uint destSubResource,
                                   const ACD_BOX* destRegion,
                                   const acd_ubyte* srcData,
                                   acd_uint srcRowPitch,
                                   acd_uint srcDepthPitch )
{
    cout << "ACDDeviceImp::updateResource(DEST_RSC," << destSubResource << ",BOX*,srcData," << srcRowPitch
         << "," << srcDepthPitch << ") -> Not implemented yet" << endl;
}

acd_bool ACDDeviceImp::swapBuffers()
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    
    
    cout << "ACDDeviceImp::swapBuffers() - OK" << endl;
    _driver->sendCommand(gpu3d::GPU_SWAPBUFFERS);

    //  If using the default render buffer swap front and back buffers (double buffering).
    /*if (_defaultRenderBuffers)
    {
        _defaultBackBuffer = _defaultFrontBuffer;
        _defaultFrontBuffer = _currentRenderTarget;
    }*/

    _currentFrame++;
    _currentBatch = 0;
    
    GLOBALPROFILER_EXITREGION()
    
    return true;
}

acd_uint ACDDeviceImp::_packRGBA8888(acd_ubyte red, acd_ubyte green, acd_uint blue, acd_uint alpha)
{
    return (static_cast<acd_uint>(alpha) << 24) +
           (static_cast<acd_uint>(blue)  << 16) +
           (static_cast<acd_uint>(green) <<  8) +
            static_cast<acd_uint>(red);
}

void ACDDeviceImp::clearColorBuffer(acd_ubyte red, acd_ubyte green, acd_ubyte blue, acd_ubyte alpha)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")

    ///////////////////////////////////////////////////////////////
    /// Synchronize render buffers                              ///
    ///////////////////////////////////////////////////////////////
    _syncRenderBuffers();

    // Update the clear color buffer
    _clearColorBuffer = _packRGBA8888(red, green, blue, alpha);

    //  Get scissor test state.
    acd_bool scissorEnabled;
    acd_int scissorIniX, scissorIniY;
    acd_uint scissorWidth, scissorHeight;

    acd_uint resWidth, resHeight;
    // Incorrecto!!
    _driver->getResolution(resWidth, resHeight);

    _rast->getScissor(scissorEnabled, scissorIniX, scissorIniY, scissorWidth, scissorHeight);

    //  Check if scissor test is enabled.
    if (_defaultRenderBuffers &&
        ((!scissorEnabled) || ((scissorIniX == 0) && (scissorIniY == 0) && (scissorWidth == resWidth) && (scissorHeight == resHeight)))
       )
    {
        //  WARNING:  The clear color register for fast color clear must only be modified before
        //  issuing a fast color clear command.

        // Check if update GPU is required.
        if ( _clearColorBuffer.changed() )
        {
            gpu3d::GPURegData data;
            data.qfVal[0] =  float(_clearColorBuffer         & 0xff) / 255.0f;
            data.qfVal[1] =  float((_clearColorBuffer >> 8)  & 0xff) / 255.0f;
            data.qfVal[2] =  float((_clearColorBuffer >> 16) & 0xff) / 255.0f;
            data.qfVal[3] =  float((_clearColorBuffer >> 24) & 0xff) / 255.0f;
            _driver->writeGPURegister(gpu3d::GPU_COLOR_BUFFER_CLEAR, data);
            _clearColorBuffer.restart();
        }

        // Perform a full (fast) clear of the color buffer
        _driver->sendCommand(gpu3d::GPU_CLEARCOLORBUFFER);
    }
    else
    {
        //  Implement a partial clear of color buffer.
        _partialClear(true, false, false, red, green, blue, alpha, 0x0000, 0x00);
    }
    
    GLOBALPROFILER_EXITREGION()
}

void ACDDeviceImp::clearZStencilBuffer( acd_bool clearZ, acd_bool clearStencil,
                                        acd_float zValue, acd_int stencilValue )
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    ///////////////////////////////////////////////////////////////
    /// Synchronize render buffers                              ///
    ///////////////////////////////////////////////////////////////
    _syncRenderBuffers();

    if (clearZ)
        _zClearValue = zValue;

    if (clearStencil)
        _stencilClearValue = stencilValue;

    //  Get scissor test parameters.
    acd_bool scissorEnabled;
    acd_int scissorIniX, scissorIniY;
    acd_uint scissorWidth, scissorHeight;

    acd_uint resWidth, resHeight;
    _driver->getResolution(resWidth, resHeight);

    _rast->getScissor(scissorEnabled, scissorIniX, scissorIniY, scissorWidth, scissorHeight);

    ACDRenderTargetImp *zstencil = _currentZStencilBuffer;

    if (clearZ && clearStencil)
    {
        //  Check if scissor test is enabled.
        if (zstencil->allowsCompression() && 
            ((!scissorEnabled) || ((scissorIniX == 0) && (scissorIniY == 0) && (scissorWidth == resWidth) && (scissorHeight == resHeight))))
        {
            //  Perform full (fast) z and stencil clear.

            //  WARNING: The z and stencil values for the fast z and stencil clear must only be updated before
            //  issuing the fast z and stencil clear command.

            if (_zClearValue.changed())
            {
                //  The float point depth value in the 0 .. 1 range must be converted to an integer
                //  value of the selected depth bit precission.
                //
                //  WARNING: This function should depend on the current depth bit precission.  Should be
                //  configurable using the wgl functions that initialize the framebuffer.  By default
                //  should be 24 (stencil 8 depth 24).  Currently the simulator only supports 24.
                //
                gpu3d::GPURegData data;
                data.uintVal = static_cast<acd_uint>(static_cast<acd_float>(_zClearValue) * static_cast<acd_float>((1 << 24) - 1));
                _driver->writeGPURegister(gpu3d::GPU_Z_BUFFER_CLEAR, data);
                _zClearValue.restart();
            }

            if (_stencilClearValue.changed())
            {
                gpu3d::GPURegData data;
                data.intVal = _stencilClearValue;
                _driver->writeGPURegister(gpu3d::GPU_STENCIL_BUFFER_CLEAR, data);
                _stencilClearValue.restart();
            }

            _driver->sendCommand( gpu3d::GPU_CLEARZSTENCILBUFFER );
            _hzBufferValid = true; // HZ buffer contents are valid
        }
        else
        {
            //  Implement a partial clear of color buffer.
            _partialClear(false, true, true, 0, 0, 0, 0, zValue, stencilValue);
        }
    }
    else if (clearZ)
    {
        //  Check if scissor test is enabled.
        if (zstencil->allowsCompression() && 
            ((!scissorEnabled) || ((scissorIniX == 0) && (scissorIniY == 0) && (scissorWidth == resWidth) && (scissorHeight == resHeight))))
        {
            //  Perform full (fast) z and stencil clear.

            //  WARNING: The z and stencil values for the fast z and stencil clear must only be updated before
            //  issuing the fast z and stencil clear command.

            if (_zClearValue.changed())
            {
                //  The float point depth value in the 0 .. 1 range must be converted to an integer
                //  value of the selected depth bit precission.
                //
                //  WARNING: This function should depend on the current depth bit precission.  Should be
                //  configurable using the wgl functions that initialize the framebuffer.  By default
                //  should be 24 (stencil 8 depth 24).  Currently the simulator only supports 24.
                //
                gpu3d::GPURegData data;
                data.uintVal = static_cast<acd_uint>(static_cast<acd_float>(_zClearValue) * static_cast<acd_float>((1 << 24) - 1));
                _driver->writeGPURegister(gpu3d::GPU_Z_BUFFER_CLEAR, data);
                _zClearValue.restart();
            }

            if (_stencilClearValue.changed())
            {
                gpu3d::GPURegData data;
                data.intVal = _stencilClearValue;
                _driver->writeGPURegister(gpu3d::GPU_STENCIL_BUFFER_CLEAR, data);
                _stencilClearValue.restart();
            }

            cout << "Warning: Clear Z implemented as Clear Z and Stencil (optimization)" << endl;
            _driver->sendCommand( gpu3d::GPU_CLEARZSTENCILBUFFER );
            _hzBufferValid = true; // HZ buffer contents are valid
        }
        else
        {
            //  Implement a partial clear of color buffer.
            _partialClear(false, true, false, 0, 0, 0, 0, zValue, stencilValue);
        }
    }
    else if (clearStencil)
    {
        //  Implement a partial clear of color buffer.
        _partialClear(false, false, true, 0, 0, 0, 0, zValue, stencilValue);
    }
    
    GLOBALPROFILER_EXITREGION()
}

void ACDDeviceImp::_partialClear(acd_bool clearColor, acd_bool clearZ, acd_bool clearStencil,
                                 acd_ubyte red, acd_ubyte green, acd_ubyte blue, acd_ubyte alpha,
                                 acd_float zValue, acd_int stencilValue)
{
    gpu3d::GPURegData data;
    vector<gpu3d::GPURegData> savedRegState;
    vector<const StoredStateItem*> storedState;

    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR_MASK_R));
    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR_MASK_G));
    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR_MASK_B));
    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR_MASK_A));
    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR));
    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_ENABLED));

    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_FRONT_STENCIL_COMPARE_MASK));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_Z_ENABLED));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_Z_FUNC));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_Z_MASK));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_RANGE_NEAR));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_RANGE_FAR));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_FRONT_COMPARE_FUNC));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_FRONT_STENCIL_REF_VALUE));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_FRONT_STENCIL_COMPARE_MASK));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_FRONT_STENCIL_OPS));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_STENCIL_ENABLED));
    storedState.push_back(_zStencil->createStoredStateItem(ACD_ZST_STENCIL_BUFFER_DEF));

    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_CULLMODE));

    storedState.push_back(createStoredStateItem(ACD_DEV_PRIMITIVE));
    storedState.push_back(createStoredStateItem(ACD_DEV_VERTEX_THREAD_RESOURCES));
    storedState.push_back(createStoredStateItem(ACD_DEV_FRAGMENT_THREAD_RESOURCES));

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_VERTEX_ATTRIBUTE_MAP + i)));

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE + i)));

    for (acd_uint i = 0; i < gpu3d::MAX_FRAGMENT_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES + i)));

    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_STRIDE));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_ELEMENTS));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_FREQUENCY));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_DATA_TYPE));

    storedState.push_back(createStoredStateItem(ACD_DEV_STREAM_COUNT));
    storedState.push_back(createStoredStateItem(ACD_DEV_STREAM_START));
    storedState.push_back(createStoredStateItem(ACD_DEV_INDEX_MODE));
    storedState.push_back(createStoredStateItem(ACD_DEV_VSH));
    storedState.push_back(createStoredStateItem(ACD_DEV_FSH));

    _driver->readGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, data);
    savedRegState.push_back(data);

    //  Check if the color buffer must be cleared.
    if (clearColor)
    {
        // Enable writing on all color channels.
        _blending->setColorMask(0, true, true, true, true);
        
        //  Disable color blending.
        _blending->setEnable(0, false);
    }
    else 
    {
        // Disable writing on all color channels
        _blending->setColorMask(0, false, false, false, false);
    }

    //  Check if the z buffer must be cleared
    if (clearZ)
    {
        // Enable writing the depth value in the buffer.
        _zStencil->setZMask(true);

        //  Enable depth test.
        _zStencil->setZEnabled(true);
        _zStencil->setZStencilBufferDefined(true);

        //  Set depth function to always pass/write.
        _zStencil->setZFunc(ACD_COMPARE_FUNCTION_ALWAYS);

        //  Set depth near and far planes.
        _zStencil->setDepthRange(0.0, 1.0);
    }
    else
    {
        // Disable writing the deth value in the buffer.
        _zStencil->setZMask(false);

        // Disable depth test.
        _zStencil->setZEnabled(false);
    }

    //  Check if the stencil buffer must be cleared
    if (clearStencil)
    {
        _zStencil->setStencilEnabled(true);

        //  Set the stencil test function to never fail.
        //  Set the stencil reference value to the clear value.
        //  Enable all bits in the stencil mask.
        _zStencil->setStencilFunc(ACD_FACE_FRONT, ACD_COMPARE_FUNCTION_ALWAYS, stencilValue, 255);

        //  Set stencil pass update function to replace with reference value.
        //  Set depth pass and fail update functions to keep stencil value.
        _zStencil->setStencilOp(ACD_FACE_FRONT, ACD_STENCIL_OP_KEEP, ACD_STENCIL_OP_REPLACE, ACD_STENCIL_OP_REPLACE);

        //  Enable stencil test.
        _zStencil->setStencilEnabled(true);
        _zStencil->setZStencilBufferDefined(true);
    }
    else
    {
        // Disable stencil test.
        _zStencil->setStencilEnabled(false);
    }

    // Disable face culling.
    _rast->setCullMode(ACD_CULL_NONE);

    //  Set rendering primitive to QUAD.
    _primitive = ACD_QUADS;

    //  Set vertex and fragment shaders to the default shaders.
    _vsh = _defaultVshProgram;
    _fsh = _defaultFshProgram;

    //  Set vertex and fragment shader resource usage.
    _vshResources = 1;
    _fshResources = 1;

    //  Set all vertex attributes but position to disabled.
    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
       _vaMap[i] = gpu3d::ST_INACTIVE_ATTRIBUTE;
    
    _vaMap[gpu3d::POSITION_ATTRIBUTE] = 0;


    ACDFloatVector4 color;
    color[0] = float((red & 0xff) / 255.0f);
    color[1] = float((green & 0xff) / 255.0f);
    color[2] = float((blue & 0xff) / 255.0f);
    color[3] = float((alpha & 0xff) / 255.0f);

    _currentColor = color;

    data.qfVal[0] = color[0];
    data.qfVal[1] = color[1];
    data.qfVal[2] = color[2];
    data.qfVal[3] = color[3];

    _driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, data);

    zValuePartialClear = zValue;

    if (zValuePartialClear.changed())
    {

        //  Allocate buffer with the quad positions.
        acd_float vertexPositions[4 * 4] =
        {
            -1.0f, -1.0f, zValue, 1.0f,
             1.0f, -1.0f, zValue, 1.0f,
             1.0f,  1.0f, zValue, 1.0f,
            -1.0f,  1.0f, zValue, 1.0f
        };

        destroy(vertexBufferPartialClear);
        vertexBufferPartialClear = createBuffer(4*16, (acd_ubyte*)vertexPositions);

    }

    ACD_STREAM_DESC descPartialClear;
    descPartialClear.components = 4;
    descPartialClear.frequency = 0;
    descPartialClear.offset = 0;
    descPartialClear.stride = 16;
    descPartialClear.type = ACD_SD_FLOAT32;

    //  Set stream 0 for reading the quad vertex positions.
    _stream[0]->set(vertexBufferPartialClear, descPartialClear);

    zValuePartialClear.restart();


    //  Disable indexed mode.
    _indexedMode = false;

    _draw(0, 4, 0, 0);


    acd_int nextReg = 0;

    _blending->restoreStoredStateItem(storedState[nextReg++]);
    _blending->restoreStoredStateItem(storedState[nextReg++]);
    _blending->restoreStoredStateItem(storedState[nextReg++]);
    _blending->restoreStoredStateItem(storedState[nextReg++]);
    _blending->restoreStoredStateItem(storedState[nextReg++]);
    _blending->restoreStoredStateItem(storedState[nextReg++]);

    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);
    _zStencil->restoreStoredStateItem(storedState[nextReg++]);


    _rast->restoreStoredStateItem(storedState[nextReg++]);

    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_FRAGMENT_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);

    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);

    _driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, savedRegState[0]);
}

void ACDDeviceImp::clearRenderTarget( ACDRenderTarget* rTarget,
                                      acd_ubyte red,
                                      acd_ubyte green,
                                      acd_ubyte blue,
                                      acd_ubyte alpha)
{
    cout << "ACDDeviceImp::clearRenderTarget(RTarget," << (acd_uint)red << "," << (acd_uint)green << ","
         << (acd_uint)blue << "," << (acd_uint)alpha << ") -> Not implemented yet" << endl;
}

void ACDDeviceImp::clearZStencilTarget( ACDZStencilTarget* zsTarget,
                                        acd_bool clearZ,
                                        acd_bool clearStencil,
                                        acd_float zValue,
                                        acd_ubyte stencilValue )
{
    cout << "ACDDeviceImp::clearZStencilTarget(ZSTarget," << (clearZ ? "TRUE," : "FALSE,")
        << (clearStencil ? "TRUE," : "FALSE,") << (acd_uint)stencilValue << ") -> Not implemented yet" << endl;
}

void ACDDeviceImp::copySurfaceDataToRenderBuffer(ACDTexture2D *sourceTexture, acd_uint mipLevel, ACDRenderTarget *destRenderTarget, bool preload)
{
    //  Get a pointer to the source mipmap data.
    u8bit *sourceData;
    u32bit sourceDataPitch;
    sourceTexture->map(mipLevel, ACD_MAP_READ, sourceData, sourceDataPitch);
    
    //  Get source mipmap information.
    u32bit sourceWidth = sourceTexture->getWidth(mipLevel);
    u32bit sourceHeight = sourceTexture->getHeight(mipLevel);
    u32bit width = destRenderTarget->getWidth();
    u32bit height = destRenderTarget->getHeight();
    gpu3d::TextureFormat format;
    bool invertColors;
    switch(sourceTexture->getFormat(mipLevel))
    {
        case ACD_FORMAT_XRGB_8888:
        case ACD_FORMAT_ARGB_8888:
            format = gpu3d::GPU_RGBA8888;
            invertColors = true;
            break;
        case ACD_FORMAT_RG16F:
            format = gpu3d::GPU_RG16F;
            invertColors = false;
            break;
        case ACD_FORMAT_R32F:
            format = gpu3d::GPU_R32F;
            invertColors = false;
            break;                
        case ACD_FORMAT_RGBA16F:
            format = gpu3d::GPU_RGBA16F;
            invertColors = false;
            break;
        case ACD_FORMAT_ABGR_161616:
            format = gpu3d::GPU_RGBA16;
            invertColors = false;
            break;

        default:
            panic("ACDDeviceImp", "copySurfaceDataToRenderBuffer", "Unsupported render buffer format.");
            break;
    }
    
    //  Tile the source data to the render buffer format.
    u8bit *destData;    
    u32bit destDataSize;    
    _driver->tileRenderBufferData(sourceData, width, height, false, 1, format, invertColors, destData, destDataSize);
    
    sourceTexture->unmap(mipLevel);
    
    TextureAdapter surface(destRenderTarget->getTexture());
    
    //  Get the memory descriptor to the render buffer.
    acd_uint destRenderTargetMD = _moa->md(static_cast<ACDTexture2DImp*>(surface.getTexture()), mipLevel);

    //  Check if data is to be preloaded into GPU memory.
    if (preload)
    {
        //  Update the render buffer.
        _driver->writeMemoryPreload(destRenderTargetMD, 0, destData, destDataSize);
    }
    else
    {
        //  Update the render buffer.
        _driver->writeMemory(destRenderTargetMD, destData, destDataSize, true);
    }    
    
    //  Delete the converted data.    
    delete[] destData;
}


void ACDDeviceImp::getShaderLimits(ACD_SHADER_TYPE type, ACD_SHADER_LIMITS* limits)
{
    cout << "ACDDeviceImp::getShaderLimits() -> Not implemented yet" << endl;
}

ACDShaderProgram* ACDDeviceImp::createShaderProgram() const
{
    return new ACDShaderProgramImp();
}

void ACDDeviceImp::setGeometryShader(ACDShaderProgram* program)
{
    ACDShaderProgramImp* gsh = static_cast<ACDShaderProgramImp*>(program);
    _gsh = gsh;
}

void ACDDeviceImp::setVertexShader(ACDShaderProgram* program)
{
    ACDShaderProgramImp* vsh = static_cast<ACDShaderProgramImp*>(program);
    _vsh = vsh;
}

void ACDDeviceImp::setFragmentShader(ACDShaderProgram* program)
{
    ACDShaderProgramImp* fsh = static_cast<ACDShaderProgramImp*>(program);
    _fsh = fsh;
}

void ACDDeviceImp::setVertexDefaultValue(acd_float currentColor[4])
{
    ACDFloatVector4 color;
    color[0] = currentColor[0]; 
    color[1] = currentColor[1]; 
    color[2] = currentColor[2];
    color[3] = currentColor[3];

    _currentColor = color;
}

void ACDDeviceImp::_syncVertexShader()
{
    ACDShaderProgramImp* vsh = _vsh;

    ACD_ASSERT(
        if ( vsh == 0 )
            panic("ACDDeviceImp", "_syncVertexShader", "Vertex program not selected");
    )

    if( _currentColor.changed())
    {
        gpu3d::GPURegData data;
        const ACDFloatVector4& _currentAux = _currentColor;
        data.qfVal[0] = _currentAux[0];
        data.qfVal[1] = _currentAux[1];
        data.qfVal[2] = _currentAux[2];
        data.qfVal[3] = _currentAux[3];

        _driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, (u32bit)GPUDriver::VS_COLOR, data);
            
        _currentColor.restart();

    }

    // Optimize the shader
    _optimizeShader(vsh, ACD_VERTEX_SHADER);

    // Synchronize the current shader written outputs
    const acd_uint vshOutputsCount = _vshOutputs.size();

    for ( acd_uint i = 0; i < vshOutputsCount; ++i )
    {
        _vshOutputs[i] = vsh->getOutputWritten(i);
    }

    // Synchronize the current shader temp resources
    _vshResources = (vsh->getMaxAliveTemps() > 0)? vsh->getMaxAliveTemps():1;

    // Synchronize GPU memory (if required)
    _moa->syncGPU(vsh);

    // Update constants
    vsh->updateConstants(_driver, gpu3d::GPU_VERTEX_CONSTANT);

    // Get program size (bytes occupied in GPU)
    acd_uint programSize;
    vsh->memoryData(0, programSize);

    ACD_ASSERT(
        if ( programSize == 0 )
            panic("ACDDeviceImp", "_syncVertexShader", "Vertex program code not defined");
    )

    // Get the memory descriptor of this program
    acd_uint md = _moa->md(vsh);

    // Selects the vertex shader as current
    _driver->commitVertexProgram(md, programSize, 0);
}


void ACDDeviceImp::_syncFragmentShader()
{
    ACDShaderProgramImp* fsh = _fsh;

    ACD_ASSERT(
        if ( fsh == 0 )
            panic("ACDDeviceImp", "_syncFragmentShader", "Fragment program not selected");
    )

    // Optimize the shader
    _optimizeShader(fsh, ACD_FRAGMENT_SHADER);

    // Synchronize the current shader written outputs
    const acd_uint fshInputsCount = _fshInputs.size();

    for ( acd_uint i = 0; i < fshInputsCount; ++i )
    {
        _fshInputs[i] = fsh->getInputRead(i);
    }

    // Synchronize the current shader temp resources
    _fshResources = (fsh->getMaxAliveTemps() > 0)? fsh->getMaxAliveTemps():1;

    // Synchronize GPU memory (if required)
    _moa->syncGPU(fsh);

    // Update constants
    fsh->updateConstants(_driver, gpu3d::GPU_FRAGMENT_CONSTANT);

    // Get program size (bytes occupied in GPU)
    acd_uint programSize;
    fsh->memoryData(0, programSize);

    ACD_ASSERT(
        if ( programSize == 0 )
            panic("ACDDeviceImp", "_syncVertexShader", "Fragment program code not defined");
    )

    // Get the memory descriptor of this program
    acd_uint md = _moa->md(fsh);

    // Selects the vertex shader as current
    _driver->commitFragmentProgram(md, programSize, 0);

}

void ACDDeviceImp::_optimizeShader(ACDShaderProgramImp* shProgramImp, ACD_SHADER_TYPE shType)
{
    if (!shProgramImp->isOptimized())
    {
        const acd_ubyte* unOptCode = shProgramImp->getCode();
        acd_uint unOptCodeSize = shProgramImp->getSize();

//cout << "ACDDeviceImp:_optimizeShader => Shader Before Optimization: " << endl;
//shProgramImp->printASM(cout);
//cout << endl;

        vector<ACDVector<acd_float,4> > constantBank;

        for (acd_uint i=0; i < 96; i++)
        {
            acd_float constant[4];
            shProgramImp->getConstant(i, constant);
            constantBank.push_back(ACDVector<acd_float,4>(constant));
        }

        _shOptimizer.setCode(unOptCode, unOptCodeSize);
        _shOptimizer.setConstants(constantBank);

        _shOptimizer.optimize();


        acd_uint optCodeSize;
        const acd_ubyte* optCode = _shOptimizer.getCode(optCodeSize);

//shProgramImp->setOptimizedCode(optCode, optCodeSize);
//cout << "ACDDeviceImp:_optimizeShader => Shader After ACD Optimization: " << endl;
//shProgramImp->printASM(cout);
//cout << endl;

        acdlib_opt::OPTIMIZATION_OUTPUT_INFO optOutInfo;
        _shOptimizer.getOptOutputInfo(optOutInfo);

        acd_uint maxAliveTemps = optOutInfo.maxAliveTemps;

        //  Perform optimization/translation/transformation of the shader using the GPU Driver
        //  functions.

        //  Allocate buffer for the driver optimized code.
        acd_uint driverOptCodeSize = optCodeSize * 10;
        u8bit *driverOptCode = new acd_ubyte[driverOptCodeSize];

        ////////////////////////////////////////////////////////////////////////////////////////////
        //  Prepare the rendering state information required for the microtriangle transformation.
        ////////////////////////////////////////////////////////////////////////////////////////////
        GPUDriver::MicroTriangleRasterSettings settings;
        
        bool frontIsCCW = (_rast->_faceMode == ACD_FACE_CCW);
        bool frontCulled = ((_rast->_cullMode == ACD_CULL_FRONT) || (_rast->_cullMode == ACD_CULL_FRONT_AND_BACK));
        bool backCulled = ((_rast->_cullMode == ACD_CULL_BACK) || (_rast->_cullMode == ACD_CULL_FRONT_AND_BACK));

        settings.faceCullEnabled = _rast->_cullMode == ACD_CULL_NONE;

        if (frontIsCCW)
        {
            settings.CCWFaceCull = frontCulled;
            settings.CWFaceCull = backCulled;
        }
        else // backface is CCW
        {
            settings.CCWFaceCull = backCulled;
            settings.CWFaceCull = frontCulled;
        }

        //  For now ACD doesn´t known anything about perspective transformation. A perspective/orthogonal
        //  projection is done according to the constants used by the vertex shader. Since the vertex shader
        //  code and constants are user-specified we cannot determine the type of perspective.
        //  The future solution will be to request the ACD user to specify the type of vertex projection.
        settings.zPerspective = true;

        //  Set attribute interpolation.
        for (unsigned int attr = 0; attr < gpu3d::MAX_FRAGMENT_ATTRIBUTES; attr++)
        {
            switch(_rast->_interpolation[attr])
            {
                case ACD_INTERPOLATION_NONE:
                    settings.smoothInterp[attr] = false;
                    settings.perspectCorrectInterp[attr] = false;
                    break;
                case ACD_INTERPOLATION_LINEAR:
                    settings.smoothInterp[attr] = true;
                    settings.perspectCorrectInterp[attr] = true;
                    break;
                case ACD_INTERPOLATION_NOPERSPECTIVE:
                    settings.smoothInterp[attr] = true;
                    settings.perspectCorrectInterp[attr] = false;
                    break;
                default:
                    panic("ACDDeviceImp", "_optimizeShader", "Unknown/unsupported interpolation mode");
            }
        }

        ////////////////////////////////////////////////////////////////////////////

        // Use the GPU driver shader program capabilities.
        _driver->translateShaderProgram((u8bit *) optCode, optCodeSize, driverOptCode, driverOptCodeSize,
                                       (shType == ACD_VERTEX_SHADER), maxAliveTemps, settings);

        shProgramImp->setOptimizedCode(driverOptCode, driverOptCodeSize);

//cout << "ACDDeviceImp:_optimizeShader => Shader After Driver Optimization: " << endl;
//shProgramImp->printASM(cout);
//cout << endl;

        //  Delete buffers.
        delete[] driverOptCode;

        constantBank = _shOptimizer.getConstants();

        for (acd_uint i=0; i < 96; i++)
        {
            shProgramImp->setConstant(i, constantBank[i].c_array());
        }

        for (acd_uint i=0; i < ACDShaderProgramImp::MAX_SHADER_ATTRIBUTES; i++)
        {
            shProgramImp->setInputRead(i, optOutInfo.inputsRead[i]);
            shProgramImp->setOutputWritten(i, optOutInfo.outputsWritten[i]);
        }

        shProgramImp->setMaxAliveTemps( maxAliveTemps );
    }
}

ACDStoredState* ACDDeviceImp::saveState(ACDStoredItemIDList siIds) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    
    ACDStoredStateImp* ret = new ACDStoredStateImp();
    ACDStoredItemIDList::const_iterator it = siIds.begin();

    for ( ; it != siIds.end(); ++it )
    {
        if ((*it) < ACD_ZST_FIRST_ID)
        {
            // Its a rasterization stage
            ret->addStoredStateItem(_rast->createStoredStateItem((*it)));
        }
        else if ((*it) < ACD_BLENDING_FIRST_ID)
        {
            // Its a z stencil stage
            ret->addStoredStateItem(_zStencil->createStoredStateItem((*it)));
        }
        else if ((*it) < ACD_LAST_STATE)
        {
            // Its a blending stage
            ret->addStoredStateItem(_blending->createStoredStateItem((*it)));
        }
        else
            panic("ACDDeviceImp","saveState","Unexpected Stored Item Id");
    }

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

ACDStoredState* ACDDeviceImp::saveState(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    ACDStoredStateImp* ret = new ACDStoredStateImp();

    if (stateId < ACD_ZST_FIRST_ID)
    {
        // Its a rasterization stage
        ret->addStoredStateItem(_rast->createStoredStateItem(stateId));
    }
    else if (stateId < ACD_BLENDING_FIRST_ID)
    {
        // Its a z stencil stage
        ret->addStoredStateItem(_zStencil->createStoredStateItem(stateId));
    }
    else if (stateId < ACD_LAST_STATE)
    {
        // Its a blending stage
        ret->addStoredStateItem(_blending->createStoredStateItem(stateId));
    }
    else
        panic("ACDDeviceImp","saveState","Unexpected Stored Item Id");

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

ACDStoredState* ACDDeviceImp::saveAllState() const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    ACDStoredStateImp* ret = new ACDStoredStateImp();

    for (acd_uint i=0; i < ACD_LAST_STATE; i++)
    {
        if (i < ACD_ZST_FIRST_ID)
        {
            // Its a rasterization stage
            ret->addStoredStateItem(_rast->createStoredStateItem(ACD_STORED_ITEM_ID(i)));
        }
        else if (i < ACD_BLENDING_FIRST_ID)
        {
            // Its a z stencil stage
            ret->addStoredStateItem(_zStencil->createStoredStateItem(ACD_STORED_ITEM_ID(i)));
        }
        else if (i < ACD_LAST_STATE)
        {
            // Its a blending stage
            ret->addStoredStateItem(_blending->createStoredStateItem(ACD_STORED_ITEM_ID(i)));
        }
        else
            panic("ACDDeviceImp","saveState","Unexpected Stored Item Id");
    }

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

void ACDDeviceImp::restoreState(const ACDStoredState* state)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    
    const ACDStoredStateImp* ssi = static_cast<const ACDStoredStateImp*>(state);

    std::list<const StoredStateItem*> ssiList = ssi->getSSIList();

    std::list<const StoredStateItem*>::const_iterator iter = ssiList.begin();

    while ( iter != ssiList.end() )
    {
        const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(*iter);

        if (acdssi->getItemId() < ACD_ZST_FIRST_ID)
        {
            // Its a rasterization stage
            _rast->restoreStoredStateItem(acdssi);
        }
        else if (acdssi->getItemId() < ACD_BLENDING_FIRST_ID)
        {
            // Its a z stencil stage
            _zStencil->restoreStoredStateItem(acdssi);

        }
        else if (acdssi->getItemId() < ACD_LAST_STATE)
        {
            // Its a blending stage
            _blending->restoreStoredStateItem(acdssi);
        }
        else
            panic("ACDDeviceImp","restoreState","Unexpected Stored Item Id");

        iter++;
    }

    GLOBALPROFILER_EXITREGION()
}

void ACDDeviceImp::setStartFrame(acd_uint startFrame)
{
    _startFrame = startFrame;
    _currentFrame = 0;
    _currentBatch = 0;
}

void ACDDeviceImp::destroyState(ACDStoredState* state)
{
    const ACDStoredStateImp* ssi = static_cast<const ACDStoredStateImp*>(state);

    std::list<const StoredStateItem*> stateList = ssi->getSSIList();

    std::list<const StoredStateItem*>::iterator iter = stateList.begin();

    while ( iter != stateList.end() )
    {
        delete (*iter);
        iter++;
    }

    delete ssi;
}

void ACDDeviceImp::DBG_dump(const acd_char* file, acd_enum flags)
{
    _dump(file, flags);
}

void ACDDeviceImp::DBG_deferred_dump(const acd_char* file, acd_enum flags, acd_uint frame, acd_uint batch)
{
    // Avoid deferring dumps for already past events.

    if (frame < _currentFrame)
        return;

    if (frame == _currentFrame)
    {
        if (batch < _currentBatch)
            return;
    }

    // The frame and batch event is in the future (not in current frame and batch)

    DumpEventInfo newEvent(file, flags, frame, batch);
    _dumpEventList.push_back(newEvent);

    // Update the _nextDumpEvent attributes for faster searches.

    if (_nextDumpEvent.valid)
    {
        if (frame < _nextDumpEvent.frame)
        {
            _nextDumpEvent = newEvent;
        }
        else if (frame == _nextDumpEvent.frame && batch < _nextDumpEvent.batch)
        {
            _nextDumpEvent = newEvent;
        }
    }
    else
        _nextDumpEvent = newEvent;
}

acd_bool ACDDeviceImp::DBG_save(const acd_char* file)
{
    return true;
}

acd_bool ACDDeviceImp::DBG_load(const acd_char* file)
{
    return true;
}

void ACDDeviceImp::HACK_setPreloadMemory(acd_bool enablePreload)
{
    _driver->setPreloadMemory(enablePreload);
}

const char* ACDDeviceImp::PrimitivePrint::print(const ACD_PRIMITIVE& primitive) const
{
    switch ( primitive )
    {
        case ACD_POINTS:
            return "ACD_POINTS";
        case ACD_LINE_LOOP:
            return "ACD_LINE_LOOP";
        case ACD_LINE_STRIP:
            return "ACD_LINE_STRIP";
        case ACD_TRIANGLES:
            return "ACD_TRIANGLES";
        case ACD_TRIANGLE_STRIP:
            return "ACD_TRIANGLE_STRIP";
        case ACD_TRIANGLE_FAN:
            return "ACD_TRIANGLE_FAN";
        case ACD_QUADS:
            return "ACD_QUADS";
        case ACD_QUAD_STRIP:
            return "ACD_QUAD_STRIP";
        case ACD_PRIMITIVE_UNDEFINED:
        default:
            return "ACD_PRIMITIVE_UNDEFINED";
    }
}

acd_bool ACDDeviceImp::DBG_printMemoryUsage()
{
    _driver->printMemoryUsage();
    return true;
}

void ACDDeviceImp::_syncHZRegister()
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Code based on previous version in Legacy GLLib, AuxfuncLib.cpp, function setHZTest() //
    //////////////////////////////////////////////////////////////////////////////////////////
    acd_bool activateHZ = false;

    // Check if z test is enabled
    if ( _zStencil->isZEnabled() ) {

        // Get the current Z compare function
        ACD_COMPARE_FUNCTION depthFunc = _zStencil->getZFunc();

        // Determine if HZ must be activated.
        activateHZ = (_hzBufferValid && ( depthFunc == ACD_COMPARE_FUNCTION_LESS
                      || depthFunc == ACD_COMPARE_FUNCTION_LESS_EQUAL || depthFunc == ACD_COMPARE_FUNCTION_EQUAL ));

        // Check if stencil test is enabled
        if ( _zStencil->isStencilEnabled() ) {

            // Get the stencil operations for the different ZStencil events
            ACD_STENCIL_OP stFail, dpFail, dpPass;

            _zStencil->getStencilOp(ACD_FACE_FRONT, stFail, dpFail, dpPass); // or back or what ?

            // Check consistency, front & back are faces with the same values (this check maybe is not required)
            ACD_STENCIL_OP dummyStFail, dummyDpFail, dummyDpPass;

            _zStencil->getStencilOp(ACD_FACE_BACK, dummyStFail, dummyDpFail, dummyDpPass);

            //if ( stFail != dummyStFail || dpFail != dummyDpFail || dpPass != dummyDpPass)
            //    panic("ACDDeviceImp", "_setupHZ", "FRONT and BACK stencil operation differs");

            // Determine if HZ must be activated
            activateHZ = (activateHZ && stFail == ACD_STENCIL_OP_KEEP && dpFail == ACD_STENCIL_OP_KEEP);
        }

        // Check if HZ buffer content is invalid
        if ( _zStencil->getZMask() && (depthFunc == ACD_COMPARE_FUNCTION_ALWAYS
             || depthFunc == ACD_COMPARE_FUNCTION_GREATER || depthFunc == ACD_COMPARE_FUNCTION_EQUAL
             || depthFunc == ACD_COMPARE_FUNCTION_NOT_EQUAL) )
        {
            // Set HZ buffer content as not valid.
            _hzBufferValid = false;
        }
    }

    _hzActivated = activateHZ;

    if ( _hzActivated.changed() ) {
        gpu3d::GPURegData data;
        data.booleanVal = _hzActivated;
        _driver->writeGPURegister(gpu3d::GPU_HIERARCHICALZ, data);
        _hzActivated.restart();
    }
}

void ACDDeviceImp::alphaTestEnabled(acd_bool enabled)
{
    _alphaTest = enabled;
}

const StoredStateItem* ACDDeviceImp::createStoredStateItem(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")

    ACDStoredStateItem* ret;
    acd_uint aux;

    if (stateId >= ACD_DEVICE_FIRST_ID && stateId < ACD_DEV_LAST)
    {  
        if ((stateId >= ACD_DEV_VERTEX_ATTRIBUTE_MAP) && (stateId < ACD_DEV_VERTEX_ATTRIBUTE_MAP + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            aux = stateId - ACD_DEV_VERTEX_ATTRIBUTE_MAP;
            ret = new ACDSingleUintStoredStateItem(_vaMap[aux]);
        }
        else if ((stateId >= ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE) && (stateId < ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            aux = stateId - ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE;
            ret = new ACDSingleBoolStoredStateItem(_vshOutputs[aux]);
        }
            else if ((stateId >= ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES) && (stateId < ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            aux = stateId - ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES;
            ret = new ACDSingleBoolStoredStateItem(_fshInputs[aux]);
        }
        else 
        {
            switch(stateId)
            {
                case ACD_DEV_FRONT_BUFFER:              ret = new ACDSingleVoidStoredStateItem((void*)(_defaultFrontBuffer));   break;
                case ACD_DEV_BACK_BUFFER:				ret = new ACDSingleVoidStoredStateItem((void*)(_defaultBackBuffer));    break;
                case ACD_DEV_PRIMITIVE:                 ret = new ACDSingleEnumStoredStateItem((acd_enum)(_primitive));	        break;
                case ACD_DEV_CURRENT_COLOR:             ret = new ACDFloatVector4StoredStateItem(_currentColor);                break;
                case ACD_DEV_HIERARCHICALZ:             ret = new ACDSingleBoolStoredStateItem((acd_enum)(_hzActivated));       break;
                case ACD_DEV_STREAM_START:              ret = new ACDSingleUintStoredStateItem((acd_enum)(_streamStart));       break;
                case ACD_DEV_STREAM_COUNT:              ret = new ACDSingleUintStoredStateItem((acd_enum)(_streamCount));       break;
                case ACD_DEV_STREAM_INSTANCES:          ret = new ACDSingleUintStoredStateItem((acd_enum)(_streamInstances));   break;
                case ACD_DEV_INDEX_MODE:                ret = new ACDSingleBoolStoredStateItem((acd_enum)(_indexedMode));       break;
                case ACD_DEV_EARLYZ:                    ret = new ACDSingleBoolStoredStateItem((acd_enum)(_earlyZ));            break;
                case ACD_DEV_VERTEX_THREAD_RESOURCES:   ret = new ACDSingleUintStoredStateItem((acd_enum)(_vshResources));      break;
                case ACD_DEV_FRAGMENT_THREAD_RESOURCES: ret = new ACDSingleUintStoredStateItem((acd_enum)(_fshResources));      break;
                case ACD_DEV_VSH:                       ret = new ACDSingleVoidStoredStateItem((void*)(_vsh));                  break;
                case ACD_DEV_FSH:                       ret = new ACDSingleVoidStoredStateItem((void*)(_fsh));                  break;


                // case ACD_RASTER_... <- add here future additional rasterization states.
                default:
                    cout << stateId << endl;
                    panic("ACDDeviceImp","createStoredStateItem()","Unknown device state");
                    ret = 0;
            }
        }
    }
    else
        panic("ACDDeviceImp","createStoredStateItem","Unexpected device state id");

    ret->setItemId(stateId);

    GLOBALPROFILER_EXITREGION()
    return ret;
}


#define CAST_TO_UINT(X)     *(static_cast<const ACDSingleUintStoredStateItem*>(X))
#define CAST_TO_BOOL(X)     *(static_cast<const ACDSingleBoolStoredStateItem*>(X))
#define CAST_TO_VECT4(X)    *(static_cast<const ACDFloatVector4StoredStateItem*>(X))

void ACDDeviceImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")

    const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(ssi);
    acd_uint aux;

    ACD_STORED_ITEM_ID stateId = acdssi->getItemId();

    if (stateId >= ACD_DEVICE_FIRST_ID && stateId < ACD_DEV_LAST)
    {  
        if ((stateId >= ACD_DEV_VERTEX_ATTRIBUTE_MAP) && (stateId < ACD_DEV_VERTEX_ATTRIBUTE_MAP + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            aux = stateId - ACD_DEV_VERTEX_ATTRIBUTE_MAP;
            _vaMap[aux] = *(static_cast<const ACDSingleUintStoredStateItem*>(acdssi));
        }
        else if ((stateId >= ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE) && (stateId < ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            aux = stateId - ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE;
            _vshOutputs[aux] = *(static_cast<const ACDSingleBoolStoredStateItem*>(acdssi));
        }
        else if ((stateId >= ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES) && (stateId < ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            aux = stateId - ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES;
            _fshInputs[aux] = *(static_cast<const ACDSingleBoolStoredStateItem*>(acdssi));
        }
        else
        {
            switch(stateId)
            {
                case ACD_DEV_PRIMITIVE:                 
                    {
                        acd_enum param = *(static_cast<const ACDSingleEnumStoredStateItem*>(acdssi));
                        _primitive = ACD_PRIMITIVE(param);
                        break;
                    }

                case ACD_DEV_CURRENT_COLOR:             _currentColor = CAST_TO_VECT4(acdssi);  break;
                case ACD_DEV_HIERARCHICALZ:             _hzActivated = CAST_TO_BOOL(acdssi);    break;
                case ACD_DEV_STREAM_START:              _streamStart = CAST_TO_UINT(acdssi);    break;
                case ACD_DEV_STREAM_COUNT:              _streamCount = CAST_TO_UINT(acdssi);    break;
                case ACD_DEV_STREAM_INSTANCES:          _streamInstances = CAST_TO_UINT(acdssi);    break;
                case ACD_DEV_INDEX_MODE:                _indexedMode = CAST_TO_BOOL(acdssi);    break;
                case ACD_DEV_EARLYZ:                    _earlyZ = CAST_TO_BOOL(acdssi);         break;
                case ACD_DEV_VERTEX_THREAD_RESOURCES:   _vshResources = CAST_TO_UINT(acdssi);   break;
                case ACD_DEV_FRAGMENT_THREAD_RESOURCES: _fshResources = CAST_TO_UINT(acdssi);   break;
                case ACD_DEV_VSH:                       
                    {
                        void* param = *(static_cast<const ACDSingleVoidStoredStateItem*>(acdssi));
                        _vsh = (ACDShaderProgramImp*)(param);            
                        break;
                    }
                case ACD_DEV_FSH: 
                    {
                        void* param = *(static_cast<const ACDSingleVoidStoredStateItem*>(acdssi));
                        _fsh = (ACDShaderProgramImp*)(param);   
                        break;
                    }
				case ACD_DEV_FRONT_BUFFER:
					{
						void* param = *(static_cast<const ACDSingleVoidStoredStateItem*>(acdssi));
						_defaultFrontBuffer = (ACDRenderTargetImp*)(param);
						break;
					}
                case ACD_DEV_BACK_BUFFER:
					{
						void* param = *(static_cast<const ACDSingleVoidStoredStateItem*>(acdssi));
						_defaultBackBuffer = (ACDRenderTargetImp*)(param);
						break;
					}


                // case ACD_RASTER_... <- add here future additional rasterization states.
                default:
                    panic("ACDDeviceImp","restoreStoredStateItem()","Unknown rasterization state");
            }
        }
    }
    else
        panic("ACDDeviceImp","restoreStoredStateItem","Unexpected raster state id");

    GLOBALPROFILER_EXITREGION()
}

#undef CAST_TO_UINT
#undef CAST_TO_BOOL
#undef CAST_TO_VECT4

ACDStoredState* ACDDeviceImp::saveAllDeviceState() const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    ACDStoredStateImp* ret = new ACDStoredStateImp();

    for (acd_uint i = 0; i < ACD_DEV_LAST; i++)
        ret->addStoredStateItem(createStoredStateItem(ACD_STORED_ITEM_ID(i)));

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

void ACDDeviceImp::restoreAllDeviceState(const ACDStoredState* state)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    const ACDStoredStateImp* ssi = static_cast<const ACDStoredStateImp*>(state);

    std::list<const StoredStateItem*> ssiList = ssi->getSSIList();

    std::list<const StoredStateItem*>::const_iterator iter = ssiList.begin();

    while ( iter != ssiList.end() )
    {
        const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(*iter);
        restoreStoredStateItem(acdssi);
        iter++;
    }

    GLOBALPROFILER_EXITREGION()
}

void ACDDeviceImp::copyMipmap (ACDTexture* inTexture, acdlib::ACD_CUBEMAP_FACE inFace, acd_uint inMipmap, acd_uint inX, acd_uint inY, acd_uint inWidth, acd_uint inHeight, 
                               ACDTexture* outTexture, acdlib::ACD_CUBEMAP_FACE outFace, acd_uint outMipmap, acd_uint outX, acd_uint outY, acd_uint outWidth, acd_uint outHeight, 
							   ACD_TEXTURE_FILTER minFilter, ACD_TEXTURE_FILTER magFilter)
{

    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

	TextureAdapter adaptedIn(inTexture);
	TextureAdapter adaptedOut(outTexture);
	ACD_FORMAT texFormat;

	if(adaptedIn.getFormat(inFace, inMipmap) != adaptedOut.getFormat(outFace, outMipmap))
		panic("ACDDevice","copyMipmap","Different format beetween input and output textures"); // Maybe it is possible to perform a conversion in case textures have different formats
    else
        texFormat = adaptedIn.getFormat(inFace, inMipmap);

	if(inX < 0 || inY < 0)
		panic("ACDDevice","copyMipmap","Start point of the portion to copy is outside the mipmap range of the input texture");
			
	if(outX < 0 || outY < 0)
		panic("ACDDevice","copyMipmap","Start point of the portion to copy is outside the mipmap range of the output texture");
	
	if(adaptedIn.getWidth(inFace, inMipmap) > inX + inWidth || adaptedIn.getHeight(inFace, inMipmap) > inY + inHeight)
		panic("ACDDevice","copyMipmap","The region to copy of the input texture is partially or totally out of the texture mipmap");
		
	if(adaptedOut.getWidth(outFace, outMipmap) > outX + outWidth || adaptedOut.getHeight(outFace, outMipmap) > outY + outHeight)
		panic("ACDDevice","copyMipmap","The region to copy of the output texture is partially or totally out of the texture mipmap");	
		
    ACDRenderTargetImp* outRT = new ACDRenderTargetImp(this, outTexture, ACD_RT_DIMENSION_TEXTURE2D, inFace, inMipmap);
    
    gpu3d::GPURegData data;
    vector<gpu3d::GPURegData> savedRegState;
    vector<const StoredStateItem*> storedState;


    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_VIEWPORT));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_TEST));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_X));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_Y));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_WIDTH));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_HEIGHT));

    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR_WRITE_ENABLED));

    storedState.push_back(createStoredStateItem(ACD_DEV_FRONT_BUFFER));
    storedState.push_back(createStoredStateItem(ACD_DEV_BACK_BUFFER));
    storedState.push_back(createStoredStateItem(ACD_DEV_PRIMITIVE));
    storedState.push_back(createStoredStateItem(ACD_DEV_VERTEX_THREAD_RESOURCES));
    storedState.push_back(createStoredStateItem(ACD_DEV_FRAGMENT_THREAD_RESOURCES));
    storedState.push_back(createStoredStateItem(ACD_DEV_STREAM_COUNT));
    storedState.push_back(createStoredStateItem(ACD_DEV_STREAM_START));
    storedState.push_back(createStoredStateItem(ACD_DEV_INDEX_MODE));
    storedState.push_back(createStoredStateItem(ACD_DEV_VSH));
    storedState.push_back(createStoredStateItem(ACD_DEV_FSH));

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_VERTEX_ATTRIBUTE_MAP + i)));

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE + i)));

    for (acd_uint i = 0; i < gpu3d::MAX_FRAGMENT_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES + i)));

    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_STRIDE));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_ELEMENTS));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_DATA_TYPE));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_BUFFER));

    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_ENABLED));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_CLAMP_S));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_CLAMP_T));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_CLAMP_R));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MIN_FILTER));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MAG_FILTER));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MIN_LOD));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MAX_LOD));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_LOD_BIAS));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_UNIT_LOD_BIAS));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_TEXTURE));

    _driver->readGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, data);
    savedRegState.push_back(data);

    acd_uint inWidthTexture = adaptedIn.getWidth(inFace, inMipmap);
    acd_uint inHeightTexture = adaptedIn.getHeight(inFace, inMipmap);

    acd_uint outWidthTexture = adaptedOut.getWidth(inFace, inMipmap);
    acd_uint outHeightTexture = adaptedOut.getHeight(inFace, inMipmap);


    acdlib::acd_float dataBuffer[] = {  
        (acdlib::acd_float)((outX*2)/outWidthTexture)-1,
        (acdlib::acd_float)((outY)*2)/outHeightTexture-1,
        0.f, 
        (acdlib::acd_float)inX/inWidthTexture,
        (acdlib::acd_float)(inY+inHeight)/inHeightTexture,
        (acdlib::acd_float)(((outX+outWidth)*2)/outWidthTexture)-1,
        (acdlib::acd_float)((outY)*2)/outHeightTexture-1,
        0.f,
        (acdlib::acd_float)(inX+inWidth)/inWidthTexture,
        (acdlib::acd_float)(inY+inHeight)/inHeightTexture,
        (acdlib::acd_float)(((outX+outWidth)*2)/outWidthTexture)-1,
        (acdlib::acd_float)(((outY+outHeight)*2)/outHeightTexture)+1, 0.f,
        (acdlib::acd_float)(inX+inWidth)/inWidthTexture,
        (acdlib::acd_float)inY/inHeightTexture,
        (acdlib::acd_float)((outX*2)/outWidthTexture)-1,
        (acdlib::acd_float)(((outY+outHeight)*2)/outHeightTexture+1), 0.f,
        (acdlib::acd_float)inX/inWidthTexture,
        (acdlib::acd_float)inY/inHeightTexture};

    acdlib::acd_uint indexBuffer[] = {0, 1, 2, 2, 3, 0};

    acdlib::acd_ubyte vertexShader[] = "mov o0, i0\n"
                                       "move o5, i1\n";
    
    acdlib::acd_ubyte fragmentShader[] = "tex r0, i5, t0\n"
                                         "mov o1, r0\n";
    
    ACDBuffer* dataACDBuffer = createBuffer(sizeof(acdlib::acd_float) * 5 * 4, (acdlib::acd_ubyte *)&dataBuffer);


    ACD_STREAM_DESC streamDesc;
    streamDesc.offset = 0;
    streamDesc.stride = sizeof(acdlib::acd_float) * 5;
    streamDesc.components = 3;
    streamDesc.frequency = 0;
    streamDesc.type = ACD_SD_FLOAT32;

    // Set the vertex buffer to ACD
    stream(0).set(dataACDBuffer, streamDesc);
    enableVertexAttribute(0, 0);


    streamDesc.offset = sizeof(acdlib::acd_float) * 3;
    streamDesc.stride = sizeof(acdlib::acd_float) * 5;
    streamDesc.components = 2;
    streamDesc.frequency = 0;
    streamDesc.type = acdlib::ACD_SD_FLOAT32;

    // Set the vertex buffer to ACD
    stream(1).set(dataACDBuffer, streamDesc);
    enableVertexAttribute(1, 1);


    ACDBuffer* indexACDBuffer = createBuffer((sizeof (acdlib::acd_uint)) * 6, (acdlib::acd_ubyte *)&indexBuffer);
    setIndexBuffer(indexACDBuffer, 0, acdlib::ACD_SD_UINT32);


    // Set source surface as a Texture
    sampler(0).setEnabled(true);
    sampler(0).setTexture(inTexture);
    sampler(0).setMagFilter(magFilter);
    sampler(0).setMinFilter(minFilter);
    sampler(0).setMaxAnisotropy(1);
    sampler(0).setTextureAddressMode(ACD_TEXTURE_S_COORD, ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);
    sampler(0).setTextureAddressMode(ACD_TEXTURE_T_COORD, ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);
    sampler(0).setTextureAddressMode(ACD_TEXTURE_R_COORD, ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);

    ACDShaderProgram* vertexProgram = createShaderProgram();
    ACDShaderProgram* fragmentProgram = createShaderProgram();

    vertexProgram->setProgram(vertexShader);
    fragmentProgram->setProgram(fragmentShader);

    setVertexShader(vertexProgram);
    setFragmentShader(fragmentProgram);

    setRenderTarget(0, outRT);
    setZStencilBuffer(NULL);

    rast().setCullMode(acdlib::ACD_CULL_NONE);
    setPrimitive(acdlib::ACD_TRIANGLES);


    drawIndexed(0, 6, 0, 0, 0);

    acd_int nextReg = 0;

    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);

    _blending->restoreStoredStateItem(storedState[nextReg++]);

    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_FRAGMENT_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);

    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);

    _driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, savedRegState[0]);



}

void ACDDeviceImp::copyTexture2RenderTarget(ACDTexture2DImp* texture, ACDRenderTargetImp* renderTarget)
{
    gpu3d::GPURegData data;
    vector<gpu3d::GPURegData> savedRegState;
    vector<const StoredStateItem*> storedState;


    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_VIEWPORT));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_TEST));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_X));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_Y));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_WIDTH));
    storedState.push_back(_rast->createStoredStateItem(ACD_RASTER_SCISSOR_HEIGHT));

    storedState.push_back(_blending->createStoredStateItem(ACD_BLENDING_COLOR_WRITE_ENABLED));

    storedState.push_back(createStoredStateItem(ACD_DEV_FRONT_BUFFER));
    storedState.push_back(createStoredStateItem(ACD_DEV_BACK_BUFFER));
    storedState.push_back(createStoredStateItem(ACD_DEV_PRIMITIVE));
    storedState.push_back(createStoredStateItem(ACD_DEV_VERTEX_THREAD_RESOURCES));
    storedState.push_back(createStoredStateItem(ACD_DEV_FRAGMENT_THREAD_RESOURCES));
    storedState.push_back(createStoredStateItem(ACD_DEV_STREAM_COUNT));
    storedState.push_back(createStoredStateItem(ACD_DEV_STREAM_START));
    storedState.push_back(createStoredStateItem(ACD_DEV_INDEX_MODE));
    storedState.push_back(createStoredStateItem(ACD_DEV_VSH));
    storedState.push_back(createStoredStateItem(ACD_DEV_FSH));

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_VERTEX_ATTRIBUTE_MAP + i)));

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE + i)));

    for (acd_uint i = 0; i < gpu3d::MAX_FRAGMENT_ATTRIBUTES; i++)
        storedState.push_back(createStoredStateItem(ACD_STORED_ITEM_ID(ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES + i)));

    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_STRIDE));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_ELEMENTS));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_DATA_TYPE));
    storedState.push_back(_stream[0]->createStoredStateItem(ACD_STREAM_BUFFER));

    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_ENABLED));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_CLAMP_S));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_CLAMP_T));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_CLAMP_R));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MIN_FILTER));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MAG_FILTER));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MIN_LOD));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_MAX_LOD));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_LOD_BIAS));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_UNIT_LOD_BIAS));
    storedState.push_back(_sampler[0]->createStoredStateItem(ACD_SAMPLER_TEXTURE));

    _driver->readGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, data);
    savedRegState.push_back(data);


    acdlib::acd_float dataBuffer[] = {  -1.f, -1.f, 0.f, 0.f, 1.f,
                                        1.f, -1.f, 0.f, 1.f, 1.f,
                                        1.f, 1.f, 0.f, 1.f, 0.f,
                                        -1.f, 1.f, 0.f, 0.f, 0.f};

    acdlib::acd_uint indexBuffer[] = {0, 1, 2, 2, 3, 0};

    acdlib::acd_ubyte vertexShader[] = "mov o0, i0\n"
                                       "mov o5, i1\n";
                                       
    acdlib::acd_ubyte fragmentShader[] = "tex r0, i5, t0\n"
                                         "mov o1, r0\n";

    ACDBuffer* dataACDBuffer = createBuffer(sizeof(acdlib::acd_float) * 5 * 4, (acdlib::acd_ubyte *)&dataBuffer);


    ACD_STREAM_DESC streamDesc;
    streamDesc.offset = 0;
    streamDesc.stride = sizeof(acdlib::acd_float) * 5;
    streamDesc.components = 3;
    streamDesc.frequency = 0;
    streamDesc.type = ACD_SD_FLOAT32;

    // Set the vertex buffer to ACD
    stream(0).set(dataACDBuffer, streamDesc);
    enableVertexAttribute(0, 0);


    streamDesc.offset = sizeof(acdlib::acd_float) * 3;
    streamDesc.stride = sizeof(acdlib::acd_float) * 5;
    streamDesc.components = 2;
    streamDesc.frequency = 0;
    streamDesc.type = acdlib::ACD_SD_FLOAT32;

    // Set the vertex buffer to ACD
    stream(1).set(dataACDBuffer, streamDesc);
    enableVertexAttribute(1, 1);


    ACDBuffer* indexACDBuffer = createBuffer((sizeof (acdlib::acd_uint)) * 6, (acdlib::acd_ubyte *)&indexBuffer);
    setIndexBuffer(indexACDBuffer, 0, acdlib::ACD_SD_UINT32);


    // Set source surface as a Texture
    sampler(0).setEnabled(true);
    sampler(0).setTexture(texture);
    sampler(0).setMagFilter(ACD_TEXTURE_FILTER_NEAREST);
    sampler(0).setMinFilter(ACD_TEXTURE_FILTER_NEAREST);
    sampler(0).setMaxAnisotropy(1);
    sampler(0).setTextureAddressMode(ACD_TEXTURE_S_COORD, ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);
    sampler(0).setTextureAddressMode(ACD_TEXTURE_T_COORD, ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);
    sampler(0).setTextureAddressMode(ACD_TEXTURE_R_COORD, ACD_TEXTURE_ADDR_CLAMP_TO_EDGE);

    ACDShaderProgram* vertexProgram = createShaderProgram();
    ACDShaderProgram* fragmentProgram = createShaderProgram();

    vertexProgram->setProgram(vertexShader);
    fragmentProgram->setProgram(fragmentShader);

    setVertexShader(vertexProgram);
    setFragmentShader(fragmentProgram);

    setRenderTarget(0, renderTarget);
    setZStencilBuffer(NULL);

    rast().setCullMode(acdlib::ACD_CULL_NONE);
    setPrimitive(acdlib::ACD_TRIANGLES);


    drawIndexed(0, 6, 0, 0, 0);

    acd_int nextReg = 0;

    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);
    _rast->restoreStoredStateItem(storedState[nextReg++]);

    _blending->restoreStoredStateItem(storedState[nextReg++]);

    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);
    restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_VERTEX_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    for (acd_uint i = 0; i < gpu3d::MAX_FRAGMENT_ATTRIBUTES; i++)
        restoreStoredStateItem(storedState[nextReg++]);

    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);
    _stream[0]->restoreStoredStateItem(storedState[nextReg++]);

    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);
    _sampler[0]->restoreStoredStateItem(storedState[nextReg++]);

    _driver->writeGPURegister(gpu3d::GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, gpu3d::COLOR_ATTRIBUTE, savedRegState[0]);
}

ACDRenderTarget* ACDDeviceImp::getFrontBufferRT()
{
    return _defaultFrontBuffer;
}

ACDRenderTarget* ACDDeviceImp::getBackBufferRT()
{
    return _defaultBackBuffer;
}

acd_uint ACDDeviceImp::getCurrentFrame()
{
    return _currentFrame;
}

acd_uint ACDDeviceImp::getCurrentBatch()
{
    return _currentBatch;
}

void ACDDeviceImp::performBlitOperation2D(acd_uint samplerID, acd_uint xoffset, acd_uint yoffset,
                                          acd_uint x, acd_uint y, acd_uint width, acd_uint height,
                                          acd_uint textureWidth, ACD_FORMAT internalFormat,
                                          ACDTexture2D* texture, acd_uint level)
{
    //  Syncronize render buffer state.
    _syncRenderBuffers();
    
    //  Perform the blit operation.
    sampler(samplerID).performBlitOperation2D(xoffset, yoffset, x, y, width, height, textureWidth, internalFormat, texture, level);
}

void ACDDeviceImp::setColorSRGBWrite(acdlib::acd_bool enable)
{
    //  Set the color conversion from linear to sRGB on color write flag.
    _colorSRGBWrite = enable;
}

acd_ubyte* ACDDeviceImp::compressTexture(ACD_FORMAT originalFormat, ACD_FORMAT compressFormat, acd_uint width, acd_uint height, acd_ubyte* originalData, acd_uint selectionMode)
{
    acd_ubyte* data;
    acd_ubyte* compressedData = 0;
    acd_uint maxWidthBlocks = std::floor(acd_double(width/4));
    acd_uint maxHeightBlocks = std::floor(acd_double(height/4));
    acd_uint bestDifference = -1;
    acd_uint difference;
    acd_uint pixelsRowSize;
    acd_uint code = 0;
    acd_uint k;
    acd_ubyte color0_R, color0_G, color0_B;
    acd_ubyte color1_R, color1_G, color1_B;


    switch (compressFormat)
    {

        case ACD_COMPRESSED_S3TC_DXT1_RGB:
            {
            if (originalFormat != ACD_FORMAT_RGBA_8888 && originalFormat != ACD_FORMAT_RGB_888)
                panic("ACDTexture2DImp", "compressTexture", "By the time you can only compress ACD_FORMAT_RGBA_8888 textures");

            compressedData = new acd_ubyte[maxWidthBlocks*maxHeightBlocks*8];

            k = 0;
            code = 0;
            
            pixelsRowSize = width * TextureMipmap::getTexelSize(originalFormat);

            for (acd_uint j = 0; j < maxHeightBlocks; j++) 
                for (acd_uint i = 0; i < maxWidthBlocks; i++)
                {

                    data = originalData + (acd_uint)(TextureMipmap::getTexelSize(originalFormat) * (width * j * 4 + i * 4));

                    switch (selectionMode)
                    {
                        case 0:
                            {
                                // In this mode color are selected statically. upper left corner and lower right corner

                                acd_uint texel = 0;
                                acd_uint compare = 15;

                                // Upper left corner
                                color0_R = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat))]);
                                color0_G = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
                                color0_B = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);

                                // Lower right corner
                                color1_R = (((acd_ubyte* )(data+(pixelsRowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat))]);
                                color1_G = (((acd_ubyte* )(data+(pixelsRowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
                                color1_B = (((acd_ubyte* )(data+(pixelsRowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);
                            }
                            break;
                        case 1:
                        default:
                            {
                                for(acd_uint texel = 0; texel < 16; texel++)
                                    for(acd_uint compare = 0; compare < 16; compare++)
                                    {
                                        
                                        if (texel == compare)
                                            continue;

                                        difference = compressionDifference(texel, compare, pixelsRowSize, data, originalFormat);

                                        if (difference < bestDifference || bestDifference == -1)
                                        {
                                            bestDifference = difference;

                                            //getColor0
                                            color0_R = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat))]);
                                            color0_G = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
                                            color0_B = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);

                                            //getColor1
                                            color1_R = (((acd_ubyte* )(data+(pixelsRowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat))]);
                                            color1_G = (((acd_ubyte* )(data+(pixelsRowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
                                            color1_B = (((acd_ubyte* )(data+(pixelsRowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);

                                        }
                                    }
                            }
                    } 

                acd_ushort color0_R5G6B5, color1_R5G6B5;
                acd_ubyte texel_R, texel_G, texel_B;
                acd_uint tryR, tryG, tryB;
                acd_uint diffA, diffB, diffC, diffD;
                acd_ushort texel_R5G6B5;

                color0_R5G6B5 = (((acd_ushort)(color0_R & 0xF8)) << 8) | (((acd_ushort)(color0_G & 0xFC)) << 3) | (((acd_ushort)(color0_B & 0xF8)) >> 3);
                color1_R5G6B5 = (((acd_ushort)(color1_R & 0xF8)) << 8) | (((acd_ushort)(color1_G & 0xFC)) << 3) | (((acd_ushort)(color1_B & 0xF8)) >> 3);

                for (acd_uint texel = 0; texel < 16; texel++)
                {
                    texel_R = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat))]);
                    texel_G = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
                    texel_B = (((acd_ubyte* )(data+(pixelsRowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);

                    texel_R5G6B5 = (((acd_ushort)(texel_R & 0xF8)) << 8) | (((acd_ushort)(texel_G & 0xFC)) << 3) | (((acd_ushort)(texel_B & 0xF8)) >> 3);

                    if (color0_R5G6B5 < color1_R5G6B5)
                    {
                        diffA = abs((int)(color0_R-texel_R)) + abs((int)(color0_G-texel_G)) + abs((int)(color0_B-texel_B));
                        diffB = abs((int)(color1_R-texel_R)) + abs((int)(color1_G-texel_G)) + abs((int)(color1_B-texel_B));

                        tryR = 2/3*color0_R + color1_R/3;
                        tryG = 2/3*color0_G + color1_G/3;
                        tryB = 2/3*color0_B + color1_B/3;

                        diffC = abs((int)(tryR-texel_R)) + abs((int)(tryG-texel_G)) + abs((int)(tryB-texel_B));

                        tryR = color0_R/3 + 2/3*color1_R;
                        tryG = color0_G/3 + 2/3*color1_G;
                        tryB = color0_B/3 + 2/3*color1_B;

                        diffD = abs((int)(tryR-texel_R)) + abs((int)(tryG-texel_G)) + abs((int)(tryB-texel_B));

                        if (diffA <= diffB && diffA <= diffC && diffA <= diffD )
                            code = code | (0x00 << 2*texel);
                        else if (diffB <= diffA && diffB <= diffC && diffB <= diffD)
                            code = code | (0x01 << 2*texel);
                        else if (diffC <= diffA && diffC <= diffB && diffC <= diffD)
                            code = code | (0x02 << 2*texel);
                        else
                            code = code | (0x03 << 2*texel);
                    }
                    else
                    {
                        diffA = abs((int)(color0_R-texel_R)) + abs((int)(color0_G-texel_G)) + abs((int)(color0_B-texel_B));
                        diffB = abs((int)(color1_R-texel_R)) + abs((int)(color1_G-texel_G)) + abs((int)(color1_B-texel_B));

                        tryR = (color0_R + color1_R)/2;
                        tryG = (color0_G + color1_G)/2;
                        tryB = (color0_B + color1_B)/2;

                        diffC = abs((int)(tryR-texel_R)) + abs((int)(tryG-texel_G)) + abs((int)(tryB-texel_B));

                        diffD = texel_R + texel_G + texel_B;

                        if (diffA <= diffB && diffA <= diffC && diffA <= diffD )
                            code = code | (0x00 << 2*texel);
                        else if (diffB <= diffA && diffB <= diffC && diffB <= diffD)
                            code = code | (0x01 << 2*texel);
                        else if (diffC <= diffA && diffC <= diffB && diffC <= diffD)
                            code = code | (0x02 << 2*texel);
                        else
                            code = code | (0x03 << 2*texel);
                    }
                }

                /*cout << (acd_uint)(color0_R5G6B5) << endl;
                cout << (acd_uint)(color1_R5G6B5) << endl;
                cout << code << endl;*/
                compressedData[k] = color0_R5G6B5 & 0xFF;
                compressedData[k+1] = (color0_R5G6B5 & 0xFF00) >> 8;
                compressedData[k+2] = color1_R5G6B5 & 0xFF;
                compressedData[k+3] = (color1_R5G6B5 & 0xFF00) >> 8;
                compressedData[k+4] = code & 0xFF;
                compressedData[k+5] = (code & 0xFF00) >> 8;
                compressedData[k+6] = (code & 0xFF0000) >> 16;
                compressedData[k+7] = (code & 0xFF000000) >> 24;

                code = 0;
                bestDifference = -1;
                k = k + 8;

            }


        }
            break;

        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
             panic("ACDTexture2DImp", "compressTexture", "111By the time you can only compress to ACD_COMPRESSED_S3TC_DXT1_RGB textures");
        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
            panic("ACDTexture2DImp", "compressTexture", "222By the time you can only compress to ACD_COMPRESSED_S3TC_DXT1_RGB textures");
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:
        default:
            panic("ACDTexture2DImp", "compressTexture", "333By the time you can only compress to ACD_COMPRESSED_S3TC_DXT1_RGB textures");
            break;

    }

    return (acd_ubyte*)compressedData;
}

acd_uint ACDDeviceImp::compressionDifference(acd_uint texel, acd_uint compare, acd_uint rowSize, acd_ubyte* data, ACD_FORMAT originalFormat)
{

    acd_uint i = 0;

    acd_uint diffA, diffB, diffC, diffD;
    acd_uint totalDiff = 0;
    acd_ushort tryR, tryG, tryB;
    acd_ushort rc, gc, bc;
    acd_ushort rt, gt, bt;
    acd_ushort r,g,b;
    acd_ushort texelR5G6B5, compareR5G6B5;

    // Obtain each color component to obtain R5G6B5 format
    rt = (((acd_ubyte* )(data+(rowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat))]);
    gt = (((acd_ubyte* )(data+(rowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
    bt = (((acd_ubyte* )(data+(rowSize*(texel/4))))[(acd_uint)((texel%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);
    texelR5G6B5 = (((rt & 0xF8) << 8 ) | ((gt & 0xFC) << 3) | ((bt & 0xF8) >> 3)); 

    // Obtain each color component to obtain R5G6B5 format
    rc = (((acd_ubyte* )(data+(rowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat))]);
    gc = (((acd_ubyte* )(data+(rowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat)+1)]);
    bc = (((acd_ubyte* )(data+(rowSize*(compare/4))))[(acd_uint)((compare%4)*TextureMipmap::getTexelSize(originalFormat)+2)]);
    compareR5G6B5 = (((rc & 0xF8) << 8) | ((gc & 0xFC) << 3) | ((bc & 0xF8) >> 3)); 

    if (texelR5G6B5 < compareR5G6B5)
    {
        for (acd_uint height = 0; height < 4; height++)
        {
            for (acd_uint width = 0; width < 4; width++)
            {
                r = data[(acd_uint)(width*TextureMipmap::getTexelSize(originalFormat))];
                g = data[(acd_uint)(width*TextureMipmap::getTexelSize(originalFormat)+1)];
                b = data[(acd_uint)(width*TextureMipmap::getTexelSize(originalFormat)+2)];

                diffA = abs((int)(rt-r)) + abs((int)(gt-g)) + abs((int)(bt-b));
                diffB = abs((int)(rc-r)) + abs((int)(gc-g)) + abs((int)(bc-b));

                tryR = 2/3*rt + rc/3;
                tryG = 2/3*gt + gc/3;
                tryB = 2/3*bt + bc/3;

                diffC = abs((int)(tryR-r)) + abs((int)(tryG-g)) + abs((int)(tryB-b));

                tryR = rt/3 + 2/3*rc;
                tryG = gt/3 + 2/3*gc;
                tryB = bt/3 + 2/3*bc;

                diffD = abs((int)(tryR-r)) + abs((int)(tryG-g)) + abs((int)(tryB-b));

                if (diffA <= diffB && diffA <= diffC && diffA <= diffD )
                    totalDiff += diffA;
                else if (diffB <= diffA && diffB <= diffC && diffB <= diffD)
                    totalDiff += diffB;
                else if (diffC <= diffA && diffC <= diffB && diffC <= diffD)
                    totalDiff += diffC;
                else
                    totalDiff += diffD;

            }

            data += rowSize;
        }
    }
    else
    {
        for (acd_uint height = 0; height < 4; height++)
        {
            for (acd_uint width = 0; width < 4; width++)
            {
                r = data[(acd_uint)(width*TextureMipmap::getTexelSize(originalFormat))];
                g = data[(acd_uint)(width*TextureMipmap::getTexelSize(originalFormat)+1)];
                b = data[(acd_uint)(width*TextureMipmap::getTexelSize(originalFormat)+2)];

                tryR = (rt + r)/2;
                tryG = (bt + g)/2;
                tryB = (gt + b)/2;

                diffA = abs((int)(rt-r)) + abs((int)(gt-g)) + abs((int)(bt-b));
                diffB = abs((int)(rc-r)) + abs((int)(gc-g)) + abs((int)(bc-b));
                diffC = abs((int)(tryR-r)) + abs((int)(tryG-g)) + abs((int)(tryB-b));
                diffD = r + g + b;

                if (diffA <= diffB && diffA <= diffC && diffA <= diffD )
                    totalDiff += diffA;
                else if (diffB <= diffA && diffB <= diffC && diffB <= diffD)
                    totalDiff += diffB;
                else if (diffC <= diffA && diffC <= diffB && diffC <= diffD)
                    totalDiff += diffC;
                else
                    totalDiff += diffD;

            }

            data += rowSize;
        }

    }

    return totalDiff;

}
