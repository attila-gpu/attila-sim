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

#include "ShProgramInfo.h"
#include <sstream>
#include <fstream>
#include "support.h"
#include <windows.h>

using namespace std;

void ShProgramInfo::setSource(const string& source)
{
    this->source = source;
    nInstr = 0;
    nTexInstr = 0;    
    
    stringstream ss(source);
    string op;

    char dummy[4096];
    
    while ( !ss.eof() )
    {
        ss >> op;

        if ( op.at(0) == '#' ) // skip comment line
            ss.getline(dummy, sizeof(dummy));
        
        if ( op == "ABS" || op == "FLR" || op == "FRC" || op == "LIT" || op == "MOV" ||
             op == "EX2" || op == "EXP" || op == "LG2" || op == "LOG" || op == "RCP" ||
             op == "RSQ" || op == "POW" || op == "ADD" || op == "DP3" || op == "DP4" ||
             op == "DPH" || op == "DTS" || op == "MAX" || op == "MIN" || op == "MUL" ||
             op == "SGE" || op == "SLT" || op == "SUB" || op == "XPD" || op == "MAD" ||
             // fragment programs specific instructions
             op == "ABS_SAT" || op == "FLR_SAT" || op == "FRC_SAT" || op == "LIT_SAT" ||
             op == "MOV_SAT" || op == "COS" || op == "CMP" || op == "CMP_SAT" || 
             op == "LRP" || op == "LRP_SAT" || op == "MAD_SAT" || op == "SWZ" ||
             op == "SWZ_SAT" || op == "KIL" )
        {
            nInstr++;
        }
        else if ( op == "TEX" || op == "TXP" || op == "TXB" ||
                  op == "TEX_SAT" || op == "TXP_SAT" || op == "TXB_SAT" )
        {
            nInstr++;
            nTexInstr++;
        }
    }
}

ShProgramInfo::ShProgramInfo(unsigned int name, unsigned int type) : 
   nInstr(0), nTexInstr(0), type(type), name(name)
{}


unsigned int ShProgramInfo::getName() const
{
    return name;
}

int ShProgramInfo::countInstructions() const
{
    return nInstr;
}

int ShProgramInfo::countTextureLoads() const
{
    return nTexInstr;
}

float ShProgramInfo::textureLoadsRatio() const
{
    return ((float)nTexInstr) / nInstr;
}

const string& ShProgramInfo::getSource() const
{
    return source;
}

unsigned int ShProgramInfo::getType() const
{
    return type;
}

ShProgramManager::ShProgramManager()
{}

ShProgramManager& ShProgramManager::instance()
{
    static ShProgramManager spm; ///< created just once (first time instance is called)
    return spm;
}

ShProgramInfo* ShProgramManager::findProgram(unsigned int name) const
{
    map<unsigned int, ShProgramInfo*>::const_iterator it = progs.find(name);
    if ( it != progs.end() )
        return it->second;
    return 0;
}

bool ShProgramManager::bindProgram(unsigned int name, unsigned int type)
{
    if ( type != GL_VERTEX_PROGRAM_ARB && type != GL_FRAGMENT_PROGRAM_ARB )
        panic("ShProgramManager", "boundProgram", "Shader type is unknown");

    // type is OK

    if ( name == 0 ) // unbound
    {
        
        if ( type == GL_VERTEX_PROGRAM_ARB )
            vsh = 0;
        else 
            fsh = 0;
        return false;
    }

    ShProgramInfo* shPI = findProgram(name);
    
    bool isNew = false;
    if ( !shPI ) // new shader program
    {
        shPI = new ShProgramInfo(name, type);
        progs.insert(make_pair(name, shPI));
        progOrder.push_back(name); // maintain program order specification
        isNew = true;
    }

    if ( type == GL_VERTEX_PROGRAM_ARB )
        vsh = shPI;
    else
        fsh = shPI;

    return isNew;
}

ShProgramInfo* ShProgramManager::getCurrent(unsigned int type) const
{
    if ( type == GL_VERTEX_PROGRAM_ARB )
        return vsh;
    else if ( type == GL_FRAGMENT_PROGRAM_ARB )
        return fsh;
    else
        panic("ShProgramManager", "getCurrent", "Shader type is unknown");
    return 0;
}

float ShProgramManager::getAverageInstructions(unsigned int type) const
{
    int instrCount = 0;
    int totalShaders = 0;

    if ( type != GL_VERTEX_PROGRAM_ARB && type != GL_FRAGMENT_PROGRAM_ARB )
        type = 0;

    map<unsigned int, ShProgramInfo*>::const_iterator it = progs.begin();
    for ( ; it != progs.end(); it++ )
    {
        if ( type == 0 || type == it->second->getType() )
        {
            totalShaders++;
            instrCount += it->second->countInstructions();
        }
    }

    return ((float)instrCount)/totalShaders;
}

float ShProgramManager::getAverageTextureLoads(unsigned int type) const
{
    float cumulativeTexRatios = 0;
    int totalShaders = 0;

    if ( type != GL_VERTEX_PROGRAM_ARB && type != GL_FRAGMENT_PROGRAM_ARB )
        type = 0;

    map<unsigned int, ShProgramInfo*>::const_iterator it = progs.begin();
    for ( ; it != progs.end(); it++ )
    {
        if ( type == 0 || type == it->second->getType() )
        {
            totalShaders++;
            cumulativeTexRatios += it->second->textureLoadsRatio();
        }
    }

    return cumulativeTexRatios / totalShaders;
}

bool ShProgramManager::writeInfo(std::string file)
{
    ofstream f(file.c_str());
    
    if ( !f ) { return false; }
        
    f << "PROGRAM TYPE;Program name;Instruction Count;Texture Load Count;Ratio TexCount/InstrCount\n";//Source code\n";

    // Print the programs sorted based on program bind order
    list<unsigned int>::const_iterator it = progOrder.begin();
    for ( ; it != progOrder.end(); it++ )
    {
        ShProgramInfo* pi = findProgram(*it);
        if ( pi->getType() == GL_VERTEX_PROGRAM_ARB )
            f << "VERTEX;";
        else if ( pi->getType() == GL_FRAGMENT_PROGRAM_ARB )
            f << "FRAGMENT;";
        else
            f << "?;"; // unknown program type
        f << pi->getName() << ";";
        f << pi->countInstructions() << ";";
        f << pi->countTextureLoads() << ";";
        f << pi->textureLoadsRatio() << "\n";
    }    
    return true;
}
