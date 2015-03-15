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

#include "ACDZStencilStageImp.h"
#include "ACDMacros.h"
#include <sstream>

#include "ACDStoredStateImp.h"

#include "GlobalProfiler.h"

using namespace acdlib;
using namespace std;

ACDZStencilStageImp::ACDZStencilStageImp(ACDDeviceImp* device, GPUDriver* driver) :
    _device(device), _driver(driver), _syncRequired(true),
    _zStencilBufferDefined(true),
    _zEnabled(false),
    _stencilEnabled(false),
    _zFunc(ACD_COMPARE_FUNCTION_LESS),
    _zMask(true),
    _depthRangeNear(0.0),
    _depthRangeFar(1.0),
    _d3d9DepthRange(false),
    _front(ACD_STENCIL_OP_KEEP, 
           ACD_STENCIL_OP_KEEP, 
           ACD_STENCIL_OP_KEEP, 
           ACD_COMPARE_FUNCTION_ALWAYS,
           0,
           0),
    _back( ACD_STENCIL_OP_KEEP,
           ACD_STENCIL_OP_KEEP,
           ACD_STENCIL_OP_KEEP,
           ACD_COMPARE_FUNCTION_ALWAYS,
           0,
           0),
    _depthSlopeFactor(0),
    _depthUnitOffset(0),
    _stencilUpdateMask(0)
{
    forceSync();
}


void ACDZStencilStageImp::setZEnabled(acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _zEnabled = enable;
    GLOBALPROFILER_EXITREGION()
}

acd_bool ACDZStencilStageImp::isZEnabled() const
{
    return _zEnabled;
}

void ACDZStencilStageImp::setZFunc(ACD_COMPARE_FUNCTION zFunc)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _zFunc = zFunc;
    GLOBALPROFILER_EXITREGION()
}

ACD_COMPARE_FUNCTION ACDZStencilStageImp::getZFunc() const
{
    return _zFunc;
}

void ACDZStencilStageImp::setZMask(acd_bool mask)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _zMask = mask;
    GLOBALPROFILER_EXITREGION()
}

acd_bool ACDZStencilStageImp::getZMask() const
{
    return _zMask;
}

void ACDZStencilStageImp::setStencilEnabled(acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _stencilEnabled = enable;
    GLOBALPROFILER_EXITREGION()
}

acd_bool ACDZStencilStageImp::isStencilEnabled() const
{
    return _stencilEnabled;
}

