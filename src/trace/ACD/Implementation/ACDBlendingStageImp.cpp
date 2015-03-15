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

#include "ACDBlendingStageImp.h"
#include "ACDMacros.h"
#include "support.h"
#include "ACDSupport.h"
#include <sstream>

#include "GlobalProfiler.h"

using namespace std;
using namespace acdlib;

ACDBlendingStageImp::ACDBlendingStageImp(ACDDeviceImp*, GPUDriver* driver) :
    _driver(driver), _enabled(MAX_RENDER_TARGETS, false), _syncRequired(true),
    _srcBlend(MAX_RENDER_TARGETS, ACD_BLEND_ONE),
    _destBlend(MAX_RENDER_TARGETS, ACD_BLEND_ZERO),
    _srcBlendAlpha(MAX_RENDER_TARGETS, ACD_BLEND_ONE),
    _destBlendAlpha(MAX_RENDER_TARGETS, ACD_BLEND_ZERO),
    _blendFunc(MAX_RENDER_TARGETS, ACD_BLEND_ADD),
    _blendFuncAlpha(MAX_RENDER_TARGETS, ACD_BLEND_ADD),
    _blendColor(MAX_RENDER_TARGETS, ACDFloatVector4(acd_float(0.0f))),
    _redMask(MAX_RENDER_TARGETS, true),
    _greenMask(MAX_RENDER_TARGETS, true),
    _blueMask(MAX_RENDER_TARGETS, true),
    _alphaMask(MAX_RENDER_TARGETS, true),
    _colorWriteEnabled(MAX_RENDER_TARGETS, false)
{
    //  The backbuffer (render target 0) is enabled by default.
    _colorWriteEnabled[0] = true;
    
    forceSync();
}

void ACDBlendingStageImp::setEnable(acd_uint renderTargetID, acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    _enabled[renderTargetID] = enable;
    GLOBALPROFILER_EXITREGION()
    
}

void ACDBlendingStageImp::setSrcBlend(acd_uint renderTargetID, ACD_BLEND_OPTION srcBlend)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    // Update object state
    _srcBlend[renderTargetID] = srcBlend;
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setDestBlend(acd_uint renderTargetID, ACD_BLEND_OPTION destBlend)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    // Update object state
    _destBlend[renderTargetID] = destBlend;
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setSrcBlendAlpha(acd_uint renderTargetID, ACD_BLEND_OPTION srcBlendAlpha)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    // update object state
    _srcBlendAlpha[renderTargetID] = srcBlendAlpha;
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setDestBlendAlpha(acd_uint renderTargetID, ACD_BLEND_OPTION destBlendAlpha)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    // update object state
    _destBlendAlpha[renderTargetID] = destBlendAlpha;
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setBlendFunc(acd_uint renderTargetID, ACD_BLEND_FUNCTION blendFunc)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    _blendFunc[renderTargetID] = blendFunc;
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setBlendFuncAlpha(acd_uint renderTargetID, ACD_BLEND_FUNCTION blendFuncAlpha)
{
    panic("ACDBlendingStageImp", "setBlendFuncAlpha", 
          "Separate ALPHA blending (function/equation) is not supported yet");

    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    _blendFuncAlpha[renderTargetID] = blendFuncAlpha;
}

void ACDBlendingStageImp::setBlendColor(acd_uint renderTargetID, acd_float red, acd_float green, acd_float blue, acd_float alpha)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")

    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    ACDFloatVector4 color;
    color[0] = acdsupport::clamp(red); 
    color[1] = acdsupport::clamp(green); 
    color[2] = acdsupport::clamp(blue);
    color[3] = acdsupport::clamp(alpha);

    _blendColor[renderTargetID] = color;
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setBlendColor(acd_uint renderTargetID, const acd_float* rgba)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    setBlendColor(renderTargetID, rgba[0], rgba[1], rgba[2], rgba[3]);
    GLOBALPROFILER_EXITREGION()
}

void ACDBlendingStageImp::setColorMask(acd_uint renderTargetID, acd_bool red, acd_bool green, acd_bool blue, acd_bool alpha)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")

    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    _redMask[renderTargetID] = red;
    _greenMask[renderTargetID] = green;
    _blueMask[renderTargetID] = blue;
    _alphaMask[renderTargetID] = alpha;
    GLOBALPROFILER_EXITREGION()
}

