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

#ifndef CLUSTER_BANK_ADAPTER_H
    #define CLUSTER_BANK_ADAPTER_H

#include "ACDXRBank.h"
#include "ACDXConstantBindingImp.h"
#include <list>

namespace acdlib
{

class ClusterBankAdapter
{
private:

    ACDXRBank<float>* _clusterBank;

    const ACDXConstantBindingImp* searchLocal(acd_uint pos, const ACDXConstantBindingList* inputConstantBindings) const;
    const ACDXConstantBindingImp* searchEnvironment(acd_uint pos, const ACDXConstantBindingList* inputConstantBindings) const;

public:

    ClusterBankAdapter(ACDXRBank<float>* clusterBank);

    ACDXConstantBindingList* getFinalConstantBindings(const ACDXConstantBindingList* inputConstantBindings) const;

};

} // namespace acdlib

#endif // CLUSTER_BANK_ADAPTER_H
