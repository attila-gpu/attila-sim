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

#include "ClusterBankAdapter.h"
#include "ImplementedConstantBindingFunctions.h"
#include "InternalConstantBinding.h"

using namespace std;
using namespace acdlib;


ClusterBankAdapter::ClusterBankAdapter(ACDXRBank<float>* clusterBank)
: _clusterBank(clusterBank)
{
}




const ACDXConstantBindingImp* ClusterBankAdapter::searchLocal(acd_uint pos, const ACDXConstantBindingList* inputConstantBindings) const
{
    list<ACDXConstantBinding*>::const_iterator iter = inputConstantBindings->begin();

    while ( iter!= inputConstantBindings->end() )
    {
        ACDXConstantBindingImp* cbi = static_cast<ACDXConstantBindingImp*>(*iter);
        if (cbi->getTarget() == ACDX_BINDING_TARGET_LOCAL && cbi->getConstantIndex() == pos)
        {
            return cbi;
        }
        iter++;
    }

    return 0;
}

const ACDXConstantBindingImp* ClusterBankAdapter::searchEnvironment(acd_uint pos, const ACDXConstantBindingList* inputConstantBindings) const
{
    list<ACDXConstantBinding*>::const_iterator iter = inputConstantBindings->begin();

    while ( iter!= inputConstantBindings->end() )
    {
        ACDXConstantBindingImp* cbi = static_cast<ACDXConstantBindingImp*>(*iter);
        if (cbi->getTarget() == ACDX_BINDING_TARGET_ENVIRONMENT && cbi->getConstantIndex() == pos)
        {
            return cbi;
        }
        iter++;
    }

    return 0;
}

ACDXConstantBindingList* ClusterBankAdapter::getFinalConstantBindings(const ACDXConstantBindingList* inputConstantBindings) const
{
    ACDXConstantBindingList* retList = new ACDXConstantBindingList;

    for ( acd_uint i = 0; i < _clusterBank->size(); i++ )
    {
        acd_int pos, pos2;
        ACDXRType regType = _clusterBank->getType(i,pos,pos2);

        switch ( regType )
        {
            case QR_CONSTANT:
                {
                    ACDXFloatVector4 constant;

                    constant[0] = (*_clusterBank)[i][0];
                    constant[1] = (*_clusterBank)[i][1];
                    constant[2] = (*_clusterBank)[i][2];
                    constant[3] = (*_clusterBank)[i][3];

                    InternalConstantBinding* icb = new InternalConstantBinding(0, false, i, new ACDXDirectSrcCopyBindFunction, constant);
                    
                    retList->push_back(icb);
                }
                break;

            case QR_PARAM_LOCAL:
                {
                    const ACDXConstantBindingImp* local = searchLocal(pos, inputConstantBindings);
                    
                    if (local) 
                    {
                        // There´s a constant binding for the program local parameter 
                        // referenced in the program. 
                        // Forward the constant binding properties.
                        ACDXConstantBindingImp* cbi = new ACDXConstantBindingImp(ACDX_BINDING_TARGET_FINAL, i, local->getStateIds(), local->getBindingFunction(), local->getDirectSource());                    
                        retList->push_back(cbi);
                    }
                    else
                    {
                        // There isn´t any constant binding for the program local
                        // Bind a "zero" vector for the local parameter

                        InternalConstantBinding* icb = new InternalConstantBinding(0, false, i, new ACDXDirectSrcCopyBindFunction, ACDXFloatVector4(acd_float(0)));
                        retList->push_back(icb);
                    }
                    
                }
                break;

            case QR_PARAM_ENV:
                {
                    const ACDXConstantBindingImp* env = searchEnvironment(pos, inputConstantBindings);

                    if (env) 
                    {
                        // There´s a constant binding for the program environment parameter 
                        // referenced in the program. 
                        // Forward the constant binding properties.
                          ACDXConstantBindingImp* cbi = new ACDXConstantBindingImp(ACDX_BINDING_TARGET_FINAL, i, env->getStateIds(), env->getBindingFunction(), env->getDirectSource());
                        retList->push_back(cbi);
                    }
                    else
                    {
                        // There isn´t any constant binding for the program environment
                        // Bind a "zero" vector for the environment parameter
                        InternalConstantBinding* icb = new InternalConstantBinding(0, false, i, new ACDXDirectSrcCopyBindFunction, ACDXFloatVector4(acd_float(0)));
                        retList->push_back(icb);
                    }
                }    
                break;

            case QR_GLSTATE:
                {
                    if ( pos < BASE_STATE_MATRIX )
                    {    
                        // Vector state
                        InternalConstantBinding* icb = new InternalConstantBinding(pos, true, i, new ACDXStateCopyBindFunction);
                        retList->push_back(icb);
                    }
                    else
                    {
                        // Matrix state
                        InternalConstantBinding* icb = new InternalConstantBinding(pos, true, i, new CopyMatrixRowFunction(pos2));
                        retList->push_back(icb);
                    }
                }
                break;

            case QR_UNKNOWN: break;
            default:
                panic("ClusterBankAdapter","getFinalConstantBindings","Unexpected ClusterBank register type");
        }
    }
    return retList;
}