acd_bool ACDBlendingStageImp::getColorEnable(acd_uint renderTargetID)
{
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    return (_redMask[renderTargetID] || _greenMask[renderTargetID] || _blueMask[renderTargetID] || _alphaMask[renderTargetID]) &&
           _colorWriteEnabled[renderTargetID];
}

void ACDBlendingStageImp::enableColorWrite(acd_uint renderTargetID)
{
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    _colorWriteEnabled[renderTargetID] = true;
}

void ACDBlendingStageImp::disableColorWrite(acd_uint renderTargetID)
{
    ACD_ASSERT(
        if ( renderTargetID >= MAX_RENDER_TARGETS )
            panic("ACDBlendingStageImp", "setEnabled", "Render target value greater than the maximum allowed id");
    )

    _colorWriteEnabled[renderTargetID] = false;
}

void ACDBlendingStageImp::forceSync()
{
    _syncRequired = true;
    sync();
    _syncRequired = false;
}

void ACDBlendingStageImp::sync()
{
    gpu3d::GPURegData data;

    for ( acd_uint rt = 0; rt < MAX_RENDER_TARGETS; ++rt )
    {
        if ( _enabled[rt].changed() || _syncRequired )
        {
            data.booleanVal = _enabled[rt];
            _driver->writeGPURegister(gpu3d::GPU_COLOR_BLEND, rt, data);
            _enabled[rt].restart();
        }
        
        if ( _blendFunc[rt].changed() || _syncRequired ) {
            _getGPUBlendFunction(_blendFunc[rt], &data);
            _driver->writeGPURegister(gpu3d::GPU_BLEND_EQUATION, rt, data);
            _blendFunc[rt].restart();
        }

        if ( _blendFuncAlpha[rt].changed() || _syncRequired ) {
            _getGPUBlendFunction(_blendFuncAlpha[rt], &data);
            // Remove this comment and fix register name to support independent ALPHA BLEND equation/function 
            // _driver->writeGPURegister(gpu3d::GPU_BLEND_ALPHA_EQUATION, rt, data);
            _blendFuncAlpha[rt].restart();
        }

        if ( _srcBlend[rt].changed() || _syncRequired ) {
            _getGPUBlendOption(_srcBlend[rt], &data);
            _driver->writeGPURegister(gpu3d::GPU_BLEND_SRC_RGB, rt, data);
            _srcBlend[rt].restart();
        }

        if ( _destBlend[rt].changed() || _syncRequired ) {
            _getGPUBlendOption(_destBlend[rt], &data);
            _driver->writeGPURegister(gpu3d::GPU_BLEND_DST_RGB, rt, data);
            _destBlend[rt].restart();
        }

        if ( _srcBlendAlpha[rt].changed() || _syncRequired ) {
            _getGPUBlendOption(_srcBlendAlpha[rt], &data);
            _driver->writeGPURegister(gpu3d::GPU_BLEND_SRC_ALPHA, rt, data);
            _srcBlendAlpha[rt].restart();
        }

        if ( _destBlendAlpha[rt].changed() || _syncRequired ) {
            _getGPUBlendOption(_destBlendAlpha[rt], &data);
            _driver->writeGPURegister(gpu3d::GPU_BLEND_DST_ALPHA, rt, data);
            _destBlendAlpha[rt].restart();
        }
        
        if ( _blendColor[rt].changed() || _syncRequired ) {
            const ACDFloatVector4& blendColor = _blendColor[rt]; // Perform a explicit cast using a const reference
            data.qfVal[0] = blendColor[0];
            data.qfVal[1] = blendColor[1];
            data.qfVal[2] = blendColor[2];
            data.qfVal[3] = blendColor[3];
            _driver->writeGPURegister(gpu3d::GPU_BLEND_COLOR, rt, data);
            _blendColor[rt].restart();
        }

        if ( _redMask[rt].changed() || _colorWriteEnabled[rt].changed() || _syncRequired )
        {
            //data.booleanVal = (_redMask != 0 ? true : false);
            data.booleanVal = _redMask[rt] && _colorWriteEnabled[rt];
            _driver->writeGPURegister(gpu3d::GPU_COLOR_MASK_R, rt, data);  //blending
            _redMask[rt].restart();
        }

        if ( _greenMask[rt].changed() || _colorWriteEnabled[rt].changed() || _syncRequired )
        {
            //data.booleanVal = (_greenMask != 0 ? true : false);
            data.booleanVal = _greenMask[rt] && _colorWriteEnabled[rt];
            _driver->writeGPURegister(gpu3d::GPU_COLOR_MASK_G, rt, data);
            _greenMask[rt].restart();
        }

        if ( _blueMask[rt].changed() || _colorWriteEnabled[rt].changed() || _syncRequired )
        {
            //data.booleanVal = (_blueMask != 0 ? true : false);
            data.booleanVal = _blueMask[rt] && _colorWriteEnabled[rt];
            _driver->writeGPURegister(gpu3d::GPU_COLOR_MASK_B, rt, data);
            _blueMask[rt].restart();
        }

        if ( _alphaMask[rt].changed() || _colorWriteEnabled[rt].changed() || _syncRequired )
        {
            //data.booleanVal = (_alphaMask != 0 ? true : false);
            data.booleanVal = _alphaMask[rt] && _colorWriteEnabled[rt];
            _driver->writeGPURegister(gpu3d::GPU_COLOR_MASK_A, rt, data);
            _alphaMask[rt].restart();
        }
    }
}

