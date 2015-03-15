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

#ifndef ACDX_RBANK_H
    #define ACDX_RBANK_H

#include "ACDTypes.h"
#include "ACDXQuadReg.h"
#include <iostream>
#include <string>
#include <cstring>
#include "support.h"

namespace acdlib
{

enum ACDXRType
{
    QR_UNKNOWN,
    QR_CONSTANT,
    QR_GLSTATE,
    QR_PARAM_LOCAL,
    QR_PARAM_ENV
};

/**
 * class implementing a generic ACDXRBank Register of four values per register
 *
 * @note Implemented using templates
 *
 * @author Carlos González Rodríguez - cgonzale@ac.upc.es
 * @date 5/7/2004
 * @ver 1.0
 *
 */
template<class T>
class ACDXRBank
{
private:

    static ACDXQuadReg<T> dummy;

    std::string name; ///< ACDXRBank name
    ACDXQuadReg<T>* bank; ///< ACDXRBank registers
    ACDXRType* types; ///< ACDXRBank registers type
    acd_int* indexes0; ///< ACDXRBank register indexes 0
    acd_int* indexes1; ///< ACDXRBank register indexes 1
    acd_bool* used; ///< bitmap of registers used
    acd_uint count; ///< number of registers within the ACDXRBank


public:
/*
    ACDXRBank() : count(0)
    {
        // default
        bank = NULL;
        types = NULL;
        used = NULL;
        indexes0 = NULL;
        indexes1 = NULL;
        //panic("ACDXRBank","ACDXRBank","Empty ACDXRBank created");
        warning("ACDXRBank","ACDXRBank constructor","Empty ACDXRBank created");
    }
*/
    /**
     * Creates a ACDXRBank of a given size
     *
     * @param size Number of registers
     */
    ACDXRBank(acd_uint size, const std::string& name="");

    ACDXRBank(const ACDXRBank<T>& rb);

    void setSize( acd_uint size );

    /**
     *
     *  ACDXRBank destructor.
     *
     */

    ~ACDXRBank();

    /**
     * Gets the contents of a given register in the ACDXRBank
     *
     * @param pos Register position in the ACDXRBank
     *
     * @return register in position pos
     */
    ACDXQuadReg<T>& get(acd_uint pos) const;

    /**
     * Gets the contents of a given register in the ACDXRBank
     *
     * @param pos Register position in the ACDXRBank
     *
     * @return register in position pos
     *
     * @note exact behaviour than ACDXRBank<T>::get()
     */
    ACDXQuadReg<T>& operator[](acd_uint pos);

    /**
     * Sets a register in the ACDXRBank with new value
     *
     * The register is also marked as used
     *
     * @param pos Register position in the ACDXRBank
     * @param value new contents
     */
    void set(acd_uint pos, const ACDXQuadReg<T>& value);

    /**
     * Mark this register as unused
     *
     * @param pos Register position in the ACDXRBank

     * @note Contents are reset to zero
     */
    void clear(acd_uint pos);

    /**
     * Get the type related with the register (QR_CONSTANT,    QR_GLSTATE,    QR_PARAM_LOCAL,    QR_PARAM_ENV) and
     * related indexes.
     *
     * @param pos Register position in the ACDXRBank
     * @param index0 In case of QR_PARAM_LOCAL or QR_PARAM_ENV is the index of parameter. In
     * case of QR_GLSTATE is the state identifier. Has no sense otherwise.
     * @param index1 In case of QR_GLSTATE is the row of the matrix or 0
     * @return type of register
     * @note Registers with uninitialized type returns QR_UNKNOWN. The register position has to be correct.
     */
    ACDXRType getType(acd_uint pos, acd_int& index0, acd_int& index1) const;

