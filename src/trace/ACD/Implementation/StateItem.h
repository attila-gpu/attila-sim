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

#ifndef STATE_ITEM
    #define STATE_ITEM

#include "ACDTypes.h"

namespace acdlib
{

template<class T>
class StateItem
{
public:

    inline StateItem();

    inline StateItem(T initialValue);

    inline void restart();

    inline acd_bool changed() const;

    inline operator const T&() const;

    inline StateItem<T>& operator=(const T& value);

    inline const T& initial() const;

private:

    T _initialValue;
    T _value;

};

} // namespace acdib

template<class T>
acdlib::StateItem<T>::StateItem() {}

template<class T>
acdlib::StateItem<T>::StateItem(T initialValue) : _initialValue(initialValue), _value(_initialValue) {}

template<class T>
void acdlib::StateItem<T>::restart() { _initialValue = _value; }

template<class T>
acdlib::acd_bool acdlib::StateItem<T>::changed()  const { return (!(_initialValue == _value)); }

template<class T>
acdlib::StateItem<T>::operator const T&() const { return _value; }

template<class T>
acdlib::StateItem<T> & acdlib::StateItem<T>::operator=(const T& value) { _value = value; return *this; }

template<class T>
const T& acdlib::StateItem<T>::initial() const { return _initialValue; }

#endif // STATE_ITEM