void ACDBlendingStageImp::_getGPUBlendOption(ACD_BLEND_OPTION option, gpu3d::GPURegData* data)
{
    // Note:
    // BLEND_FUNCTION in Attila GPU is equivalent to BLEND_OPTION in Attila ACD
    // BLEND_EQUATION in Attila GPU is equivalent to BLEND_FUNCTION in Attila ACD

    using namespace gpu3d;
    switch ( option )
    {
        case ACD_BLEND_ZERO:
            data->blendFunction = BLEND_ZERO;
            break;
        case ACD_BLEND_ONE:
            data->blendFunction = BLEND_ONE;
            break;
        case ACD_BLEND_SRC_COLOR:
            data->blendFunction = BLEND_SRC_COLOR;
            break;
        case ACD_BLEND_INV_SRC_COLOR:
            data->blendFunction = BLEND_ONE_MINUS_SRC_COLOR;
            break;
        case ACD_BLEND_DEST_COLOR:
            data->blendFunction = BLEND_DST_COLOR;
            break;
        case ACD_BLEND_INV_DEST_COLOR:
            data->blendFunction = BLEND_ONE_MINUS_DST_COLOR;
            break;
        case ACD_BLEND_SRC_ALPHA:
            data->blendFunction = BLEND_SRC_ALPHA;
            break;
        case ACD_BLEND_INV_SRC_ALPHA:
            data->blendFunction = BLEND_ONE_MINUS_SRC_ALPHA;
            break;
        case ACD_BLEND_DEST_ALPHA:
            data->blendFunction = BLEND_DST_ALPHA;
            break;
        case ACD_BLEND_INV_DEST_ALPHA:
            data->blendFunction = BLEND_ONE_MINUS_DST_ALPHA;
            break;
        case ACD_BLEND_CONSTANT_COLOR:
            data->blendFunction = BLEND_CONSTANT_COLOR;
            break;
        case ACD_BLEND_INV_CONSTANT_COLOR:
            data->blendFunction = BLEND_ONE_MINUS_CONSTANT_COLOR;
            break;
        case ACD_BLEND_CONSTANT_ALPHA:
            data->blendFunction = BLEND_CONSTANT_ALPHA;
            break;
        case ACD_BLEND_INV_CONSTANT_ALPHA:
            data->blendFunction = BLEND_ONE_MINUS_CONSTANT_ALPHA;
            break;
        case ACD_BLEND_SRC_ALPHA_SAT:
            data->blendFunction = BLEND_SRC_ALPHA_SATURATE;
            break;
        case ACD_BLEND_BLEND_FACTOR:
            panic("ACDBlendingStageImp", "_setBlendOption", "ACD_BLEND_BLEND_FACTOR not supported");
            break;
        case ACD_BLEND_INV_BLEND_FACTOR:
            panic("ACDBlendingStageImp", "_setBlendOption", "ACD_BLEND_INV_BLEND_FACTOR not supported");
            break;
        default:
            panic("ACDBlendingStageImp", "_setBlendOption", "Unexpected blending function");
    }
}