    /**
     * Set the type related with the register and type parameters (indexes)
     *
     * @param pos Register position in the ACDXRBank
     * @param type Type of register.
     * @param index0 In case of QR_PARAM_LOCAL or QR_PARAM_ENV is the index of parameter. In
     * case of QR_GLSTATE is the state identifier. Has no sense otherwise.
     * @param index1 In case of QR_GLSTATE is the row of the matrix or 0
     * @note The register position has to be correct.
     */
    void setType(acd_uint pos, ACDXRType type = QR_UNKNOWN, acd_int index0 = 0, acd_int index1 = 0);

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
    acd_uint getRegister(acd_bool& found, const ACDXQuadReg<T>& value, ACDXRType regType = QR_UNKNOWN, 
                         acd_int index0 = 0, acd_int index1 = 0) const;

    /**
     * Find a free (unused register)
     *
     * @return Register position in the ACDXRBank
     */
    acd_uint findFree() const;

    /**
     * gets ACDXRBank's size
     *
     * @return ACDXRBank's size
     */
    acd_uint size() const;

    /**
     * Tests if a register in the ACDXRBank is being used
     *
     * @parameter pos Register position in the ACDXRBank
     *
     * @return true if the register is being used, false otherwise
     */
    acd_bool usedPosition(acd_uint pos) const;

    void copyContents(const ACDXRBank& rb)
    {
        if (count != rb.count)
            panic("ACDXRBank","copyContents","Target bank has not the same size");

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
        for (acd_uint i = 0; i < count; i++)
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
                        panic("ACDXRBank","print","Unexpected ACDXQuadReg type");
                }
                os << std::endl;
            }
        }
    }

    ACDXRBank<T>& operator=(const ACDXRBank<T>& rb);

    friend std::ostream& operator<<(std::ostream& os, const ACDXRBank& rb)
    {
        rb.print(os);
        return os;
    }
};

template<class T>
ACDXRBank<T>::ACDXRBank(const ACDXRBank<T>& rb)
{
    name = rb.name;
    count = rb.count;
    bank = new ACDXQuadReg<T>[count];
    types = new ACDXRType[count];
    used = new acd_bool[count];
    indexes0 = new acd_int[count];
    indexes1 = new acd_int[count];

    for ( acd_uint i = 0; i < count; i++ )
    {
        bank[i] = rb.bank[i];
        used[i] = rb.used[i];
        types[i] = rb.types[i];
        indexes0[i] = rb.indexes0[i];
        indexes1[i] = rb.indexes1[i];
    }
}


template<class T>
ACDXRBank<T>::ACDXRBank(acd_uint size, const std::string& name) : name(name)
{
    count = size;
    bank = new ACDXQuadReg<T>[count];
    types = new ACDXRType[count];
    indexes0 = new acd_int[count];
    indexes1 = new acd_int[count];
    used = new acd_bool[count];
    memset(used,0,sizeof(bool)*count);
    memset(types,QR_UNKNOWN,sizeof(ACDXRType)*count);
    memset(indexes0,0,sizeof(int)*count);
    memset(indexes1,0,sizeof(int)*count);

}

template<class T>
ACDXRBank<T>::~ACDXRBank()
{
    delete[] bank;
    delete[] types;
    delete[] indexes0;
    delete[] indexes1;
    delete[] used;
}

template<class T>
ACDXRBank<T>& ACDXRBank<T>::operator=(const ACDXRBank<T>& rb)
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
            panic("ACDXRBank<T>", "operator=(const ACDXRBank<T>& rb)", "No registers in the source register bank.");
    )

    count = rb.count;
    bank = new ACDXQuadReg<T>[count];
    used = new acd_bool[count];
    types = new ACDXRType[count];
    indexes0 = new acd_int[count];
    indexes1 = new acd_int[count];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (bank == NULL)
            panic("ACDXRBank", "operator=(const ACDXRBank<T>& rb)", "Error allocating bank array.");
        if (used == NULL)
            panic("ACDXRBank", "operator=(const ACDXRBank<T>& rb)", "Error allocating used array.");
        if (types == NULL)
            panic("ACDXRBank", "operator=(const ACDXRBank<T>& rb)", "Error allocating types array.");
        if (indexes0 == NULL)
            panic("ACDXRBank", "operator=(const ACDXRBank<T>& rb)", "Error allocating indexes0 array.");
        if (indexes1 == NULL)
            panic("ACDXRBank", "operator=(const ACDXRBank<T>& rb)", "Error allocating indexes1 array.");
    )

    for ( acd_uint i = 0; i < count; i++ )
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
acd_uint ACDXRBank<T>::size() const
{
    return count;
}

