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

#include "ACDRasterizationStageImp.h"
#include <iostream>
#include <sstream>

#include "GlobalProfiler.h"

#include "ACDStoredStateImp.h"

using namespace acdlib;
using namespace std;

ACDRasterizationStageImp::ACDRasterizationStageImp(ACDDeviceImp* device, GPUDriver* driver) : 
    _device(device), _driver(driver), _syncRequired(true),
    _fillMode(ACD_FILL_SOLID),
    _cullMode(ACD_CULL_NONE),
    _faceMode(ACD_FACE_CCW),
    _xViewport(0),
    _yViewport(0),
    _widthViewport(0),
    _heightViewport(0),
    _interpolation(gpu3d::MAX_FRAGMENT_ATTRIBUTES, ACD_INTERPOLATION_LINEAR),
    _scissorEnabled(false),
    _xScissor(0),
    _yScissor(0),
    _widthScissor(400),
    _heightScissor(400),
    _useD3D9RasterizationRules(false),
    _useD3D9PixelCoordConvention(false)
{
    forceSync();
}

void ACDRasterizationStageImp::setInterpolationMode(acd_uint fshInputAttribute, ACD_INTERPOLATION_MODE mode)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    if ( fshInputAttribute >= _interpolation.size() )
        panic("ACDRasterizationStageImp", "setInterpolationMode", "Fragment shader input attribute out of range");

    _interpolation[fshInputAttribute] = mode;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::enableScissor(acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _scissorEnabled = enable;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::setFillMode(ACD_FILL_MODE fillMode)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _fillMode = fillMode;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::setCullMode(ACD_CULL_MODE cullMode)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _cullMode = cullMode;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::setFaceMode(ACD_FACE_MODE faceMode)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _faceMode = faceMode;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::useD3D9RasterizationRules(acd_bool use)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _useD3D9RasterizationRules = use;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::useD3D9PixelCoordConvention(acd_bool use)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _useD3D9PixelCoordConvention = use;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::setViewport(acd_int x, acd_int y, acd_uint width, acd_uint height)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _xViewport = x;
    _yViewport = y;
    _widthViewport = width;
    _heightViewport = height;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::setScissor(acd_int x, acd_int y, acd_uint width, acd_uint height)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _xScissor = x;
    _yScissor= y;
    _widthScissor = width;
    _heightScissor = height;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::getViewport(acd_int &x, acd_int &y, acd_uint &width, acd_uint &height)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    x = _xViewport;
    y = _yViewport;
    width = _widthViewport;
    height = _heightViewport;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::getScissor(acd_bool &enabled, acd_int &x, acd_int &y, acd_uint &width, acd_uint &height)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    enabled = _scissorEnabled;
    x = _xScissor;
    y = _yScissor;
    width = _widthScissor;
    height = _heightScissor;
    GLOBALPROFILER_EXITREGION()
}