void ACDBlendingStageImp::_getGPUBlendFunction(ACD_BLEND_FUNCTION func, gpu3d::GPURegData* eq)
{
    // Note:
    // BLEND_FUNCTION in Attila GPU is equivalent to BLEND_OPTION in Attila ACD
    // BLEND_EQUATION in Attila GPU is equivalent to BLEND_FUNCTION in Attila ACD

    using namespace gpu3d;
    
    switch ( func )
    {
        case ACD_BLEND_ADD:
            eq->blendEquation = BLEND_FUNC_ADD;
            break;
        case ACD_BLEND_SUBTRACT:
            eq->blendEquation = BLEND_FUNC_SUBTRACT;
            break;
        case ACD_BLEND_REVERSE_SUBTRACT:
            eq->blendEquation = BLEND_FUNC_REVERSE_SUBTRACT;
            break;
        case ACD_BLEND_MIN:
            eq->blendEquation = BLEND_MIN;
            break;
        case ACD_BLEND_MAX:
            eq->blendEquation = BLEND_MAX;
            break;
        default:
            panic("ACDBlendingStageImp", "setBlendOp", "Unexpected blend mode equation");
    }
}

const char* ACDBlendingStageImp::BlendOptionPrint::print(const ACD_BLEND_OPTION& option) const
{
    using namespace gpu3d;
    switch ( option )
    {
        case ACD_BLEND_ZERO:
            return "ZERO";
        case ACD_BLEND_ONE:
            return "ONE";
        case ACD_BLEND_SRC_COLOR:
            return "SRC_COLOR";
        case ACD_BLEND_INV_SRC_COLOR:
            return "INC_SRC_COLOR";
        case ACD_BLEND_DEST_COLOR:
            return "DEST_COLOR";
        case ACD_BLEND_INV_DEST_COLOR:
            return "INV_DEST_COLOR";
        case ACD_BLEND_SRC_ALPHA:
            return "SRC_ALPHA";
        case ACD_BLEND_INV_SRC_ALPHA:
            return "INV_SRC_ALPHA";
        case ACD_BLEND_DEST_ALPHA:
            return "DEST_ALPHA";
        case ACD_BLEND_INV_DEST_ALPHA:
            return "INV_DEST_ALPHA";
        case ACD_BLEND_CONSTANT_COLOR:
            return "CONSTANT_COLOR";
        case ACD_BLEND_INV_CONSTANT_COLOR:
            return "INV_CONSTANT_COLOR";
        case ACD_BLEND_CONSTANT_ALPHA:
            return "CONSTANT_ALPHA";
        case ACD_BLEND_INV_CONSTANT_ALPHA:
            return "INV_CONSTANT_ALPHA";
        case ACD_BLEND_SRC_ALPHA_SAT:
            return "SRC_ALPHA_SAT";
        case ACD_BLEND_BLEND_FACTOR:
            return "BLEND_FACTOR";
        case ACD_BLEND_INV_BLEND_FACTOR:
            return "INV_BLEND_FACTOR";
        default:
            panic("ACDBlendingStageImp::BlendOptionPrint", "print", "Unexpected blending function");
    }
    return 0;
}

const char* ACDBlendingStageImp::BlendFunctionPrint::print(const ACD_BLEND_FUNCTION& func) const
{
    using namespace gpu3d;
    
    switch ( func )
    {
        case ACD_BLEND_ADD:
            return "ADD";
        case ACD_BLEND_SUBTRACT:
            return "SUBTRACT";
        case ACD_BLEND_REVERSE_SUBTRACT:
            return "REVERSE_SUBTRACT";
        case ACD_BLEND_MIN:
            return "MIN";
        case ACD_BLEND_MAX:
            return "MAX";
        default:
            panic("ACDBlendingStageImp::BlendFunctionPrint", "print", "Unexpected blend mode equation");
    }
    return 0;
}

