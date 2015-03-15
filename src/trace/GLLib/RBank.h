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

#ifndef RBANK_H
    #define RBANK_H

#include "QuadReg.h"
#include <iostream>
#include <string>
#include "gl.h"
#include "support.h"
#include "GPUTypes.h"
#include <cstring>

enum RType
{
    QR_UNKNOWN,
    QR_CONSTANT,
    QR_GLSTATE,
    QR_PARAM_LOCAL,
    QR_PARAM_ENV
};

/**
 * class implementing a generic RBank Register of four values per register
 *
 * @note Implemented using templates
 *
 * @author Carlos González Rodríguez - cgonzale@ac.upc.es
 * @date 5/7/2004
 * @ver 1.0
 *
 */
template<class T>
class RBank
{
private:

    static QuadReg<T> dummy;

    std::string name; ///< RBank name
    QuadReg<T>* bank; ///< RBank registers
    RType* types; ///< RBank registers type
    int* indexes0; ///< RBank register indexes 0
    int* indexes1; ///< RBank register indexes 1
    bool* used; ///< bitmap of registers used
    GLuint count; ///< number of registers within the RBank


public:
/*
    RBank() : count(0)
    {
        // default
        bank = NULL;
        types = NULL;
        used = NULL;
        indexes0 = NULL;
        indexes1 = NULL;
        //panic("RBank","RBank","Empty RBank created");
        warning("RBank","RBank constructor","Empty RBank created");
    }
*/
    /**
     * Creates a RBank of a given size
     *
     * @param size Number of registers
     */
    RBank(GLuint size, const std::string& name="");

    RBank(const RBank<T>& rb);

    void setSize( GLuint size );

    /**
     *
     *  RBank destructor.
     *
     */

    ~RBank();

    /**
     * Gets the contents of a given register in the RBank
     *
     * @param pos Register position in the RBank
     *
     * @return register in position pos
     */
    QuadReg<T>& get(GLuint pos) const;

    /**
     * Gets the contents of a given register in the RBank
     *
     * @param pos Register position in the RBank
     *
     * @return register in position pos
     *
     * @note exact behaviour than RBank<T>::get()
     */
    QuadReg<T>& operator[](GLuint pos);

    /**
     * Sets a register in the RBank with new value
     *
     * The register is also marked as used
     *
     * @param pos Register position in the RBank
     * @param value new contents
     */
    void set(GLuint pos, const QuadReg<T>& value);

    /**
     * Mark this register as unused
     *
     * @param pos Register position in the RBank

     * @note Contents are reset to zero
     */
    void clear(GLuint pos);

    /**
     * Get the type related with the register (QR_CONSTANT,    QR_GLSTATE,    QR_PARAM_LOCAL,    QR_PARAM_ENV) and
     * related indexes.
     *
     * @param pos Register position in the RBank
     * @param index0 In case of QR_PARAM_LOCAL or QR_PARAM_ENV is the index of parameter. In
     * case of QR_GLSTATE is the state identifier. Has no sense otherwise.
     * @param index1 In case of QR_GLSTATE is the row of the matrix or 0
     * @return type of register
     * @note Registers with uninitialized type returns QR_UNKNOWN. The register position has to be correct.
     */
    RType getType(GLuint pos, int& index0, int& index1) const;

    /**
     * Set the type related with the register and type parameters (indexes)
     *
     * @param pos Register position in the RBank
     * @param type Type of register.
     * @param index0 In case of QR_PARAM_LOCAL or QR_PARAM_ENV is the index of parameter. In
     * case of QR_GLSTATE is the state identifier. Has no sense otherwise.
     * @param index1 In case of QR_GLSTATE is the row of the matrix or 0
     * @note The register position has to be correct.
     */
    void setType(GLuint pos, RType type = QR_UNKNOWN, int index0 = 0, int index1 = 0);

    /**
     * Get the first register containing a specified value
     *
     * If a register with the specified value is not found then a free register is returned
     *
     * @param value Register value searched
     * @found out parameter indicating if a register with the specified value was found
     *
     * @retval found true if the register was found, false otherwise
     * @return position of a register found or position ofa new register or equals to size()
     * if there are not free registers
     */
    GLuint getRegister(bool& found, const QuadReg<T>& value, RType regType = QR_UNKNOWN, int index0 = 0, int index1 = 0) const;

    /**
     * Find a free (unused register)
     *
     * @return Register position in the RBank
     */
    GLuint findFree() const;

