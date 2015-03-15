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
 * $RCSfile: DynamicObject.h,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-10-10 10:57:09 $
 *
 * Dynamic Object definitions file.
 *
 */

#ifndef _DYNAMIC_OBJECT_H_
    #define _DYNAMIC_OBJECT_H_

#include "GPUTypes.h"
#include "OptimizedDynamicMemory.h"
#include <string>

namespace gpu3d
{

/**
 * Must be inherited by all traceable Objects used in Signal's traffic
 *
 * @date 29/05/2003
 */
class DynamicObject : public OptimizedDynamicMemory
{

private:

    /*  Weird way to define a private constants for a class ...  */
    enum { MAX_COOKIES = 8 };
    enum { MAX_INFO_SIZE = 255 };

    u32bit cookies[MAX_COOKIES];    ///< array of tracing cookies
    u32bit lastCookie;              ///< last object cookie
    u32bit color;                   ///< color object
    u8bit  info[MAX_INFO_SIZE];     ///< additional info ( i.e text )

    static u32bit nextCookie[];     ///< last cookie generated ( in a level )

public:

    /**
     * Creates a new DynamicObject without any information
     */
    DynamicObject();

    /**
     * Creates a new DynamicObject initialized with one cookie ( first level cookie )
     *
     * @param aCookie identifier of the first level cookie
     */
    DynamicObject( u32bit aCookie);

    /**
     * Creates a new DynamicObject initialized with one cookie ( first level cookie )
     * and a specified color
     *
     * @param aCookie identifier of the first level cookie
     * @param color color identifier
     */
    DynamicObject( u32bit aCookie, u32bit color);

    /**
     * Overwrites all cookies in this DynamicObject with the cookies in the parent DynamicObject
     * specified as parameter ( tipically used to simulate the inheritance of cookies )
     *
     * @param parent from where cookies are copied
     */
    void copyParentCookies( const DynamicObject& parent );

    /**
     * Modifies the cookie identifier of the current cookie level
     *
     * @param aCookie new value for the current level cookie
     */
    void setCookie( u32bit aCookie );

    /**
     * Sets a new Color for this DynamicObject
     *
     * @param aColor color identifier
     */
    void setColor( u32bit aColor );

    /**
     * Adds a new cookie in the next level
     *
     * @param aCookie cookie identifier
     */
    void addCookie( u32bit aCookie );

    /**
     * Adds a new cookie ( selected automatically by the cookie generator )
     */
    void addCookie();

    /**
     *
     *  Removes a cookie.
     *
     */

    void removeCookie();

    /**
     * Obtains a maximum of maxCookies ( first maxCookies ) for this DynamicObject.
     *
     * @param numCookies Reference to a variable where to store
     *  the current number of cookies for the dynamic object.
     *
     * @return The pointer to the array of cookies for the
     * dynamic object.
     */
    u32bit *getCookies( u32bit &numCookies );

    /**
     * Gets the color for this DynamicObject
     *
     * @return the color of this DynamicObject
     */
    u32bit getColor();

    /**
     * Sets a new info for this DynamicObject ( replacing the current info )
     *
     * @param info information that is going to be attached to this DynamicObject
     */
    //void setInfo( u8bit* info );

    /**
     * Obtains the info associated to this DynamicObject
     *
     * @return string representing the information associated to this DynamicObject
     */
    u8bit* getInfo();

    const u8bit* getInfo() const;

    virtual  std::string toString() const;

};

} // namespace gpu3d

#endif
