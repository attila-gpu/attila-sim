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

#ifndef ACD_BLENDINGSTAGE_IMP
    #define ACD_BLENDINGSTAGE_IMP

#include "ACDTypes.h"
#include "ACDVector.h"
#include "ACDBlendingStage.h"
#include "GPUDriver.h"
#include "StateItem.h"
#include "StateComponent.h"
#include "StateItemUtils.h"
#include "ACDStoredItemID.h"
#include "ACDStoredStateImp.h"
#include <string>

namespace acdlib
{

class ACDDeviceImp;
class StoredStateItem;

class ACDBlendingStageImp : public ACDBlendingStage, public StateComponent
{
public:

    ACDBlendingStageImp(ACDDeviceImp* device, GPUDriver* driver);

    virtual void setEnable(acd_uint renderTargetID, acd_bool enable);
    virtual void setSrcBlend(acd_uint renderTargetID, ACD_BLEND_OPTION srcBlend);
    virtual void setDestBlend(acd_uint renderTargetID, ACD_BLEND_OPTION destBlend);
    virtual void setBlendFunc(acd_uint renderTargetID, ACD_BLEND_FUNCTION blendOp);
    virtual void setSrcBlendAlpha(acd_uint renderTargetID, ACD_BLEND_OPTION srcBlendAlpha);
    virtual void setDestBlendAlpha(acd_uint renderTargetID, ACD_BLEND_OPTION destBlendAlpha);
    virtual void setBlendFuncAlpha(acd_uint renderTargetID, ACD_BLEND_FUNCTION blendOpAlpha);

    virtual void setBlendColor(acd_uint renderTargetID, acd_float red, acd_float green, acd_float blue, acd_float alpha);
    virtual void setBlendColor(acd_uint renderTargetID, const acd_float* rgba);

    virtual void setColorMask(acd_uint renderTargetID, acd_bool red, acd_bool green, acd_bool blue, acd_bool alpha);
    virtual void disableColorWrite(acd_uint renderTargetID);
    virtual void enableColorWrite(acd_uint renderTargetID);
    virtual acd_bool getColorEnable(acd_uint renderTargetID);
    
    std::string getInternalState() const;

    void sync();

    void forceSync();

    const StoredStateItem* createStoredStateItem(ACD_STORED_ITEM_ID stateId) const;

    void restoreStoredStateItem(const StoredStateItem* ssi);

    ACDStoredState* saveAllState() const;

    void restoreAllState(const ACDStoredState* ssi);

private:

    static const acd_uint MAX_RENDER_TARGETS = ACD_MAX_RENDER_TARGETS;

    acd_bool _syncRequired;
    GPUDriver* _driver;

    std::vector<StateItem<acd_bool> > _enabled;

    std::vector<StateItem<ACD_BLEND_FUNCTION> > _blendFunc;
    std::vector<StateItem<ACD_BLEND_OPTION> > _srcBlend;
    std::vector<StateItem<ACD_BLEND_OPTION> > _destBlend;

    std::vector<StateItem<ACD_BLEND_FUNCTION> > _blendFuncAlpha; // Not supported by Attila Simulator
    std::vector<StateItem<ACD_BLEND_OPTION> > _srcBlendAlpha;
    std::vector<StateItem<ACD_BLEND_OPTION> > _destBlendAlpha;

    typedef ACDVector<acd_float,4> ACDFloatVector4;

    std::vector<StateItem<ACDFloatVector4> > _blendColor;

    // Color Mask
    std::vector<StateItem<acd_bool> > _redMask;
    std::vector<StateItem<acd_bool> > _greenMask;
    std::vector<StateItem<acd_bool> > _blueMask;
    std::vector<StateItem<acd_bool> > _alphaMask;
    
    std::vector<StateItem<acd_bool> > _colorWriteEnabled;

    static void _getGPUBlendOption(ACD_BLEND_OPTION option, gpu3d::GPURegData* data);
    static void _getGPUBlendFunction(ACD_BLEND_FUNCTION func, gpu3d::GPURegData* data);
    
    class BlendOptionPrint: public PrintFunc<ACD_BLEND_OPTION>
    {
    public:

        virtual const char* print(const ACD_BLEND_OPTION& var) const;        

    } blendOptionPrint;

    class BlendFunctionPrint: public PrintFunc<ACD_BLEND_FUNCTION>
    {
    public:
    
        virtual const char* print(const ACD_BLEND_FUNCTION& var) const;        
    
    } blendFunctionPrint;

    BoolPrint boolPrint;
};

}


#endif // ACD_BLENDINGSTAGE_IMP