    /**
     * gets RBank's size
     *
     * @return RBank's size
     */
    GLuint size() const;

    /**
     * Tests if a register in the RBank is being used
     *
     * @parameter pos Register position in the RBank
     *
     * @return true if the register is being used, false otherwise
     */
    bool usedPosition(GLuint pos) const;

    void copyContents(const RBank& rb)
    {
        if (count != rb.count)
            panic("RBank","copyContents","Target bank has not the same size");

        for ( int i = 0; i < rb.count && i < count; i++ )
        {
            bank[i] = rb.bank[i];
            used[i] = rb.used[i];
            types[i] = rb.types[i];
            indexes0[i] = rb.indexes0[i];
            indexes1[i] = rb.indexes1[i];

        }
        //count = rb.count;
    }

    virtual void print(std::ostream& os) const
    {
        os << "Register bank name: " << name.c_str() << std::endl;
        for (GLuint i = 0; i < count; i++)
        {
            if ( used[i] )
            {
                os << i << ": " << bank[i];
                switch ( types[i] )
                {
                    case QR_UNKNOWN:
                        os << 'U'; break;
                    case QR_CONSTANT:
                        os << 'C'; break;
                    case QR_PARAM_LOCAL:
                        os << " L: " << indexes0[i]; break;
                    case QR_PARAM_ENV:
                        os << " E: " << indexes0[i]; break;
                    case QR_GLSTATE:
                        os << " S: " << indexes0[i] << " row: " << indexes1[i]; break;
                    default:
                        panic("QuadReg","print","Unexpected QuadReg type");
                }
                os << std::endl;
            }
        }
    }

    RBank<T>& operator=(const RBank<T>& rb);

    friend std::ostream& operator<<(std::ostream& os, const RBank& rb)
    {
        rb.print(os);
        return os;
    }
};

template<class T>
RBank<T>::RBank(const RBank<T>& rb)
{
    name = rb.name;
    count = rb.count;
    bank = new QuadReg<T>[count];
    types = new RType[count];
    used = new bool[count];
    indexes0 = new int[count];
    indexes1 = new int[count];

    for ( u32bit i = 0; i < count; i++ )
    {
        bank[i] = rb.bank[i];
        used[i] = rb.used[i];
        types[i] = rb.types[i];
        indexes0[i] = rb.indexes0[i];
        indexes1[i] = rb.indexes1[i];
    }
}


template<class T>
RBank<T>::RBank(GLuint size, const std::string& name) : name(name)
{
    count = size;
    bank = new QuadReg<T>[count];
    types = new RType[count];
    indexes0 = new int[count];
    indexes1 = new int[count];
    used = new bool[count];
    memset(used,0,sizeof(bool)*count);
    memset(types,QR_UNKNOWN,sizeof(RType)*count);
    memset(indexes0,0,sizeof(int)*count);
    memset(indexes1,0,sizeof(int)*count);

}

template<class T>
RBank<T>::~RBank()
{
    delete[] bank;
    delete[] types;
    delete[] indexes0;
    delete[] indexes1;
    delete[] used;
}

template<class T>
RBank<T>& RBank<T>::operator=(const RBank<T>& rb)
{
    name = rb.name;
    if ( count != 0 )
    {
        delete[] bank;
        delete[] used;
        delete[] types;
        delete[] indexes0;
        delete[] indexes1;
    }

    /*  Check number of registers in the source register bank.  */
    GPU_ASSERT(
        if (rb.count == 0)
            panic("RBank<T>", "operator=(const RBank<T>& rb)", "No registers in the source register bank.");
    )

    count = rb.count;
    bank = new QuadReg<T>[count];
    used = new bool[count];
    types = new RType[count];
    indexes0 = new int[count];
    indexes1 = new int[count];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (bank == NULL)
            panic("RBank", "operator=(const RBank<T>& rb)", "Error allocating bank array.");
        if (used == NULL)
            panic("RBank", "operator=(const RBank<T>& rb)", "Error allocating used array.");
        if (types == NULL)
            panic("RBank", "operator=(const RBank<T>& rb)", "Error allocating types array.");
        if (indexes0 == NULL)
            panic("RBank", "operator=(const RBank<T>& rb)", "Error allocating indexes0 array.");
        if (indexes1 == NULL)
            panic("RBank", "operator=(const RBank<T>& rb)", "Error allocating indexes1 array.");
    )

    for ( u32bit i = 0; i < count; i++ )
    {
        bank[i] = rb.bank[i];
        used[i] = rb.used[i];
        types[i] = rb.types[i];
        indexes0[i] = rb.indexes0[i];
        indexes1[i] = rb.indexes1[i];
    }
    return *this;
}


