
#include "ShaderTranslator.h"

#include <sstream>

using namespace std;
using namespace gpu3d;

NativeShader *ShaderTranslator::translate(const DWORD *source, D3DCMPFUNC alpha_test) {

    //static int progID = 0;

    string shaderTokenDump;
    string shaderDisassembly;
    
    IR *ir = builder->build(source);

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

	translator->setAlphaTest(alpha_test);
    ir->traverse(translator, false);

    list<ShaderInstruction *> &instructions = translator->get_instructions();

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
        
    delete ir;

    cout << "ATTILA Assembly Code. " << endl;
    cout << translated->debug_assembler << endl;
    cout << "ATTILA Binary Code. " << endl;
    cout << translated->debug_binary << endl;
    cout << "---------------------" << endl;

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



string ShaderTranslator::printInstructionsDisassembled(std::list<gpu3d::ShaderInstruction *> instructions) {
    std::stringstream ss;
    list<ShaderInstruction *>::iterator itSI;
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

string ShaderTranslator::printInstructionsBinary(std::list<gpu3d::ShaderInstruction *> instructions) 
{
    u8bit *memory = new u8bit[instructions.size() * 16];
    
    std::list<gpu3d::ShaderInstruction *>::iterator itSI;
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
