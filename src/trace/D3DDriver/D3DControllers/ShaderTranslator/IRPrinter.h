/**
 * Printer visitor for IRNodes
 * @author Chema Solís
 */

#ifndef __IR_PRINTER
#define __IR_PRINTER

#include "IR.h"

#include <map>

/**
 * Prints IRNodes for debugging purposes.
 **/
class IRPrinter : public IRVisitor
{
public:
    void begin();
    void visit(IRNode *n);
    void visit(CommentIRNode *n);
    void visit(CommentDataIRNode *n);
    void visit(VersionIRNode *n);
    void visit(InstructionIRNode *n);
    void visit(DeclarationIRNode *n);
    void visit(DefinitionIRNode *n);
    void visit(ParameterIRNode *n);
    void visit(SourceParameterIRNode *n);
    void visit(DestinationParameterIRNode *n);
    void visit(RelativeAddressingIRNode *n);
    void visit(BoolIRNode *n);
    void visit(FloatIRNode *n);
    void visit(IntegerIRNode *n);
    void visit(SemanticIRNode *n);
    void visit(SamplerInfoIRNode *n);

    void visit(EndIRNode *n);

    void setOut(std::ostream *o) { out = o; }
    void end() { out->flush(); }
    IRPrinter(std::ostream *o = &std::cout);
private:
    ShaderType type;
    DWORD version;
    void configure(VersionIRNode *n);
    std::map<D3DSHADER_PARAM_REGISTER_TYPE, std::string> paramRegisterTypeNames;
    std::map<D3DSHADER_INSTRUCTION_OPCODE_TYPE, std::string> opcodeNames;
    std::map<D3DSHADER_PARAM_SRCMOD_TYPE, std::string> paramSourceModNames;
    std::map<D3DSHADER_COMPARISON, std::string> comparisonNames;
    std::map<D3DSAMPLER_TEXTURE_TYPE, std::string> textureTypeNames;
    std::map<DWORD, std::string> usageNames;
    char components[4];
    std::string getSwizzleStr(SourceParameterIRNode *n);
    std::string getWriteMaskStr(DestinationParameterIRNode *n);
    std::string getDestModifierStr(DestinationParameterIRNode *n);
    std::string getShiftScaleStr(DestinationParameterIRNode *n);
    std::string getControlStr(InstructionIRNode *n);
        
    std::ostream *out;
    
    bool preAppendComma;
};

#endif