template<class T>
GLuint RBank<T>::size() const
{
    return count;
}

template<class T>
void RBank<T>::setSize( GLuint size_ )
{
    if ( count != 0 )
    {
        delete[] bank;
        delete[] used;
        delete[] types;
        delete[] indexes0;
        delete[] indexes1;
    }

    count = size_;

    bank = new QuadReg<T>[count];
    used = new bool[count];
    types = new RType[count];
    indexes0 = new int[count];
    indexes1 = new int[count];
    memset(used,0,sizeof(bool)*count);
    memset(types,QR_UNKNOWN,sizeof(RType)*count);
    memset(indexes0,0,sizeof(int)*count);
    memset(indexes1,0,sizeof(int)*count);
}

template<class T>
QuadReg<T>& RBank<T>::get(GLuint pos) const
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("RBank","get","Invalid position out of bounds");

    return bank[pos];
}

template<class T>
QuadReg<T>& RBank<T>::operator[](GLuint pos)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("RBank","operator[]","Invalid position out of bounds");

    used[pos] = true;
    return bank[pos];
}


template<class T>
void RBank<T>::set(GLuint pos, const QuadReg<T>& value)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("RBank","set","Invalid position out of bounds");

    used[pos] = true;
    bank[pos] = value;
}

template<class T>
void RBank<T>::clear(GLuint pos)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("RBank","clear","Invalid position out of bounds");

    used[pos] = false;
    types[pos] = QR_UNKNOWN;
    QuadReg<T> q; /* init to zero */
    bank[pos] = q;
}

template<class T>
RType RBank<T>::getType(GLuint pos,int& index0, int& index1) const
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("RBank","getType","Invalid position out of bounds");

    index0 = indexes0[pos];
    index1 = indexes1[pos];
    return types[pos];
}

template<class T>
void RBank<T>::setType(GLuint pos, RType type, int index0, int index1)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("RBank","setType","Invalid position out of bounds");

    types[pos] = type;
    indexes0[pos] = index0;
    indexes1[pos] = index1;
}

template<class T>
GLuint RBank<T>::getRegister(bool& found, const QuadReg<T>& value, RType regType, int index0, int index1) const
{
    GLuint i;
    found = false;
    for ( i = 0; i < count; i++ )
    {
        if ( types[i] == regType )
        {
            if ( types[i] == QR_CONSTANT || types[i] == QR_UNKNOWN)
            {
                if ( bank[i] == value )
                {
                    found =  true;
                    return i;
                }
            }
            else if ( (types[i] == QR_PARAM_LOCAL) || (types[i] == QR_PARAM_ENV) )
            {
                if (indexes0[i] == index0)
                {
                   found =  true;
                   return i;
                }
            }
            else if ( types[i] == QR_GLSTATE )
            {
                if ( indexes0[i] == index0 && indexes1[i] == index1 )
                {
                   found = true;
                   return i;
                }
            }
            else
            {
                panic("RBank","getRegister()","Unexpected QuadReg type");
                return 0;
            }
        }

    }
    return findFree();
}


template<class T>
bool RBank<T>::usedPosition(GLuint pos) const
{
    if (pos >= count)
        panic("RBank","usedPosition","Invalid position out of bounds");

    return used[pos];
}


template<class T>
GLuint RBank<T>::findFree() const
{
    u32bit i;
    for ( i = 0; i < count; i++ )
        if ( !used[i] )
            break;
    return i;
}



template<class T>
class GPUBank : public RBank<T>
{
private:
    int readPorts;
public:

    GPUBank(GLuint size, std::string name) : RBank<T>(size,name), readPorts(0) {}

    GPUBank(GLuint size, GLuint readPorts = 0, std::string name="") :
        RBank<T>(size,name), readPorts(readPorts)
        {}

    GPUBank(const RBank<T>& rb, GLuint readPorts = 0, std::string name="") :
        RBank<T>(rb.size(),name), readPorts(readPorts)
        {
            copyContents(rb);
        }

    int getReadPorts() const { return readPorts; }

    void setReadPorts(int nReads) { readPorts = nReads; }

    void print(std::ostream& os) const
    {
        RBank<T>::print(os);
        os << "Read ports: " << readPorts << std::endl;
    }
};




#endif // RBANK_H

