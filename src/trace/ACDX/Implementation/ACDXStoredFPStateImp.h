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

#ifndef ACDX_STORED_FP_STATE_IMP_H
    #define ACDX_STORED_FP_STATE_IMP_H

#include "ACDXStoredFPState.h"
#include "StateItem.h"
#include <list>
#include "StoredStateItem.h"
#include "ACDXGlobalTypeDefinitions.h"
#include "ACDXStoredFPItemID.h"

namespace acdlib
{

class ACDXStoredFPStateItem: public StoredStateItem
{
protected:
    
    ACDX_STORED_FP_ITEM_ID    _itemId;

public:

    ACDX_STORED_FP_ITEM_ID getItemId() const {    return _itemId;    }

    void setItemId(ACDX_STORED_FP_ITEM_ID itemId)    {    _itemId = itemId;    }

    virtual ~ACDXStoredFPStateItem() = 0;
};

class ACDXFloatVector4StoredStateItem: public ACDXStoredFPStateItem
{
private:

    ACDXFloatVector4    _value;

public:
    
    ACDXFloatVector4StoredStateItem(ACDXFloatVector4 value) : _value(value) {};

    inline operator const ACDXFloatVector4&() const { return _value; }

    virtual ~ACDXFloatVector4StoredStateItem() {;};
};

class ACDXFloatVector3StoredStateItem: public ACDXStoredFPStateItem
{
private:

    ACDXFloatVector3    _value;

public:

    ACDXFloatVector3StoredStateItem(ACDXFloatVector3 value)    : _value(value) {};

    inline operator const ACDXFloatVector3&() const { return _value; }

    virtual ~ACDXFloatVector3StoredStateItem() {;};
};

class ACDXSingleFloatStoredStateItem: public ACDXStoredFPStateItem
{
private:

    acd_float    _value;

public:

    ACDXSingleFloatStoredStateItem(acd_float value) : _value(value) {};

    inline operator const acd_float&() const { return _value; }

    virtual ~ACDXSingleFloatStoredStateItem() {;};
};

class ACDXFloatMatrix4x4StoredStateItem: public ACDXStoredFPStateItem
{
private:

    ACDXFloatMatrix4x4    _value;

public:
    
    ACDXFloatMatrix4x4StoredStateItem(ACDXFloatMatrix4x4 value)    : _value(value) {};

    inline operator const ACDXFloatMatrix4x4&() const { return _value; }

    virtual ~ACDXFloatMatrix4x4StoredStateItem() {;};
};


class ACDXStoredFPStateImp: public ACDXStoredFPState
{
//////////////////////////
//  interface extension //
//////////////////////////
private:

    std::list<const StoredStateItem*> _ssiList;

public:
    
    ACDXStoredFPStateImp();

    void addStoredStateItem(const StoredStateItem* ssi);

    std::list<const StoredStateItem*> getSSIList() const;
};

} // namespace acdlib

#endif // ACDX_STORED_FP_STATE_IMP_H
