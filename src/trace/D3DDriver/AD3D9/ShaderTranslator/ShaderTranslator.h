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
	NativeShader *translate(IR *ir, D3DCMPFUNC alpha_test = D3DCMP_ALWAYS, bool fogEnable = false);
	u32bit buildIR(const DWORD *source, IR* &ir);
    static ShaderTranslator &get_instance();
private:
    ShaderTranslator();
    ~ShaderTranslator();
    std::string printInstructionsDisassembled(std::vector<gpu3d::ShaderInstruction *> instr);
    std::string printInstructionsBinary(std::vector<gpu3d::ShaderInstruction *> instr);
    IRTranslator *translator;
    IRPrinter *printer;
    IRDisassembler *disassembler;
    IRBuilder *builder;
};



#endif