void ACDZStencilStageImp::setStencilOp( ACD_FACE face, 
                                        ACD_STENCIL_OP onStencilFail,
                                        ACD_STENCIL_OP onStencilPassZFails,
                                        ACD_STENCIL_OP onStencilPassZPass)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT
    (
        if ( face != ACD_FACE_FRONT && face != ACD_FACE_BACK && face != ACD_FACE_FRONT_AND_BACK )
            panic("ACDZStencilStageImp", "setStencilOp", "Only FRONT, BACK or FRONT_AND_BACK faces accepted");
    )

    if ( face == ACD_FACE_FRONT || face == ACD_FACE_FRONT_AND_BACK ) {
        _front.onStencilFail = onStencilFail;
        _front.onStencilPassZFails = onStencilPassZFails;
        _front.onStencilPassZPass = onStencilPassZPass;
    }
    if ( face == ACD_FACE_BACK || face == ACD_FACE_FRONT_AND_BACK ) {
        _back.onStencilFail = onStencilFail;
        _back.onStencilPassZFails = onStencilPassZFails;
        _back.onStencilPassZPass = onStencilPassZPass;
    }
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::getStencilOp( ACD_FACE face,
                                        ACD_STENCIL_OP& onStencilFail,
                                        ACD_STENCIL_OP& onStencilPassZFail,
                                        ACD_STENCIL_OP& onStencilPassZPass) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT
    (
        if ( face != ACD_FACE_FRONT && face != ACD_FACE_BACK )
            panic("ACDZStencilStageImp", "getStencilOp", "Only FRONT or BACK faces accepted");
    )
    
    const _FaceInfo& faceInfo = ( face == ACD_FACE_FRONT ? _front : _back );

    onStencilFail = faceInfo.onStencilFail;
    onStencilPassZFail = faceInfo.onStencilPassZFails;
    onStencilPassZPass = faceInfo.onStencilPassZPass;
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::setStencilFunc( ACD_FACE face, ACD_COMPARE_FUNCTION func, acd_uint stencilRef, acd_uint stencilMask ) 
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT
    (
        if ( face != ACD_FACE_FRONT && face != ACD_FACE_BACK && face != ACD_FACE_FRONT_AND_BACK ) 
            panic("ACDZStencilStageImp", "setStencilFunc", "Only FRONT, BACK or FRONT_AND_BACK faces accepted");
    )

    if ( face == ACD_FACE_FRONT || face == ACD_FACE_FRONT_AND_BACK ) {
        _front.stencilFunc = func;
        _front.stencilRef = stencilRef;
        _front.stencilMask = stencilMask;
    }
    else if ( face == ACD_FACE_BACK || face == ACD_FACE_FRONT_AND_BACK ) {
        _back.stencilFunc = func;
        _back.stencilRef = stencilRef;
        _back.stencilMask = stencilMask;
    }
    else
        panic("ACDZStencilStageImp", "setStencilFunc", "Stencil Function not defined neither front nor back");
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::setDepthRange (acd_float near, acd_float far)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _depthRangeNear = near;
    _depthRangeFar = far;
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::setD3D9DepthRangeMode(acd_bool mode)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _d3d9DepthRange = mode;
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::setPolygonOffset (acd_float factor, acd_float units)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _depthSlopeFactor = factor;
    _depthUnitOffset = units;
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::setStencilUpdateMask (acd_uint mask)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _stencilUpdateMask = mask;
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::setZStencilBufferDefined(acd_bool present)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _zStencilBufferDefined = present;
    GLOBALPROFILER_EXITREGION()
}

