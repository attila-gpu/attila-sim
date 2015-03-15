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

#ifndef CONSTEXTRACTOR_H
    #define CONSTEXTRACTOR_H

#include "ConstDescription.h"
#include <vector>

class ConstExtractor
{
private:

    enum { MAX_CONSTANTS = 10000 };

    int iConst;

    const ConstDescription* cdv[MAX_CONSTANTS];

    std::vector<char*> filter; ///< list of filters 

    bool matchWithFilter( const char* name );

    /**
     * If it is prefix returns the length of prefix
     * If it is not prefix returns -1
     */
    static int isPrefix(const char* str, const char* prefix);

    /**
     * returns max matching characters starting from the beginning
     *
     * "GL_COLOR_BIT"
     * "GL_CONVOLUTION"
     *
     * will return '6'
     */
    static int maxMatch( const char* str1, const char* str2 );

public:

    ~ConstExtractor();

    ConstExtractor();
    
    int extractConstants( const char* file );

    int count() const;

    /**
     * O(1)
     */
    const ConstDescription* getConstant( int pos ) const;

    const ConstDescription* getConstant( const char* name ) const;

    /**
     * Returs all constant positions of constants with value 'value'
     *
     * set maxPositions to -1 for get all of them
     */
    std::vector<int>* getConstantPosition( int value, int maxPositions ) const;

    /**
     * Returns one of the constants with value 'value'
     */
    int getConstantPosition( int value ) const;

    /**
     * Return constants position using name for searching
     */
    int getConstantPosition( const char* name ) const;

    void addFilter( const char* aFilter );
    
    void dump();

};

#endif // CONSTEXTRACTOR_H
