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

#include "ARBRegisterBank.h"
#include "support.h"
#include <cstring>

using namespace std;
using namespace agl;

ARBRegisterBank::ARBRegisterBank(GLuint registerCount) : _RegisterCount(250)
{
    _registers = new _Register[250];
    for(GLuint reg = 0; reg < 250; reg++)
        memset(_registers[reg], 0, 4*sizeof(GLfloat));
}

void ARBRegisterBank::set(GLuint reg, GLfloat coord1, GLfloat coord2, GLfloat coord3, 
                          GLfloat coord4, GLubyte mask)
{
    GLfloat aux[4];
    aux[0] = coord1;
    aux[1] = coord2;
    aux[2] = coord3;
    aux[3] = coord4;

    set(reg, aux, mask);
}

const GLfloat* ARBRegisterBank::get(GLuint reg) const
{
    if ( reg >= _RegisterCount )
        panic("ARBARBProgramObject", "get", "Register identifier out of bounds");

    return _registers[reg];
}

void ARBRegisterBank::get(GLuint reg, GLfloat* coords) const
{
    if ( reg >= _RegisterCount )
        panic("ARBARBProgramObject", "get", "Register identifier out of bounds");

    memcpy(coords, _registers[reg], 4 * sizeof(GLfloat));
}

void ARBRegisterBank::get(GLuint reg, GLfloat& coord1, GLfloat& coord2, GLfloat& coord3, GLfloat& coord4) const
{
    if ( reg >= _RegisterCount )
        panic("ARBARBProgramObject", "get", "Register identifier out of bounds");

    coord1 = _registers[reg][0];
    coord2 = _registers[reg][1];
    coord3 = _registers[reg][2];
    coord4 = _registers[reg][3];
}

void ARBRegisterBank::set(GLuint reg, const GLfloat* coords, GLubyte mask)
{
    if ( reg >= _RegisterCount )
        panic("ARBARBProgramObject", "set", "Register identifier out of bounds");

    if ( (mask & 0xF) == 0xF ) { // Write without mask
        if ( memcmp(_registers[reg], coords, 4*sizeof(GLfloat)) == 0 )
            return ;
        memcpy(_registers[reg], coords, 4*sizeof(GLfloat));
        _modified.insert(reg);
    }
    else { 
        // masked write
        bool changed = false;
        GLfloat* ptr = _registers[reg];
        GLubyte currentMask = 0x8;
        for ( GLuint i = 0; i < 4; ++i, ++ptr ) {
            if ( (currentMask & mask) == currentMask && *ptr != coords[i] ) {
                *ptr = coords[i];
                changed = true;
            }
            currentMask = currentMask >> 1;
        }

        if ( changed )
            _modified.insert(reg);
    }
}


vector<GLuint> ARBRegisterBank::getModified() const
{
    return vector<GLuint>(_modified.begin(), _modified.end());    
}