string ACDBlendingStageImp::getInternalState() const
{
    stringstream ss, ssAux;

    ssAux.str("");

    for ( acd_uint i = 0; i < MAX_RENDER_TARGETS; ++i )
    {
        ssAux << "RENDER_TARGET" << i << "_ENABLED"; ss << stateItemString<acd_bool>(_enabled[i], ssAux.str().c_str(), false, &boolPrint); ssAux.str("");
        ss << stateItemString<ACD_BLEND_FUNCTION>(_blendFunc[i], "BLEND_FUNCTION_RGB", false, &blendFunctionPrint); 
        ss << stateItemString<ACD_BLEND_OPTION>(_srcBlend[i], "BLEND_SRC_RGB", false, &blendOptionPrint);
        ss << stateItemString<ACD_BLEND_OPTION>(_destBlend[i], "BLEND_DST_RGB", false, &blendOptionPrint);

        ss << stateItemString<ACD_BLEND_FUNCTION>(_blendFuncAlpha[i], "BLEND_FUNCTION_ALPHA", false, &blendFunctionPrint);
        ss << stateItemString<ACD_BLEND_OPTION>(_srcBlendAlpha[i], "BLEND_SRC_ALPHA", false, &blendOptionPrint);
        ss << stateItemString<ACD_BLEND_OPTION>(_destBlendAlpha[i], "BLEND_DST_ALPHA", false, &blendOptionPrint);

        const ACDFloatVector4& color = _blendColor[i];
        ss << stateItemString<ACDFloatVector4>(color, "BLEND_COLOR", false);
    }


    return ss.str();

}

const StoredStateItem* ACDBlendingStageImp::createStoredStateItem(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACDStoredStateItem* ret;
    acd_uint rTarget;

    if (stateId >= ACD_BLENDING_FIRST_ID && stateId < ACD_BLENDING_LAST)
    {    
        // It큦 a blending state
        
        if ((stateId >= ACD_BLENDING_ENABLED) && (stateId < ACD_BLENDING_ENABLED + ACD_MAX_RENDER_TARGETS))
        {    
            // It큦 a blending enabled state
            rTarget = stateId - ACD_BLENDING_ENABLED;
            ret = new ACDSingleBoolStoredStateItem(_enabled[rTarget]);
        }
        else 
        {
            switch(stateId)
            {
                case ACD_BLENDING_SRC_RGB:              ret = new ACDSingleEnumStoredStateItem((acd_enum)_srcBlend[0]);            break;
                case ACD_BLENDING_DST_RGB:              ret = new ACDSingleEnumStoredStateItem((acd_enum)_destBlend[0]);           break;
                case ACD_BLENDING_FUNC_RGB:             ret = new ACDSingleEnumStoredStateItem((acd_enum)_blendFunc[0]);           break;
                case ACD_BLENDING_SRC_ALPHA:            ret = new ACDSingleEnumStoredStateItem((acd_enum)_srcBlendAlpha[0]);       break;
                case ACD_BLENDING_DST_ALPHA:            ret = new ACDSingleEnumStoredStateItem((acd_enum)_destBlendAlpha[0]);      break;
                case ACD_BLENDING_FUNC_ALPHA:           ret = new ACDSingleEnumStoredStateItem((acd_enum)_blendFuncAlpha[0]);      break;
                case ACD_BLENDING_COLOR:                ret = new ACDFloatVector4StoredStateItem(_blendColor[0]);                  break;
                case ACD_BLENDING_COLOR_WRITE_ENABLED:  ret = new ACDSingleBoolStoredStateItem((acd_enum)_colorWriteEnabled[0]);   break;
                case ACD_BLENDING_COLOR_MASK_R:         ret = new ACDSingleBoolStoredStateItem((acd_enum)_redMask[0]);             break;
                case ACD_BLENDING_COLOR_MASK_G:         ret = new ACDSingleBoolStoredStateItem((acd_enum)_greenMask[0]);           break;
                case ACD_BLENDING_COLOR_MASK_B:         ret = new ACDSingleBoolStoredStateItem((acd_enum)_blueMask[0]);            break;
                case ACD_BLENDING_COLOR_MASK_A:         ret = new ACDSingleBoolStoredStateItem((acd_enum)_alphaMask[0]);           break;
                // case ACDX_BLENDING_... <- add here future additional blending states.

                default:
                    panic("ACDBlendingStageImp","createStoredStateItem","Unexpected blending state id");
            }
        }            
        // else if (... <- write here for future additional defined blending states.
    }
    else
        panic("ACDBlendingStageImp","createStoredStateItem","Unexpected blending state id");

    ret->setItemId(stateId);

    GLOBALPROFILER_EXITREGION()

    return ret;
}