void ACDRasterizationStageImp::sync()
{
    gpu3d::GPURegData data;
    
    // Fill mode not sync. Atila GPU doesn´t support wireframe mode
    if ( _fillMode.changed() || _syncRequired ) {
        if (_fillMode == ACD_FILL_WIREFRAME)
            panic("ACDRasterizationStageImp","sync","Wireframe mode not supported natively");

        _fillMode.restart();
    }
    if ( _cullMode.changed() || _syncRequired ) {
        _getGPUCullMode(_cullMode, &data);
        _driver->writeGPURegister(gpu3d::GPU_CULLING, data);
        _cullMode.restart();
    }
    if ( _faceMode.changed() || _syncRequired ) {
        _getGPUFaceMode(_faceMode, &data);
        _driver->writeGPURegister(gpu3d::GPU_FACEMODE, data);
        _faceMode.restart();
    }
    if (_useD3D9RasterizationRules.changed() || _syncRequired)
    {
        data.booleanVal = _useD3D9RasterizationRules;
        _driver->writeGPURegister(gpu3d::GPU_D3D9_RASTERIZATION_RULES, data);
        _useD3D9RasterizationRules.restart();
    }    
    if (_useD3D9PixelCoordConvention.changed() || _syncRequired)
    {
        data.booleanVal = _useD3D9PixelCoordConvention;
        _driver->writeGPURegister(gpu3d::GPU_D3D9_PIXEL_COORDINATES, data);
        _useD3D9PixelCoordConvention.restart();
    }    
    if ( _xViewport.changed() || _syncRequired ) {
        data.intVal = _xViewport;
        _driver->writeGPURegister(gpu3d::GPU_VIEWPORT_INI_X, data);
        _xViewport.restart();
    }
    if ( _yViewport.changed() || _syncRequired ) {
        data.intVal = _yViewport;
        _driver->writeGPURegister(gpu3d::GPU_VIEWPORT_INI_Y, data);
        _yViewport.restart();
    }
    if ( _widthViewport.changed() || _syncRequired ) {
        data.uintVal = _widthViewport;
        _driver->writeGPURegister(gpu3d::GPU_VIEWPORT_WIDTH, data);
        _widthViewport.restart();
    }
    if ( _heightViewport.changed() || _syncRequired ) {
        data.uintVal = _heightViewport;
        _driver->writeGPURegister(gpu3d::GPU_VIEWPORT_HEIGHT, data);
        _heightViewport.restart();
    }

    const acd_uint iCount = _interpolation.size();
    for ( acd_uint i = 0; i < iCount; ++i ) {
        if ( _interpolation[i].changed() || _syncRequired ) {
            _getGPUInterpolationMode(_interpolation[i], &data);
            _driver->writeGPURegister(gpu3d::GPU_INTERPOLATION, i, data);
            _interpolation[i].restart();
        }
    }

    if ( _scissorEnabled.changed() || _syncRequired ) {
        data.booleanVal = _scissorEnabled;
        _driver->writeGPURegister( gpu3d::GPU_SCISSOR_TEST, data );
        _scissorEnabled.restart();
    }

    if ( _xScissor.changed() || _syncRequired ) {
        data.intVal = _xScissor;
        _driver->writeGPURegister( gpu3d::GPU_SCISSOR_INI_X, data );
        _xScissor.restart();
    }

    if ( _yScissor.changed() || _syncRequired ) {
        data.intVal = _yScissor;
        _driver->writeGPURegister( gpu3d::GPU_SCISSOR_INI_Y, data );
        _yScissor.restart();
    }

    if ( _widthScissor.changed() || _syncRequired ) {
        data.uintVal = _widthScissor;
        _driver->writeGPURegister( gpu3d::GPU_SCISSOR_WIDTH, data );
        _widthScissor.restart();
    }
    
    if ( _heightScissor.changed() || _syncRequired ) {
        data.uintVal = _heightScissor;
        _driver->writeGPURegister( gpu3d::GPU_SCISSOR_HEIGHT, data );
        _heightScissor.restart();
    }
}

void ACDRasterizationStageImp::forceSync()
{
    _syncRequired = true;
    sync();
    _syncRequired = false;
}

void ACDRasterizationStageImp::_getGPUCullMode(ACD_CULL_MODE mode, gpu3d::GPURegData* data)
{
    switch ( mode )
    {
        case ACD_CULL_NONE:
            data->culling = gpu3d::NONE;
            break;
        case ACD_CULL_FRONT:
            data->culling = gpu3d::FRONT;
            break;
        case ACD_CULL_BACK:
            data->culling = gpu3d::BACK;
            break;
        case ACD_CULL_FRONT_AND_BACK:
            data->culling = gpu3d::FRONT_AND_BACK;
            break;
        default:
            panic("ACDRasterizationStageImp", "setCullMode", "Unknown cull mode");
    }    
}

void ACDRasterizationStageImp::_getGPUFaceMode(ACD_FACE_MODE mode, gpu3d::GPURegData* data)
{
    switch ( mode )
    {
        case ACD_FACE_CW:
            data->faceMode = gpu3d::GPU_CW;
            break;
        case ACD_FACE_CCW:
            data->faceMode = gpu3d::GPU_CCW;
            break;
        default:
            panic("ACDRasterizationStageImp", "setCullMode", "Unknown cull mode");
    }    
}

void ACDRasterizationStageImp::_getGPUInterpolationMode(ACD_INTERPOLATION_MODE mode, gpu3d::GPURegData* data)
{
    switch ( mode ) {
        case ACD_INTERPOLATION_NONE:
            data->booleanVal = false;
            break;
        case ACD_INTERPOLATION_LINEAR:
            data->booleanVal = true;
            break;
        case ACD_INTERPOLATION_CENTROID:
            panic("ACDRasterizationStageImp","_getGPUInterpolationMode", "ACD_INTERPOLATION_CENTROID not supported yet");
            break;
        case ACD_INTERPOLATION_NOPERSPECTIVE:
            panic("ACDRasterizationStageImp","_getGPUInterpolationMode", "ACD_INTERPOLATION_NOPERSPECTIVE supported yet");
            break;
        default:
            panic("ACDRasterizationStageImp","_getGPUInterpolationMode", "Unknown interpolation mode");
    }
}

