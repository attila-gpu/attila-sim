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

#ifndef CONSTDESCRIPTION_H
    #define CONSTDESCRIPTION_H



class ConstDescription
{
private:
    
    char* name;
    int value;

    ConstDescription();
    ConstDescription( const ConstDescription& );

    static int parseHex( const char* hexStr );

    static char toStringBuffer[];

public:

    static const ConstDescription* parseConstant( const char* constantDef );

    int getValue() const;

    const char* getString() const;

    const char* toString() const;

    ~ConstDescription();
};


#endif
