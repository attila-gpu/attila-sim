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
 * $RCSfile: DynamicObject.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-11-11 15:41:49 $
 *
 * Dynamic Object implementation file.
 *
 */

#include "DynamicObject.h"
#include "support.h"
#include <iostream>

using namespace gpu3d;

u32bit DynamicObject::nextCookie[MAX_COOKIES]; // static implies zero initialization automatically

DynamicObject::DynamicObject() : lastCookie( 0 ), color(0)
{
    cookies[lastCookie] = nextCookie[lastCookie]++;

    /*  Clear info field (zero string).  */
    info[0] = 0;

    setTag("Dyn");
}

/*  Creates a dynamic object with one cookie.  The cookie is assigned from
    the internal cookie generator.  */
DynamicObject::DynamicObject( u32bit aCookie ) : lastCookie( 0 ), color(0)
{
    cookies[lastCookie] = aCookie;

    /*  Clear info field (zero string).  */
    info[0] = 0;
}

/*  Creates a dynamic object with one cookie and sets its color.  */
DynamicObject::DynamicObject( u32bit aCookie, u32bit aColor ) : lastCookie( 0 )
{
    cookies[lastCookie] = aCookie;
    color = aColor;

    /*  Clear info field (zero string).  */
    info[0] = 0;
}

/*  Copies cookie list from another dynamic object.  */
void DynamicObject::copyParentCookies( const DynamicObject& parent )
{
    for ( lastCookie = 0; lastCookie <= parent.lastCookie; lastCookie++ )
        cookies[lastCookie] = parent.cookies[lastCookie];

    /*  The for adds one false cookie.  */
    lastCookie--;
}

/*  Sets dynamic object last cookie.  */
void DynamicObject::setCookie( u32bit aCookie )
{
    cookies[lastCookie] = aCookie;
}

/*  Adds a new cookie level.  */
void DynamicObject::addCookie( u32bit aCookie )
{
    lastCookie++;

    cookies[lastCookie] = aCookie;
}

/*  Removes a cookie level for the object.  */
void DynamicObject::removeCookie()
{
    if (lastCookie > 0)
        lastCookie--;
}

/*  Adds a new cookie level using the internal cookie generator.  */
void DynamicObject::addCookie()
{
    lastCookie++;

    cookies[lastCookie] = nextCookie[lastCookie]++; // increase cookie level generator
}

/*  Sets dynamic object color.  */
void DynamicObject::setColor( u32bit aColor )
{
    color = aColor;
}

/*  Returns a pointer the dynamic object cookies list.  */
u32bit *DynamicObject::getCookies( u32bit &numCookies )
{
    numCookies = lastCookie + 1;

    return cookies;
}

/*  Returns the object color.  */
u32bit DynamicObject::getColor()
{
    return color;
}

/*  Sets dynamic object info field.  */
//void DynamicObject::setInfo( u8bit* info )
//{
//    this->info = info;
//}

/*  Returns dynamic object info field.  */
u8bit* DynamicObject::getInfo()
{
    return info;
}

const u8bit* DynamicObject::getInfo() const
{
    return info;
}

std::string DynamicObject::toString() const
{
    return OptimizedDynamicMemory::toString() + "   INFO: \"" + std::string((const char*)getInfo()) + "\"";
}

