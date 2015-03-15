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

#include "ARBProgramObject.h"
#include "ARBProgramTarget.h"
#include "ACDShaderProgram.h" // Include the ACD interface for shader programs

#include "support.h"

//#define ENABLE_GLOBAL_PROFILER
#include "GlobalProfiler.h"

#include <cstring>
#include "glext.h"

using namespace agl;
using namespace std;
using namespace acdlib;

ARBProgramObject::ARBProgramObject(GLenum name, GLenum targetName) : BaseObject(name, targetName),
    _shader(0), _arbCompiledProgram(0), _format(0), _locals(MaxLocalRegisters), _sourceCompiled(false),
    _acddev(0)
{}

void ARBProgramObject::setSource(const string& arbCode, GLenum format)
{
    if ( arbCode == _source ) // ignore updates with the same code
        return ;

    _source = arbCode;
    _sourceCompiled = false; // New source requiring compilation
}

const string& ARBProgramObject::getSource() const // new version
{
    return _source;
}


void ARBProgramObject::attachDevice(ACDDevice* device)
{
    _acddev = device;
}


ACDShaderProgram* ARBProgramObject::compileAndResolve(acdlib::ACDXFixedPipelineState* fpState)
{
    // Compilation process
    if ( !_sourceCompiled )
    {
        ACDXDestroyCompiledProgram(_arbCompiledProgram); // Discard previous compilation
        GLOBALPROFILER_ENTERREGION("compile ARB", "", "")
        _arbCompiledProgram = ACDXCompileProgram(_source); // Generate the new compilation
        GLOBALPROFILER_EXITREGION()
        _sourceCompiled = true; // Source and binary are synchronized
    }

    // Create a shader program if it does not exist yet
    if ( !_shader ) {
        if ( !_acddev )
            panic("ARBProgramObject", "compileAndResolve", "No ACDDevice attached. ACDShaderProgram object cannot be created");
        _shader = _acddev->createShaderProgram();
    }

    
    std::vector<ACDX_STORED_FP_ITEM_ID> emptyList;
    ACDXConstantBindingList cbList;
    
    // Resolve local params
    vector<acd_uint> params = _locals.getModified();
    ACDXDirectSrcCopyBindFunction directCopyFunction;
    for ( acd_uint i = 0; i < params.size(); ++i ) {
        ACDXFloatVector4 directSource;
        _locals.get(params[i], directSource[0], directSource[1], directSource[2], directSource[3]);        
        cbList.push_back( ACDXCreateConstantBinding( ACDX_BINDING_TARGET_LOCAL, 
                                       params[i], 
                                       emptyList,
                                       &directCopyFunction, 
                                       directSource) );
    }

    // Resolve environment params
    ARBRegisterBank& envs = static_cast<ARBProgramTarget&>(getTarget()).getEnv();
    params = envs.getModified();
    for ( acd_uint i = 0; i < params.size(); ++i ) {
        ACDFloatVector4 directSource;
        envs.get(params[i], directSource[0], directSource[1], directSource[2], directSource[3]);
        cbList.push_back( ACDXCreateConstantBinding( ACDX_BINDING_TARGET_ENVIRONMENT, 
                                       params[i], 
                                       emptyList,
                                       &directCopyFunction, 
                                       directSource) );
    }

    // Resolve process
    ACDXResolveProgram(fpState, _arbCompiledProgram, &cbList, _shader);

    ACDXConstantBindingList::iterator it;
    it = cbList.begin();
    while (it != cbList.end())
    {
        ACDXDestroyConstantBinding(*it);
        it++;
    }
    cbList.clear();
    
    // Return result shader
    return _shader;
}


ARBProgramObject::~ARBProgramObject()
{
    if ( _acddev ) {
        if ( _shader )
            _acddev->destroy(_shader);
    }
    if ( _arbCompiledProgram )
        ACDXDestroyCompiledProgram(_arbCompiledProgram);
}

const char* ARBProgramObject::getStringID() const
{
    GLenum tname = getTargetName();
    
    if ( tname == GL_VERTEX_PROGRAM_ARB )
        return "VERTEX_PROGRAM";
    else if ( tname == GL_FRAGMENT_PROGRAM_ARB )
        return "FRAGMENT_PROGRAM";
    else
        return BaseObject::getStringID();
}

ARBRegisterBank& ARBProgramObject::getLocals()
{
    return _locals;
}

const ARBRegisterBank& ARBProgramObject::getLocals() const
{
    return _locals;
}