template<class T>
void ACDXRBank<T>::setSize( acd_uint size_ )
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

    bank = new ACDXQuadReg<T>[count];
    used = new acd_bool[count];
    types = new ACDXRType[count];
    indexes0 = new acd_int[count];
    indexes1 = new acd_int[count];
    memset(used,0,sizeof(bool)*count);
    memset(types,QR_UNKNOWN,sizeof(ACDXRType)*count);
    memset(indexes0,0,sizeof(int)*count);
    memset(indexes1,0,sizeof(int)*count);
}

template<class T>
ACDXQuadReg<T>& ACDXRBank<T>::get(acd_uint pos) const
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("ACDXRBank","get","Invalid position out of bounds");

    return bank[pos];
}

template<class T>
ACDXQuadReg<T>& ACDXRBank<T>::operator[](acd_uint pos)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("ACDXRBank","operator[]","Invalid position out of bounds");

    used[pos] = true;
    return bank[pos];
}


template<class T>
void ACDXRBank<T>::set(acd_uint pos, const ACDXQuadReg<T>& value)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("ACDXRBank","set","Invalid position out of bounds");

    used[pos] = true;
    bank[pos] = value;
}

template<class T>
void ACDXRBank<T>::clear(acd_uint pos)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("ACDXRBank","clear","Invalid position out of bounds");

    used[pos] = false;
    types[pos] = QR_UNKNOWN;
    ACDXQuadReg<T> q; /* init to zero */
    bank[pos] = q;
}

template<class T>
ACDXRType ACDXRBank<T>::getType(acd_uint pos,int& index0, int& index1) const
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("ACDXRBank","getType","Invalid position out of bounds");

    index0 = indexes0[pos];
    index1 = indexes1[pos];
    return types[pos];
}

template<class T>
void ACDXRBank<T>::setType(acd_uint pos, ACDXRType type, int index0, int index1)
{
    /*  Check index to the register bank.  */
    if (pos >= count)
        panic("ACDXRBank","setType","Invalid position out of bounds");

    types[pos] = type;
    indexes0[pos] = index0;
    indexes1[pos] = index1;
}

template<class T>
acd_uint ACDXRBank<T>::getRegister(bool& found, const ACDXQuadReg<T>& value, ACDXRType regType, int index0, int index1) const
{
    acd_uint i;
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
                panic("ACDXRBank","getRegister()","Unexpected ACDXQuadReg type");
                return 0;
            }
        }

    }
    return findFree();
}


template<class T>
bool ACDXRBank<T>::usedPosition(acd_uint pos) const
{
    if (pos >= count)
        panic("ACDXRBank","usedPosition","Invalid position out of bounds");

    return used[pos];
}


template<class T>
acd_uint ACDXRBank<T>::findFree() const
{
    acd_uint i;
    for ( i = 0; i < count; i++ )
        if ( !used[i] )
            break;
    return i;
}



template<class T>
class GPUBank : public ACDXRBank<T>
{
private:
    int readPorts;
public:

    GPUBank(acd_uint size, std::string name) : ACDXRBank<T>(size,name), readPorts(0) {}

    GPUBank(acd_uint size, acd_uint readPorts = 0, std::string name="") :
        ACDXRBank<T>(size,name), readPorts(readPorts)
        {}

    GPUBank(const ACDXRBank<T>& rb, acd_uint readPorts = 0, std::string name="") :
        ACDXRBank<T>(rb.size(),name), readPorts(readPorts)
        {
            copyContents(rb);
        }

    int getReadPorts() const { return readPorts; }

    void setReadPorts(int nReads) { readPorts = nReads; }

    void print(std::ostream& os) const
    {
        ACDXRBank<T>::print(os);
        os << "Read ports: " << readPorts << std::endl;
    }
};


} // namespace acdlib

#endif // ACDX_RBANK_H