void ACDZStencilStageImp::sync()
{
    gpu3d::GPURegData data;

    if ( _zEnabled.changed() || _zStencilBufferDefined.changed() || _syncRequired ) {
        data.booleanVal = _zEnabled && _zStencilBufferDefined;
        _driver->writeGPURegister(gpu3d::GPU_DEPTH_TEST, data);
        _zEnabled.restart();
        _zStencilBufferDefined.restart();
    }

    if ( (_zEnabled && _zFunc.changed()) || _syncRequired ) { // only updates if _zEnabled == true
        _getGPUCompareFunction(_zFunc, &data);
        _driver->writeGPURegister(gpu3d::GPU_DEPTH_FUNCTION, data);
        _zFunc.restart();
    }

    if ( (_zEnabled && _zMask.changed()) || _syncRequired ) {
        data.booleanVal = _zMask;
        _driver->writeGPURegister(gpu3d::GPU_DEPTH_MASK, data);
        _zMask.restart();
    }

    if ( _stencilEnabled.changed() || _zStencilBufferDefined.changed() || _syncRequired ) {
        data.booleanVal = _stencilEnabled && _zStencilBufferDefined;
        _driver->writeGPURegister(gpu3d::GPU_STENCIL_TEST, data);
        _stencilEnabled.restart();    
        _zStencilBufferDefined.restart();
    }
    
    if ( _stencilEnabled || _syncRequired ) {
        // Updates FRONT_FACE related state
        if ( _front.stencilFunc.changed() || _syncRequired ) {
            _getGPUCompareFunction(_front.stencilFunc, &data);
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_FUNCTION, data);
            _front.stencilFunc.restart();
        }
        if ( _front.stencilRef.changed() || _syncRequired ) {
            data.uintVal = _front.stencilRef;
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_REFERENCE, data);
            _front.stencilRef.restart();
        }
        if ( _front.stencilMask.changed() || _syncRequired ) {
            data.uintVal = _front.stencilMask;
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_COMPARE_MASK, data);
            _front.stencilMask.restart();
        }
        if ( _front.onStencilFail.changed() || _syncRequired ) {
            _getGPUStencilOperation(_front.onStencilFail, &data);
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_FAIL_UPDATE, data);
            _front.onStencilFail.restart();
        }
        if ( _front.onStencilPassZFails.changed() || _syncRequired ) {
            _getGPUStencilOperation(_front.onStencilPassZFails, &data);
            _driver->writeGPURegister(gpu3d::GPU_DEPTH_FAIL_UPDATE, data);
            _front.onStencilPassZFails.restart();
        }
        if ( _front.onStencilPassZPass.changed() || _syncRequired ) {
            _getGPUStencilOperation(_front.onStencilPassZPass, &data);
            _driver->writeGPURegister(gpu3d::GPU_DEPTH_PASS_UPDATE, data);
            _front.onStencilPassZPass.restart();
        }
        // Updates BACK_FACE related state
        if ( _back.stencilFunc.changed() || _syncRequired ) {
            _getGPUCompareFunction(_back.stencilFunc, &data);
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_FUNCTION, data);
            _back.stencilFunc.restart();
        }
        if ( _back.stencilRef.changed() || _syncRequired ) {
            data.uintVal = _back.stencilRef;
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_REFERENCE, data);
            _back.stencilRef.restart();
        }
        if ( _back.stencilMask.changed() || _syncRequired ) {
            data.uintVal = _back.stencilMask;
            _driver->writeGPURegister(gpu3d::GPU_STENCIL_COMPARE_MASK, data);
            _back.stencilMask.restart();
        }
        if ( _back.onStencilFail.changed() || _syncRequired ) {
            _getGPUStencilOperation(_back.onStencilFail, &data);
            // Separate per faces stencil not supported by Attila
            // Remove the comment and fix the register name when the support is available
            //_driver->writeGPURegister(gpu3d::GPU_STENCIL_FAIL_UPDATE_BACK, data);
            _back.onStencilFail.restart();
        }
        if ( _back.onStencilPassZFails.changed() || _syncRequired ) {
            _getGPUStencilOperation(_back.onStencilPassZFails, &data);
            // Separate per faces stencil not supported by Attila
            // Remove the comment and fix the register name when the support is available
            //_driver->writeGPURegister(gpu3d::GPU_DEPTH_FAIL_UPDATE_BACK, data);
            _back.onStencilPassZFails.restart();
        }
        if ( _back.onStencilPassZPass.changed() || _syncRequired ) {
            _getGPUStencilOperation(_back.onStencilPassZPass, &data);
            // Separate per faces stencil not supported by Attila
            // Remove the comment and fix the register name when the support is available
            //_driver->writeGPURegister(gpu3d::GPU_DEPTH_PASS_UPDATE_BACK, data);
            _back.onStencilPassZPass.restart();
        }
    }

    if (_depthRangeNear.changed() || _syncRequired) {
        data.f32Val = _depthRangeNear;
        _driver->writeGPURegister( gpu3d::GPU_DEPTH_RANGE_NEAR, data );
        _depthRangeNear.restart();
    }

    if (_depthRangeFar.changed() || _syncRequired) {
        data.f32Val = _depthRangeFar;
        _driver->writeGPURegister( gpu3d::GPU_DEPTH_RANGE_FAR, data );
        _depthRangeFar.restart();
    }
    
    if (_d3d9DepthRange.changed() || _syncRequired)
    {
        data.booleanVal = _d3d9DepthRange;
        _driver->writeGPURegister(gpu3d::GPU_D3D9_DEPTH_RANGE, data);
        _d3d9DepthRange.restart();
    }

    if (_depthSlopeFactor.changed() || _syncRequired) {
        data.f32Val = _depthSlopeFactor;
        _driver->writeGPURegister( gpu3d::GPU_DEPTH_SLOPE_FACTOR, data );
        _depthSlopeFactor.restart();
    }

    if (_depthUnitOffset.changed() || _syncRequired) {
        data.f32Val = _depthUnitOffset;
        data.f32Val = 0;
        _driver->writeGPURegister( gpu3d::GPU_DEPTH_UNIT_OFFSET, data );
        _depthUnitOffset.restart();
    }

    if (_stencilUpdateMask.changed() || _syncRequired) {
        data.uintVal = _stencilUpdateMask;
        _driver->writeGPURegister( gpu3d::GPU_STENCIL_UPDATE_MASK, data );
        _stencilUpdateMask.restart();
    }

}

