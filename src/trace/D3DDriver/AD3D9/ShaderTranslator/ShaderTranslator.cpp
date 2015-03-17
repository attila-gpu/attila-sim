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


#include "ShaderTranslator.h"

#include <sstream>
#include <stdio.h>

using namespace std;
using namespace gpu3d;

u32bit ShaderTranslator::buildIR(const DWORD *source, IR *&ir)
{
    u32bit numTokens;
        
    ir = builder->build(source, numTokens);

    /*string shaderTokenDump;
    string shaderDisassembly;

    stringstream ss;
    printer->setOut(&ss);
    ir->traverse(printer, true);
    shaderTokenDump = ss.str();
    
    cout << "---------------------" << endl;     
    cout << "D3D9 Tokens. " << endl;
    cout << shaderTokenDump << endl;

    ss.clear();
    ss.str("");
    disassembler->setOut(&ss);
    ir->traverse(disassembler, true);
    shaderDisassembly = ss.str();

    cout << "D3D9 Disassembled program. " << endl;
    cout << shaderDisassembly << endl << endl;
    cout << "---------------------" << endl;*/

    return numTokens;
}

NativeShader *ShaderTranslator::translate(IR *ir, D3DCMPFUNC alpha_test, bool pixelFog)
{
    //static int progID = 0;

    string shaderTokenDump;
    string shaderDisassembly;

    stringstream ss;
    printer->setOut(&ss);
    ir->traverse(printer, true);
    shaderTokenDump = ss.str();
    
    //cout << "---------------------" << endl;     
    //cout << "D3D9 Tokens. " << endl;
    //cout << shaderTokenDump << endl;

    ss.clear();
    ss.str("");
    disassembler->setOut(&ss);
    ir->traverse(disassembler, true);
    shaderDisassembly = ss.str();

    //cout << "D3D9 Disassembled program. " << endl;
    //cout << shaderDisassembly << endl << endl;

	translator->setAlphaTest(alpha_test);
	translator->setFogEnabled(pixelFog);
    ir->traverse(translator, false);

    vector<ShaderInstruction *> &instructions = translator->get_instructions();

    NativeShader *translated = translator->build_native_shader();
    translated->debug_binary = printInstructionsBinary(instructions);
    translated->debug_assembler = printInstructionsDisassembled(instructions);
    translated->debug_ir = shaderTokenDump;
    translated->debug_disassembler = shaderDisassembly;
    translated->untranslatedControlFlow = translator->getUntranslatedControlFlow();
    translated->untranslatedInstructions = translator->getUntranslatedInstructions();

    if (translated->untranslatedControlFlow)
        cout << "TRANSLATION ERROR : Unstranslated control flow" << endl;
    
    if (translated->untranslatedInstructions)
        cout << "TRANSLATION ERROR : Number of untranslated instructions is " << translated->untranslatedInstructions << endl;
        
    //cout << "ATTILA Assembly Code. " << endl;
    //cout << translated->debug_assembler << endl;
    //cout << "ATTILA Binary Code. " << endl;
    //cout << translated->debug_binary << endl;
    //cout << "---------------------" << endl;

    return translated;
}

ShaderTranslator::~ShaderTranslator() {
    delete builder;
    delete printer;
    delete disassembler;
    delete translator;
}

ShaderTranslator &ShaderTranslator::get_instance() {
    static ShaderTranslator lonely;
    return lonely;
}

ShaderTranslator::ShaderTranslator()
{
    builder = new IRBuilder();
    printer = new IRPrinter();
    disassembler = new IRDisassembler();
    translator = new IRTranslator();
}



string ShaderTranslator::printInstructionsDisassembled(std::vector<gpu3d::ShaderInstruction *> instructions) {
    std::stringstream ss;
    vector<ShaderInstruction *>::iterator itSI;
    char auxs[80];
    bool programEnd = false;
    for(itSI = instructions.begin(); (itSI != instructions.end()) && !programEnd; itSI ++)
    {
        (*itSI)->disassemble(auxs);
        ss << auxs << endl;
        programEnd = (*itSI)->getEndFlag();
    }
    return ss.str();
}

string ShaderTranslator::printInstructionsBinary(std::vector<gpu3d::ShaderInstruction *> instructions) 
{
    u8bit *memory = new u8bit[instructions.size() * 16];
    
    std::vector<gpu3d::ShaderInstruction *>::iterator itSI;
    u32bit i = 0;
    bool programEnd = false;
    for(itSI = instructions.begin(); (itSI != instructions.end()) && !programEnd; itSI ++)
    {
        (*itSI)->getCode(&memory[i * 16]);
        i++;
        programEnd = (*itSI)->getEndFlag();
    }

    stringstream ss;
    u32bit count_u32bit = i * 16;
    for(u32bit i = 0; i < count_u32bit; i++)
    {
        char aux[20];
        sprintf(aux, "%02x ", memory[i]);
        ss << aux;
        
        if ((i % 16) == 15)
            ss << endl;
    }

    delete []memory;

    return ss.str();
}