const char* ACDRasterizationStageImp::CullModePrint::print(const ACD_CULL_MODE& cullMode) const
{
    switch ( cullMode )
    {
        case ACD_CULL_NONE:
            return "NONE";
        case ACD_CULL_FRONT:
            return "FRONT";
        case ACD_CULL_BACK:
            return "BACK";
        case ACD_CULL_FRONT_AND_BACK:
            return "FRONT_AND_BACK";
        default:
            panic("ACDRasterizationStageImp::CullModePrint", "print", "Unknown cull mode");
            return ""; 
    }

};

const char* ACDRasterizationStageImp::InterpolationModePrint::print(const ACD_INTERPOLATION_MODE& iMode) const
{
    switch ( iMode ) {
        case ACD_INTERPOLATION_NONE:
            return "NONE";
        case ACD_INTERPOLATION_LINEAR:
            return "LINEAR";
        case ACD_INTERPOLATION_CENTROID:
            return "CENTROID";
        case ACD_INTERPOLATION_NOPERSPECTIVE:
            return "NOPERSPECTIVE";
        default:
            panic("ACDRasterizationStageImp::CullModePrint", "print", "Unknown interpolation mode");
            return "";
    }
}

string ACDRasterizationStageImp::getInternalState() const
{
    stringstream ss;

    ss << stateItemString(_cullMode, "CULL_MODE", false, &cullModePrint);

    // Viewport
    ss << stateItemString(_xViewport, "VIEWPORT_X", false); 
    ss << stateItemString(_yViewport, "VIEWPORT_Y", false); 
    ss << stateItemString(_widthViewport, "VIEWPORT_WIDTH", false);
    ss << stateItemString(_heightViewport, "VIEWPORT_HEIGHT", false);

    const acd_uint iCount = _interpolation.size();
    for ( acd_uint i = 0; i < iCount; ++i ) {
        stringstream aux;
        aux << "RASTER_ATTRIBUTE" << i << "_INTERPOLATION";
        ss << stateItemString(_interpolation[i], aux.str().c_str(), false, &interpolationModePrint);
    }

    return ss.str();
}

