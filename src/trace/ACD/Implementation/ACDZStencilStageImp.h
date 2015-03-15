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

#ifndef ACD_ZSTENCILSTAGE_IMP
    #define ACD_ZSTENCILSTAGE_IMP

#include "ACDTypes.h"
#include "ACDZStencilStage.h"
#include "GPUDriver.h"
#include "StateItem.h"
#include "StateComponent.h"
#include "StateItemUtils.h"
#include "ACDStoredItemID.h"
#include "ACDStoredStateImp.h"

namespace acdlib
{

class ACDDeviceImp;
class StoredStateItem;

class ACDZStencilStageImp : public ACDZStencilStage, public StateComponent
{
public:

    ACDZStencilStageImp(ACDDeviceImp* device, GPUDriver* driver);

    virtual void setZEnabled(acd_bool enable);

    virtual acd_bool isZEnabled() const;

    virtual void setZFunc(ACD_COMPARE_FUNCTION zFunc);

    virtual ACD_COMPARE_FUNCTION getZFunc() const;

    virtual void setZMask(acd_bool mask);

    virtual acd_bool getZMask() const;

    virtual void setStencilEnabled(acd_bool enable);

    virtual acd_bool isStencilEnabled() const;

    virtual void setStencilOp( ACD_FACE face, 
                               ACD_STENCIL_OP onStencilFail,
                               ACD_STENCIL_OP onStencilPassZFail,
                               ACD_STENCIL_OP onStencilPassZPass);
    virtual void getStencilOp( ACD_FACE face,
                               ACD_STENCIL_OP& onStencilFail,
                               ACD_STENCIL_OP& onStencilPassZFail,
                               ACD_STENCIL_OP& onStencilPassZPass) const;

    virtual void setStencilFunc( ACD_FACE face, ACD_COMPARE_FUNCTION func, acd_uint stencilRef, acd_uint mask );

    virtual void setDepthRange (acd_float near, acd_float far);

    virtual void setD3D9DepthRangeMode(acd_bool mode);
    
    virtual void setPolygonOffset (acd_float factor, acd_float units);

    virtual void setStencilUpdateMask (acd_uint mask);

    virtual void setZStencilBufferDefined(acd_bool enable);
    
    void sync();

    void forceSync();

    const StoredStateItem* createStoredStateItem(ACD_STORED_ITEM_ID stateId) const;

    void restoreStoredStateItem(const StoredStateItem* ssi);

    ACDStoredState* saveAllState() const;

    void restoreAllState(const ACDStoredState* ssi);

    std::string getInternalState() const;

private:

    ACDDeviceImp* _device;
    GPUDriver* _driver;
    acd_bool _syncRequired;

    StateItem<acd_bool> _zEnabled;
    StateItem<acd_bool> _stencilEnabled;
    StateItem<acd_float> _depthRangeNear;
    StateItem<acd_float> _depthRangeFar;
    StateItem<acd_bool> _d3d9DepthRange;
    
    StateItem<ACD_COMPARE_FUNCTION> _zFunc;
    StateItem<acd_bool> _zMask;

    StateItem<acd_float> _depthSlopeFactor;
    StateItem<acd_float> _depthUnitOffset;

    StateItem<acd_uint> _stencilUpdateMask;

    StateItem<acd_uint> _zStencilBufferDefined;
    
    // Per face state
    struct _FaceInfo
    {
        StateItem<ACD_STENCIL_OP> onStencilFail;
        StateItem<ACD_STENCIL_OP> onStencilPassZFails;
        StateItem<ACD_STENCIL_OP> onStencilPassZPass;

        StateItem<ACD_COMPARE_FUNCTION> stencilFunc;
        StateItem<acd_uint> stencilRef;
        StateItem<acd_uint> stencilMask;

        _FaceInfo(ACD_STENCIL_OP onStencilFail, ACD_STENCIL_OP onStencilPassZFails, ACD_STENCIL_OP onStencilPassZPass,
                  ACD_COMPARE_FUNCTION stencilFunc, acd_uint stencilRef, acd_uint stencilMask) :
                onStencilFail(onStencilFail),
                onStencilPassZFails(onStencilPassZFails),
                onStencilPassZPass(onStencilPassZPass),
                stencilFunc(stencilFunc),
                stencilRef(stencilRef),
                stencilMask(stencilMask)
        {}
    };

    _FaceInfo _front;
    _FaceInfo _back;

    static void _getGPUCompareFunction(ACD_COMPARE_FUNCTION comp, gpu3d::GPURegData* data);
    static void _getGPUStencilOperation(ACD_STENCIL_OP op, gpu3d::GPURegData* data);
    
    class CompareFunctionPrint: public PrintFunc<ACD_COMPARE_FUNCTION>
    {
    public:

        virtual const char* print(const ACD_COMPARE_FUNCTION& var) const;
    } compareFunctionPrint;

    class StencilOperationPrint: public PrintFunc<ACD_STENCIL_OP>
    {
    public:

        virtual const char* print(const ACD_STENCIL_OP& var) const;
    } stencilOperationPrint;

    //static const char* _getCompareFunctionString(ACD_COMPARE_FUNCTION comp);
    //static const char* _getStencilOperationString(ACD_STENCIL_OP op);

    BoolPrint boolPrint;
};

}

#endif // ACD_ZSTENCILSTAGE_IMP
