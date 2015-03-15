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

#ifndef STRINGTOFUNCDESCRIPTION_H
    #define STRINGTOFUNCDESCRIPTION_H

#include "FuncDescription.h"

/**
 * Implements a very simple map between names and FuncDescription pointers
 *
 * @version 1.0
 * @date 10/11/2003
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class StringToFuncDescription
{

private:
    
    FuncDescription** funcs; ///< FuncDescription pointers

    int growth; ///< increasement if array of FuncDescription pointer must grow
    int capacity; ///< current real size of 'funcs'
    int nFuncs; ///< Number of FuncDescription pointers mapped

    /**
     * Returns the position in funcs array given a FuncDescription name
     *
     * @param name FuncDescription name
     * @return if exist a FuncDescription object with name 'name' returns its position,
     *         -1 otherwise
     */
    int findPos( const char* name ) const;

    /**
     * resizes funcs array
     *
     * the capacity of funcs array is modified, new capacity is equal to
     * capacity = capacity + increasement. If increasement is > 0 the funcs
     * array is enlarged, if increasement is < 0 funcs array is shrunk
     *
     * @warning if increasement is < 0 some pointers can be lost
     *
     * @param increasement increasement with the property given above
     * @return true if the operation was performed correctly, false otherwise
     */
    bool resize(int increasement);

public:

    /**
     * Creates a map
     *
     * Creates a map with initial capacity ( default capacity 10 )
     *
     * @param initialSize initial capacity
     */
    StringToFuncDescription( int initialSize = 10 );

    /**
     * Finds a FuncDescription object given its name
     *
     * @param FuncDescription name
     * @return if exist a FuncDescription object with name 'name' in the map,
     *         a pointer to this object is returned, null otherwise
     */
    FuncDescription* find( const char* name ) const;

    /**
     * Finds a FuncDescription object given a position in the map
     *
     * @param position in the map
     * @return FuncDescription pointer pointing to FuncDescription object in
     *         position 'pos'
     */
    FuncDescription* find( int pos ) const;
    
    /**
     * Makes a new mapping name/FuncDescription
     *
     * The name attribute in FuncDescription is used to create the map name/FuncDescription
     *
     * @param fd FuncDescription pointer
     * @return true if the method was executed correctly, false otherwise
     */
    bool insert( FuncDescription* fd );

    /**
     * Removes mapping name/FuncDescription
     *
     * @param name FuncDescription name
     * @return true if there was a map with name 'name', the map is removed. If there was not
     *         a map with name 'name' returns false
     */
    bool remove( const char* name );

    /**
     * Removes the mapping name/FuncDescription in a given position
     *
     * @param pos position in the map
     * @return true if the method was corractly executed, false otherwise
     */
    bool remove( int pos );

    /**
     * Sets new growth
     *
     * @param growth new growth
     * @return true if new growth was a value allowed ( >0 ), false otherwise
     */
    bool setGrowth( int growth );

    /**
     * Obtains the value for current growth
     *
     * @return current growth
     */
    int getGrowth() const;

    /**
     * Returns the number of mappings in the map
     *
     * @return number of mappings
     */ 
    int count() const;

    /**
     * Debug
     */
    void dump() const;
};


#endif // STRINGTOFUNCDESCRIPTION_H