const StoredStateItem* ACDRasterizationStageImp::createStoredStateItem(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACDStoredStateItem* ret;
    acd_uint interpolation;

    if (stateId >= ACD_RASTER_FIRST_ID && stateId < ACD_RASTER_LAST)
    {  
        if ((stateId >= ACD_RASTER_INTERPOLATION) && (stateId < ACD_RASTER_INTERPOLATION + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            // It´s a blending enabled state
            interpolation = stateId - ACD_RASTER_INTERPOLATION;
            ret = new ACDSingleBoolStoredStateItem(_interpolation[interpolation]);
        }
        else 
        {
            switch(stateId)
            {
                case ACD_RASTER_FILLMODE:       ret = new ACDSingleEnumStoredStateItem((acd_enum)(_fillMode));          break;
                case ACD_RASTER_CULLMODE:       ret = new ACDSingleEnumStoredStateItem((acd_enum)(_cullMode));          break;
                case ACD_RASTER_VIEWPORT:       ret = new ACDViewportStoredStateItem(_xViewport, _yViewport, _widthViewport, _heightViewport);  break;
                case ACD_RASTER_FACEMODE:       ret = new ACDSingleEnumStoredStateItem((acd_enum)(_faceMode));          break;
                case ACD_RASTER_SCISSOR_TEST:   ret = new ACDSingleBoolStoredStateItem((acd_enum)(_scissorEnabled));    break;
                case ACD_RASTER_SCISSOR_X:      ret = new ACDSingleUintStoredStateItem((acd_enum)(_xScissor));          break;
                case ACD_RASTER_SCISSOR_Y:      ret = new ACDSingleUintStoredStateItem((acd_enum)(_yScissor));          break;
                case ACD_RASTER_SCISSOR_WIDTH:  ret = new ACDSingleUintStoredStateItem((acd_enum)(_widthScissor));      break;
                case ACD_RASTER_SCISSOR_HEIGHT: ret = new ACDSingleUintStoredStateItem((acd_enum)(_heightScissor));     break;

                // case ACD_RASTER_... <- add here future additional rasterization states.
                default:
                    panic("ACDRasterizationStageImp","createStoredStateItem()","Unknown rasterization state");
                    ret = 0;
            }
        }
    }
    else
        panic("ACDRasterizationStageImp","createStoredStateItem","Unexpected raster state id");

    ret->setItemId(stateId);

    GLOBALPROFILER_EXITREGION()
    return ret;
}


#define CAST_TO_UINT(X)     *(static_cast<const ACDSingleUintStoredStateItem*>(X))
#define CAST_TO_BOOL(X)     *(static_cast<const ACDSingleBoolStoredStateItem*>(X))

void ACDRasterizationStageImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(ssi);
    acd_uint interpolation;

    ACD_STORED_ITEM_ID stateId = acdssi->getItemId();

    if (stateId >= ACD_RASTER_FIRST_ID && stateId < ACD_RASTER_LAST)
    {  
        if ((stateId >= ACD_RASTER_INTERPOLATION) && (stateId < ACD_RASTER_INTERPOLATION + gpu3d::MAX_FRAGMENT_ATTRIBUTES))
        {    
            // It´s a blending enabled state
            interpolation = stateId - ACD_RASTER_INTERPOLATION;
             acd_enum param = *(static_cast<const ACDSingleEnumStoredStateItem*>(acdssi));
            _interpolation[interpolation] = ACD_INTERPOLATION_MODE(param);
            delete(acdssi);
        }
        else
        {
            switch(stateId)
            {
                case ACD_RASTER_FILLMODE:
                {
                    acd_enum param = *(static_cast<const ACDSingleEnumStoredStateItem*>(acdssi));
                    _fillMode = ACD_FILL_MODE(param);
                    delete(acdssi);
                    break;
                }
                case ACD_RASTER_CULLMODE:
                {
                    acd_enum param = *(static_cast<const ACDSingleEnumStoredStateItem*>(acdssi));
                    _cullMode = ACD_CULL_MODE(param);
                    delete(acdssi);
                    break;
                }
                case ACD_RASTER_VIEWPORT:
                {
                    const ACDViewportStoredStateItem* params = static_cast<const ACDViewportStoredStateItem*>(acdssi);
                    _xViewport = params->getXViewport(); 
                    _yViewport = params->getYViewport(); 
                    _widthViewport = params->getWidthViewport(); 
                    _heightViewport = params->getHeightViewport(); 
                    delete(acdssi);
                    break;
                }

                case ACD_RASTER_SCISSOR_TEST:   _scissorEnabled = CAST_TO_BOOL(acdssi);     delete(acdssi);     break;
                case ACD_RASTER_SCISSOR_X:      _xScissor = CAST_TO_UINT(acdssi);           delete(acdssi);     break;
                case ACD_RASTER_SCISSOR_Y:      _yScissor = CAST_TO_UINT(acdssi);           delete(acdssi);     break;
                case ACD_RASTER_SCISSOR_WIDTH:  _widthScissor = CAST_TO_UINT(acdssi);       delete(acdssi);     break;
                case ACD_RASTER_SCISSOR_HEIGHT: _heightScissor = CAST_TO_UINT(acdssi);      delete(acdssi);     break;
                // case ACD_RASTER_... <- add here future additional rasterization states.
                default:
                    panic("ACDRasterizationStageImp","restoreStoredStateItem()","Unknown rasterization state");
            }
        }
    }
    else
        panic("ACDRasterizationStageImp","restoreStoredStateItem","Unexpected raster state id");

    GLOBALPROFILER_EXITREGION()
}

#undef CAST_TO_UINT
#undef CAST_TO_BOOL

ACDStoredState* ACDRasterizationStageImp::saveAllState() const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    ACDStoredStateImp* ret = new ACDStoredStateImp();

    for (acd_uint i = 0; i < ACD_RASTER_LAST; i++)
        ret->addStoredStateItem(createStoredStateItem(ACD_STORED_ITEM_ID(i)));

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

void ACDRasterizationStageImp::restoreAllState(const ACDStoredState* state)
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