void ACDZStencilStageImp::forceSync()
{
    _syncRequired = true;
    sync();
    _syncRequired = false;
}

string ACDZStencilStageImp::getInternalState() const
{
    stringstream ss;
    
    // Depth state
    ss << stateItemString<acd_bool>(_zEnabled, "Z_ENABLED", false, &boolPrint);
    ss << stateItemString<ACD_COMPARE_FUNCTION>(_zFunc, "Z_FUNCTION", false, &compareFunctionPrint);
    ss << stateItemString<acd_bool>(_zMask, "Z_MASK", false, &boolPrint);

    // Stencil state
    ss << stateItemString<acd_bool>(_stencilEnabled, "STENCIL_ENABLED", false, &boolPrint);
    // Front face stencil state
    ss << stateItemString<ACD_STENCIL_OP>(_front.onStencilFail, "STENCIL_FRONT_ON_STENCIL_FAIL", false, &stencilOperationPrint);
    ss << stateItemString<ACD_STENCIL_OP>(_front.onStencilPassZFails, "STENCIL_FRONT_ON_STENCIL_PASS_Z_FAILS", false, &stencilOperationPrint);
    ss << stateItemString<ACD_STENCIL_OP>(_front.onStencilPassZPass, "STENCIL_FRONT_ON_STENCIL_PASS_Z_PASS", false, &stencilOperationPrint);
    ss << stateItemString<ACD_COMPARE_FUNCTION>(_front.stencilFunc, "STENCIL_FRONT_STENCIL_FUNC", false, &compareFunctionPrint);
    ss << stateItemString(_front.stencilRef, "STENCIL_FRONT_STENCIL_REF", false);
    // Back face stencil state
    ss << stateItemString<ACD_STENCIL_OP>(_back.onStencilFail, "STENCIL_BACK_ON_STENCIL_FAIL", false, &stencilOperationPrint);
    ss << stateItemString<ACD_STENCIL_OP>(_back.onStencilPassZFails, "STENCIL_BACK_ON_STENCIL_PASS_Z_FAILS", false, &stencilOperationPrint);
    ss << stateItemString<ACD_STENCIL_OP>(_back.onStencilPassZPass, "STENCIL_BACK_ON_STENCIL_PASS_Z_PASS", false, &stencilOperationPrint);
    ss << stateItemString<ACD_COMPARE_FUNCTION>(_back.stencilFunc, "STENCIL_BACK_STENCIL_FUNC", false, &compareFunctionPrint);
    ss << stateItemString(_back.stencilRef, "STENCIL_BACK_STENCIL_REF", false);

    return ss.str();
}

void ACDZStencilStageImp::_getGPUCompareFunction(ACD_COMPARE_FUNCTION comp, gpu3d::GPURegData* data)
{
    using namespace gpu3d;
    switch ( comp ) {
        case ACD_COMPARE_FUNCTION_NEVER:
            data->compare = GPU_NEVER; break;
        case ACD_COMPARE_FUNCTION_LESS:
            data->compare = GPU_LESS; break;
        case ACD_COMPARE_FUNCTION_EQUAL:
            data->compare = GPU_EQUAL; break;
        case ACD_COMPARE_FUNCTION_LESS_EQUAL:
            data->compare = GPU_LEQUAL; break;
        case ACD_COMPARE_FUNCTION_GREATER:
            data->compare = GPU_GREATER; break;
        case ACD_COMPARE_FUNCTION_NOT_EQUAL:
            data->compare = GPU_NOTEQUAL; break;
        case ACD_COMPARE_FUNCTION_GREATER_EQUAL:
            data->compare = GPU_GEQUAL; break;
        case ACD_COMPARE_FUNCTION_ALWAYS:
            data->compare = GPU_ALWAYS; break;
        default:
            panic("ACDZStencilStageImp", "_getGPUCompareFunction", "Unknown compare function"); 
    }
}

