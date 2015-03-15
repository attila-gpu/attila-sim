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

#include "ACDStoredStateImp.h"

using namespace acdlib;

ACDStoredStateItem::~ACDStoredStateItem()
{
};

ACDStoredStateImp::ACDStoredStateImp()
{
};

void ACDStoredStateImp::addStoredStateItem(const StoredStateItem* ssi)
{
    _ssiList.push_back(ssi);
}

std::list<const StoredStateItem*> ACDStoredStateImp::getSSIList() const
{
    return _ssiList;
}