#define CAST_TO_BOOL(X)             *(static_cast<const ACDSingleBoolStoredStateItem*>(X))
#define BLEND_FUNCTION(DST,X)       { const ACDSingleEnumStoredStateItem* aux = static_cast<const ACDSingleEnumStoredStateItem*>(X); acd_enum value = *aux; DST = static_cast<ACD_BLEND_FUNCTION>(value); }
#define BLEND_OPTION(DST,X)         { const ACDSingleEnumStoredStateItem* aux = static_cast<const ACDSingleEnumStoredStateItem*>(X); acd_enum value = *aux; DST = static_cast<ACD_BLEND_OPTION>(value); }
#define CAST_TO_VECT4(X)            *(static_cast<const ACDFloatVector4StoredStateItem*>(X))

void ACDBlendingStageImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    acd_uint rTarget;

    const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(ssi);

    ACD_STORED_ITEM_ID stateId = acdssi->getItemId();

    if (stateId >= ACD_BLENDING_FIRST_ID && stateId < ACD_BLENDING_LAST)
    {    
        // It큦 a blending state
        
        if ((stateId >= ACD_BLENDING_ENABLED) && (stateId < ACD_BLENDING_ENABLED + ACD_MAX_RENDER_TARGETS))
        {    
            // It큦 a blending enabled state
            rTarget = stateId - ACD_BLENDING_ENABLED;
            _enabled[rTarget] = CAST_TO_BOOL(acdssi);
            delete(acdssi);
        }
        else 
        {
            switch(stateId)
            {
                case ACD_BLENDING_SRC_RGB:              BLEND_OPTION(_srcBlend[0], acdssi);            delete(acdssi);     break;
                case ACD_BLENDING_DST_RGB:              BLEND_OPTION(_destBlend[0], acdssi);           delete(acdssi);     break;
                case ACD_BLENDING_FUNC_RGB:             BLEND_FUNCTION(_blendFunc[0], acdssi);         delete(acdssi);     break;
                case ACD_BLENDING_SRC_ALPHA:            BLEND_OPTION(_srcBlendAlpha[0], acdssi);       delete(acdssi);     break;
                case ACD_BLENDING_DST_ALPHA:            BLEND_OPTION(_destBlendAlpha[0], acdssi);      delete(acdssi);     break;
                case ACD_BLENDING_FUNC_ALPHA:           BLEND_FUNCTION(_blendFuncAlpha[0], acdssi);    delete(acdssi);     break;
                case ACD_BLENDING_COLOR_WRITE_ENABLED:  _colorWriteEnabled[0] = CAST_TO_BOOL(acdssi);  delete(acdssi);     break;
                case ACD_BLENDING_COLOR:                _blendColor[0] = CAST_TO_VECT4(acdssi);        delete(acdssi);     break;
                case ACD_BLENDING_COLOR_MASK_R:         _redMask[0] = CAST_TO_BOOL(acdssi);            delete(acdssi);     break;
                case ACD_BLENDING_COLOR_MASK_G:         _greenMask[0] = CAST_TO_BOOL(acdssi);          delete(acdssi);     break;
                case ACD_BLENDING_COLOR_MASK_B:         _blueMask[0] = CAST_TO_BOOL(acdssi);           delete(acdssi);     break;
                case ACD_BLENDING_COLOR_MASK_A:         _alphaMask[0] = CAST_TO_BOOL(acdssi);          delete(acdssi);     break;

                // case ACDX_BLENDING_... <- add here future additional blending states.
                default:
                    panic("ACDBlendingStageImp","createStoredStateItem","Unexpected blending state id");
            }
        }            
        // else if (... <- write here for future additional defined blending states.
    }
    else
        panic("ACDBlendingStageImp","restoreStoredStateItem","Unexpected blending state id");

    GLOBALPROFILER_EXITREGION()
}

#undef CAST_TO_BOOL             
#undef BLEND_FUNCTION
#undef BLEND_OPTION
#undef CAST_TO_VECT4

ACDStoredState* ACDBlendingStageImp::saveAllState() const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    
    ACDStoredStateImp* ret = new ACDStoredStateImp();

    for (acd_uint i = 0; i < ACD_BLENDING_LAST; i++)
         ret->addStoredStateItem(createStoredStateItem(ACD_STORED_ITEM_ID(i)));

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

void ACDBlendingStageImp::restoreAllState(const ACDStoredState* state)
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

    delete(state);

    GLOBALPROFILER_EXITREGION()
}
