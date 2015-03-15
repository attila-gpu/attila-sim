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

#ifndef ACD_STORED_STATE_IMP_H
    #define ACD_STORED_STATE_IMP_H

#include "ACDStoredState.h"
#include "StateItem.h"
#include <list>
#include "StoredStateItem.h"
#include "ACDStoredItemID.h"
#include "ACDVector.h"

namespace acdlib
{

class ACDStoredStateItem: public StoredStateItem
{
protected:
    
    ACD_STORED_ITEM_ID    _itemId;

public:

    ACD_STORED_ITEM_ID getItemId() const {    return _itemId;    }

    void setItemId(ACD_STORED_ITEM_ID itemId)    {    _itemId = itemId;    }

    virtual ~ACDStoredStateItem() = 0;
};

/*
class ACDXFloatVector4StoredStateItem: public ACDStoredStateItem
{
private:

    ACDXFloatVector4    _value;

public:
    
    ACDXFloatVector4StoredStateItem(ACDXFloatVector4 value) : _value(value) {};

    inline operator const ACDXFloatVector4&() const { return _value; }

    virtual ~ACDXFloatVector4StoredStateItem() { ; }
};

class ACDXFloatVector3StoredStateItem: public ACDXStoredFPStateItem
{
private:

    ACDXFloatVector3    _value;

public:

    ACDXFloatVector3StoredStateItem(ACDXFloatVector3 value)    : _value(value) {};

    inline operator const ACDXFloatVector3&() const { return _value; }

    virtual ~ACDXFloatVector3StoredStateItem() { ; }
};
*/

class ACDSingleBoolStoredStateItem: public ACDStoredStateItem
{
private:

    acd_bool _value;

public:

    ACDSingleBoolStoredStateItem(acd_bool value) : _value(value) {};

    inline operator const acd_bool&() const { return _value; }

    virtual ~ACDSingleBoolStoredStateItem() { ; }
};

class ACDSingleFloatStoredStateItem: public ACDStoredStateItem
{
private:

    acd_float    _value;

public:

    ACDSingleFloatStoredStateItem(acd_float value) : _value(value) {};

    inline operator const acd_float&() const { return _value; }

    virtual ~ACDSingleFloatStoredStateItem() { ; }
};

class ACDSingleUintStoredStateItem: public ACDStoredStateItem
{
private:

    acd_uint    _value;

public:

    ACDSingleUintStoredStateItem(acd_uint value) : _value(value) {};

    inline operator const acd_uint&() const { return _value; }

    virtual ~ACDSingleUintStoredStateItem() { ; }
};

class ACDSingleEnumStoredStateItem: public ACDStoredStateItem
{
private:

    acd_enum _value;

public:

    ACDSingleEnumStoredStateItem(acd_enum value) : _value(value) {};

    inline operator const acd_enum&() const { return _value; }

    virtual ~ACDSingleEnumStoredStateItem() { ; }
};

class ACDSingleVoidStoredStateItem: public ACDStoredStateItem
{
private:

    void* _value;

public:

    ACDSingleVoidStoredStateItem(void* value) : _value(value) {};

    inline operator void*() const { return _value; }

    virtual ~ACDSingleVoidStoredStateItem() { ; }
};

typedef ACDVector<acd_float,4> ACDFloatVector4;

class ACDFloatVector4StoredStateItem: public ACDStoredStateItem
{
private:

    ACDFloatVector4 _value;

public:

    ACDFloatVector4StoredStateItem(ACDFloatVector4 value) : _value(value) {};

    inline operator const ACDFloatVector4&() const { return _value; }

    virtual ~ACDFloatVector4StoredStateItem() { ; }
};

typedef ACDVector<acd_enum,3> ACDEnumVector3;

class ACDEnumVector3StoredStateItem: public ACDStoredStateItem
{
private:

    ACDEnumVector3 _value;

public:

    ACDEnumVector3StoredStateItem(ACDEnumVector3 value) : _value(value) {};

    inline operator const ACDEnumVector3&() const { return _value; }

    virtual ~ACDEnumVector3StoredStateItem() { ; }
};

class ACDViewportStoredStateItem: public ACDStoredStateItem
{
private:

    acd_int _xViewport;
    acd_int _yViewport;
    acd_uint _widthViewport;
    acd_uint _heightViewport;

public:

    ACDViewportStoredStateItem(acd_int xViewport, acd_int yViewport, acd_uint widthViewport, acd_uint heightViewport) 
        : _xViewport(xViewport), _yViewport(yViewport), _widthViewport(xViewport), _heightViewport(xViewport) {};

    acd_int getXViewport() const { return _xViewport; } 
    acd_int getYViewport() const { return _yViewport; } 
    acd_uint getWidthViewport() const { return _widthViewport; } 
    acd_uint getHeightViewport() const { return _heightViewport; }

    virtual ~ACDViewportStoredStateItem() { ; }
};

/*
class ACDXFloatMatrix4x4StoredStateItem: public ACDXStoredFPStateItem
{
private:

    ACDXFloatMatrix4x4    _value;

public:
    
    ACDXFloatMatrix4x4StoredStateItem(ACDXFloatMatrix4x4 value)    : _value(value) {};

    inline operator const ACDXFloatMatrix4x4&() const { return _value; }

    virtual ~ACDXFloatMatrix4x4StoredStateItem() { ; }
};
*/

class ACDStoredStateImp: public ACDStoredState
{
//////////////////////////
//  interface extension //
//////////////////////////
private:

    std::list<const StoredStateItem*> _ssiList;

public:
    
    ACDStoredStateImp();

    void addStoredStateItem(const StoredStateItem* ssi);

    std::list<const StoredStateItem*> getSSIList() const;
};

} // namespace acdlib

#endif // ACD_STORED_STATE_IMP_H
