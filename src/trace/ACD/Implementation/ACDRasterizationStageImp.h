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

#ifndef ACD_RASTERIZATIONSTAGE_IMP
    #define ACD_RASTERIZATIONSTAGE_IMP

#include "ACDTypes.h"
#include "ACDRasterizationStage.h"
#include "GPUDriver.h"
#include <string>
#include "StateItem.h"
#include "StateComponent.h"
#include "StateItemUtils.h"
#include "ACDStoredStateImp.h"
#include "ACDStoredItemID.h"

namespace acdlib
{

class ACDDeviceImp;
class StoredStateItem;

class ACDRasterizationStageImp : public ACDRasterizationStage, public StateComponent
{
public:

    ACDRasterizationStageImp(ACDDeviceImp* device, GPUDriver* driver);

    virtual void setInterpolationMode(acd_uint fshInputAttribute, ACD_INTERPOLATION_MODE mode);

    virtual void enableScissor(acd_bool enable);

    virtual void setFillMode(ACD_FILL_MODE fillMode);

    virtual void setCullMode(ACD_CULL_MODE cullMode);

    virtual void setFaceMode(ACD_FACE_MODE faceMode);

    virtual void setViewport(acd_int xMin, acd_int yMin, acd_uint xMax, acd_uint yMax);

    virtual void setScissor(acd_int xMin, acd_int yMin, acd_uint xMax, acd_uint yMax);

    virtual void getViewport(acd_int &xMin, acd_int &yMin, acd_uint &xMax, acd_uint &yMax);

    virtual void getScissor(acd_bool &enabled, acd_int &xMin, acd_int &yMin, acd_uint &xMax, acd_uint &yMax);

    virtual void useD3D9RasterizationRules(acd_bool use);
    
    virtual void useD3D9PixelCoordConvention(acd_bool use);

    // StateComponent interface

    /**
     * Writes all the rasterization state that requires update
     */
    void sync();

    /**
     * Writes all the rasterization state without checking if updating is required
     */
    void forceSync();

    const StoredStateItem* createStoredStateItem(ACD_STORED_ITEM_ID stateId) const;

    void restoreStoredStateItem(const StoredStateItem* ssi);

    ACDStoredState* saveAllState() const;

    void restoreAllState(const ACDStoredState* ssi);

    std::string getInternalState() const;

private:

    std::vector<StateItem<ACD_INTERPOLATION_MODE> > _interpolation;

    GPUDriver* _driver;

    ACDDeviceImp* _device;

    acd_bool _syncRequired;

    // Fill mode state
    StateItem<ACD_FILL_MODE> _fillMode;

    // Culling state
    StateItem<ACD_CULL_MODE> _cullMode;

    // Facing state
    StateItem<ACD_FACE_MODE> _faceMode;

    // Viewport state
    StateItem<acd_uint> _xViewport;
    StateItem<acd_uint> _yViewport;
    StateItem<acd_uint> _widthViewport;
    StateItem<acd_uint> _heightViewport;

    // Scissor state
    StateItem<acd_bool> _scissorEnabled;
    StateItem<acd_uint> _xScissor;
    StateItem<acd_uint> _yScissor;
    StateItem<acd_uint> _widthScissor;
    StateItem<acd_uint> _heightScissor;

    //  Rasterization rules.
    StateItem<acd_bool> _useD3D9RasterizationRules;
    
    //  Pixel coordinates convention
    StateItem<acd_bool> _useD3D9PixelCoordConvention;
    
    static void _getGPUCullMode(ACD_CULL_MODE mode, gpu3d::GPURegData* data);
    static void _getGPUFaceMode(ACD_FACE_MODE mode, gpu3d::GPURegData* data);
    static void _getGPUInterpolationMode(ACD_INTERPOLATION_MODE mode, gpu3d::GPURegData* data);

    //  Allow the device implementation class to access private attributes.
    friend class ACDDeviceImp;

    class CullModePrint: public PrintFunc<ACD_CULL_MODE>
    {
    public:

        virtual const char* print(const ACD_CULL_MODE& cullMode) const;
    } cullModePrint;

    class InterpolationModePrint : public PrintFunc<ACD_INTERPOLATION_MODE>
    {
        virtual const char* print(const ACD_INTERPOLATION_MODE& iMode) const;
    } interpolationModePrint;

};

}

#endif // ACD_RASTERIZATIONSTAGE_IMP