void ACDZStencilStageImp::_getGPUStencilOperation(ACD_STENCIL_OP op, gpu3d::GPURegData* data)
{
    switch ( op ) {
        case ACD_STENCIL_OP_KEEP:
            data->stencilUpdate = gpu3d::STENCIL_KEEP; break;
        case ACD_STENCIL_OP_ZERO:
            data->stencilUpdate = gpu3d::STENCIL_ZERO; break;
        case ACD_STENCIL_OP_REPLACE:
            data->stencilUpdate = gpu3d::STENCIL_REPLACE; break;
        case ACD_STENCIL_OP_INCR_SAT:
            data->stencilUpdate = gpu3d::STENCIL_INCR; break;            
        case ACD_STENCIL_OP_DECR_SAT:
            data->stencilUpdate = gpu3d::STENCIL_DECR; break;
        case ACD_STENCIL_OP_INVERT:
            data->stencilUpdate = gpu3d::STENCIL_INVERT; break;
        case ACD_STENCIL_OP_INCR:
            data->stencilUpdate = gpu3d::STENCIL_INCR_WRAP; break;
        case ACD_STENCIL_OP_DECR:
            data->stencilUpdate = gpu3d::STENCIL_DECR_WRAP; break;
        default:
            panic("ACDZStencilStageImp", "_getGPUStencilOperation", "Unknown stencil operation");
    }
}

const char* ACDZStencilStageImp::CompareFunctionPrint::print(const ACD_COMPARE_FUNCTION& comp) const
{
    using namespace gpu3d;
    switch ( comp ) {
        case ACD_COMPARE_FUNCTION_NEVER:
            return "NEVER";
        case ACD_COMPARE_FUNCTION_LESS:
            return "LESS";
        case ACD_COMPARE_FUNCTION_EQUAL:
            return "EQUAL";
        case ACD_COMPARE_FUNCTION_LESS_EQUAL:
            return "LESS_EQUAL";
        case ACD_COMPARE_FUNCTION_GREATER:
            return "GREATER";
        case ACD_COMPARE_FUNCTION_NOT_EQUAL:
            return "NOT_EQUAL";
        case ACD_COMPARE_FUNCTION_GREATER_EQUAL:
            return "GREATER_EQUAL";
        case ACD_COMPARE_FUNCTION_ALWAYS:
            return "ALWAYS";
        default:
            panic("ACDZStencilStageImp::CompareFunctionPrint", "print", "Unknown compare function"); 
    }
    return 0;
}

const char* ACDZStencilStageImp::StencilOperationPrint::print(const ACD_STENCIL_OP& op) const
{
    switch ( op ) {
        case ACD_STENCIL_OP_KEEP:
            return "KEEP";
        case ACD_STENCIL_OP_ZERO:
            return "ZERO";
        case ACD_STENCIL_OP_REPLACE:
            return "REPLACE";
        case ACD_STENCIL_OP_INCR_SAT:
            return "INCR_SAT";
        case ACD_STENCIL_OP_DECR_SAT:
            return "DECR_SAT";
        case ACD_STENCIL_OP_INVERT:
            return "INVERT";
        case ACD_STENCIL_OP_INCR:
            return "INCR";
        case ACD_STENCIL_OP_DECR:
            return "DECR";
        default:
            panic("ACDZStencilStageImp::StencilOperationPrint", "print", "Unknown stencil operation");
    }
    return 0;
}

