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

#ifndef STORED_STATE_ITEM_H
    #define STORED_STATE_ITEM_H

namespace acdlib
{

class StoredStateItem
{
public:
    /**
     * @note This pure virtual destructor with a defined body allows
     * deleting StoredStateItem objects from the "StoredStateItem"
     * pointer level, being sure that the subclasses destructors will
     * be called properly from the lowest to the highest level in the
     * hierarchy.
     */
    virtual ~StoredStateItem() = 0;
};


} // namespace acdlib

#endif // STORED_STATE_ITEM_H
