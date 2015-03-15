#ifndef ____SHADER_TRANSLATOR
#define ____SHADER_TRANSLATOR

#include "Types.h"
#include "IR.h"
#include "IRTranslator.h"
#include "IRPrinter.h"
#include "IRDisassembler.h"
#include "IRBuilder.h"


class ShaderTranslator  {
public:
	NativeShader *translate(const DWORD *source, D3DCMPFUNC alpha_test = D3DCMP_ALWAYS);
    static ShaderTranslator &get_instance();
private:
    ShaderTranslator();
    ~ShaderTranslator();
    std::string printInstructionsDisassembled(std::list<gpu3d::ShaderInstruction *> instr);
    std::string printInstructionsBinary(std::list<gpu3d::ShaderInstruction *> instr);
    IRTranslator *translator;
    IRPrinter *printer;
    IRDisassembler *disassembler;
    IRBuilder *builder;
};



#endif