const StoredStateItem* ACDZStencilStageImp::createStoredStateItem(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACDStoredStateItem* ret;

    switch(stateId)
    {
        case ACD_ZST_Z_ENABLED:                     ret = new ACDSingleBoolStoredStateItem(_zEnabled);                      break;
        case ACD_ZST_Z_FUNC:                        ret = new ACDSingleEnumStoredStateItem((acd_enum)_zFunc);               break;
        case ACD_ZST_Z_MASK:                            ret = new ACDSingleBoolStoredStateItem(_zMask);                     break;
        case ACD_ZST_STENCIL_ENABLED:               ret = new ACDSingleBoolStoredStateItem(_stencilEnabled);                break;
        case ACD_ZST_STENCIL_BUFFER_DEF:            ret = new ACDSingleUintStoredStateItem(_zStencilBufferDefined);         break;
        case ACD_ZST_FRONT_COMPARE_FUNC:            ret = new ACDSingleEnumStoredStateItem((acd_enum)_front.stencilFunc);   break;
        case ACD_ZST_FRONT_STENCIL_REF_VALUE:       ret = new ACDSingleUintStoredStateItem(_front.stencilRef);              break;
        case ACD_ZST_FRONT_STENCIL_COMPARE_MASK:    ret = new ACDSingleUintStoredStateItem(_front.stencilMask);             break;
        case ACD_ZST_FRONT_STENCIL_OPS: 
            { 
                ACDEnumVector3 v; 
                v[0] = (acd_enum)_front.onStencilFail; 
                v[1] = (acd_enum)_front.onStencilPassZFails; 
                v[2] = (acd_enum)_front.onStencilPassZPass; 
                ret = new ACDEnumVector3StoredStateItem(v); 
                break; 
            }
        case ACD_ZST_BACK_COMPARE_FUNC:             ret = new ACDSingleEnumStoredStateItem((acd_enum)_back.stencilFunc);    break;
        case ACD_ZST_BACK_STENCIL_REF_VALUE:        ret = new ACDSingleUintStoredStateItem((acd_enum)_back.stencilRef);     break;
        case ACD_ZST_BACK_STENCIL_COMPARE_MASK:     ret = new ACDSingleUintStoredStateItem((acd_enum)_back.stencilMask);    break;
        case ACD_ZST_BACK_STENCIL_OPS: 
            { 
                ACDEnumVector3 v; 
                v[0] = (acd_enum)_back.onStencilFail; 
                v[1] = (acd_enum)_back.onStencilPassZFails; 
                v[2] = (acd_enum)_back.onStencilPassZPass; 
                ret = new ACDEnumVector3StoredStateItem(v); 
                break; 
            }

        case ACD_ZST_RANGE_NEAR:            ret = new ACDSingleFloatStoredStateItem((acd_enum)_depthRangeNear);     break;
        case ACD_ZST_RANGE_FAR:             ret = new ACDSingleFloatStoredStateItem((acd_enum)_depthRangeFar);      break;
        case ACD_ZST_D3D9_DEPTH_RANGE:      ret = new ACDSingleBoolStoredStateItem((acd_enum) _d3d9DepthRange);     break;
        case ACD_ZST_SLOPE_FACTOR:          ret = new ACDSingleFloatStoredStateItem((acd_enum)_depthSlopeFactor);   break;
        case ACD_ZST_UNIT_OFFSET:           ret = new ACDSingleFloatStoredStateItem((acd_enum)_depthUnitOffset);    break;
        case ACD_ZST_DEPTH_UPDATE_MASK:     ret = new ACDSingleUintStoredStateItem((acd_enum)_stencilUpdateMask);   break;


        // case ACD_ZST_... <- add here future additional z stencil states.
        default:
            panic("ACDZStencilStageImp","createStoredStateItem()","Unknown z stencil state");
            ret = 0;
    }

    ret->setItemId(stateId);

    GLOBALPROFILER_EXITREGION()

    return ret;
}

#define CAST_TO_BOOL(X)         *(static_cast<const ACDSingleBoolStoredStateItem*>(X))
#define COMP_FUNC(DST,X)        { const ACDSingleEnumStoredStateItem* aux = static_cast<const ACDSingleEnumStoredStateItem*>(X); acd_enum value = *aux; DST = static_cast<ACD_COMPARE_FUNCTION>(value); }
#define STENCIL_OP(DST,X,i)     { const ACDEnumVector3StoredStateItem* aux = static_cast<const ACDEnumVector3StoredStateItem*>(X); ACDEnumVector3 v = (*aux); acd_enum value = v[i]; DST = static_cast<ACD_STENCIL_OP>(value); }
#define CAST_TO_UINT(X)         *(static_cast<const ACDSingleUintStoredStateItem*>(X))
#define CAST_TO_FLOAT(X)        *(static_cast<const ACDSingleFloatStoredStateItem*>(X))

void ACDZStencilStageImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(ssi);

    ACD_STORED_ITEM_ID stateId = acdssi->getItemId();

    switch(stateId)
    {
        case ACD_ZST_Z_ENABLED:                     _zEnabled = CAST_TO_BOOL(acdssi);               delete(acdssi);     break;
        case ACD_ZST_Z_FUNC:                        COMP_FUNC(_zFunc, acdssi);                      delete(acdssi);     break;
        case ACD_ZST_Z_MASK:                        _zMask = CAST_TO_BOOL(acdssi);                  delete(acdssi);     break;
        case ACD_ZST_STENCIL_ENABLED:               _stencilEnabled = CAST_TO_BOOL(acdssi);         delete(acdssi);     break;
        case ACD_ZST_STENCIL_BUFFER_DEF:            _zStencilBufferDefined = CAST_TO_UINT(acdssi);  delete(acdssi);     break;
        case ACD_ZST_FRONT_STENCIL_REF_VALUE:       _front.stencilRef = CAST_TO_UINT(acdssi);       delete(acdssi);     break;
        case ACD_ZST_FRONT_COMPARE_FUNC:            COMP_FUNC(_front.stencilFunc, acdssi);          delete(acdssi);     break;
        case ACD_ZST_FRONT_STENCIL_COMPARE_MASK:    _front.stencilMask = CAST_TO_UINT(acdssi);      delete(acdssi);     break;
        case ACD_ZST_FRONT_STENCIL_OPS:         
            STENCIL_OP(_front.onStencilFail, acdssi, 0); 
            STENCIL_OP(_front.onStencilPassZFails, acdssi, 1); 
            STENCIL_OP(_front.onStencilPassZPass, acdssi, 2); 
            delete(acdssi);
            break;
        case ACD_ZST_BACK_COMPARE_FUNC:             COMP_FUNC(_back.stencilFunc, acdssi);           delete(acdssi);     break;
        case ACD_ZST_BACK_STENCIL_REF_VALUE:        _back.stencilRef = CAST_TO_UINT(acdssi);        delete(acdssi);     break;
        case ACD_ZST_BACK_STENCIL_COMPARE_MASK:     _back.stencilMask = CAST_TO_UINT(acdssi);       delete(acdssi);     break;
        case ACD_ZST_BACK_STENCIL_OPS:          
            STENCIL_OP(_back.onStencilFail, acdssi, 0); 
            STENCIL_OP(_back.onStencilPassZFails, acdssi, 1); 
            STENCIL_OP(_back.onStencilPassZPass, acdssi, 2); 
            delete(acdssi);
            break;
        case ACD_ZST_RANGE_NEAR:                    _depthRangeNear = CAST_TO_FLOAT(acdssi);        delete(acdssi);     break;
        case ACD_ZST_RANGE_FAR:                     _depthRangeFar = CAST_TO_FLOAT(acdssi);         delete(acdssi);     break;
        case ACD_ZST_D3D9_DEPTH_RANGE:              _d3d9DepthRange = CAST_TO_BOOL(acdssi);         delete(acdssi);     break;
        case ACD_ZST_SLOPE_FACTOR:                  _depthSlopeFactor = CAST_TO_FLOAT(acdssi);      delete(acdssi);     break;
        case ACD_ZST_UNIT_OFFSET:                   _depthUnitOffset = CAST_TO_FLOAT(acdssi);       delete(acdssi);     break;
        case ACD_ZST_DEPTH_UPDATE_MASK:             _stencilUpdateMask = CAST_TO_UINT(acdssi);      delete(acdssi);     break;


        // case ACD_ZST_... <- add here future additional z stencil states.
        default:
            panic("ACDZStencilStageImp::","restoreStoredStateItem()","Unknown z stencil state");
    }
    
    GLOBALPROFILER_EXITREGION()
}

#undef CAST_TO_BOOL
#undef COMP_FUNC
#undef STENCIL_OP
#undef CAST_TO_UINT
#undef CAST_TO_FLOAT

ACDStoredState* ACDZStencilStageImp::saveAllState() const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    
    ACDStoredStateImp* ret = new ACDStoredStateImp();

    for (acd_uint i = 0; i < ACD_ZST_LAST; i++)
        ret->addStoredStateItem(createStoredStateItem(ACD_STORED_ITEM_ID(i)));

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

void ACDZStencilStageImp::restoreAllState(const ACDStoredState* state)
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
