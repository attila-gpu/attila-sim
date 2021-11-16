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

#ifndef ACDX_IRNODE_H
    #define ACDX_IRNODE_H
#include <list>
#include <string>

namespace acdlib
{

namespace GenerationCode
{

/* Forwarding of traverser base class */

class ACDXIRTraverser;

/**
 *  Abstract base class for the Intermediate Representation(IR) tree
 */

class ACDXIRNode
{
protected:
    unsigned int line;

public:
    ACDXIRNode();
    ACDXIRNode(unsigned int line);

    void setLine(unsigned int line_) { this->line = line_; }
    
    unsigned int getLine() const { return line; }

    virtual void traverse(ACDXIRTraverser*) = 0;
    virtual ~ACDXIRNode();
};

/**
 *  The class for the tree root. It manages a list of options
 *  specified in the program and the list of statements (declarations
 *  and instructions).
 */

class IROption;
class IRStatement;

class IRProgram: public ACDXIRNode
{
private:
    std::string headerString;
    std::list<IROption *>*    programOptions;
    std::list<IRStatement *>* programStatements;

public:
    IRProgram();
    IRProgram(const std::string& headerstring);

    void setHeaderString(const std::string& headerString_)   {   this->headerString = headerString_;  }
    void addOptionList(std::list<IROption *>* programOptions_)   {   this->programOptions = programOptions_;  }
    void addStatementList(std::list<IRStatement *>* programStatements_)  {   this->programStatements = programStatements_;    }

    std::string getHeaderString() const { return headerString; }

    void dump(std::ostream& os);
    
    friend std::ostream& operator<<(std::ostream& os, IRProgram *ir)
    {
        ir->dump(os);
        return os;
    }

    virtual void traverse(ACDXIRTraverser*);
    virtual ~IRProgram();
};

/**
 *  The class that represents a program option. The option
 *  can have a associated value. At moment this value is a
 *  positive integer number.
 */

class IROption: public ACDXIRNode
{
private:
    std::string optionString;

public:
    IROption(unsigned int line, const std::string& optionString);
    
    std::string getOptionString() const { return optionString; }
    
    virtual void traverse(ACDXIRTraverser*)= 0;
};

class VP1IROption: public IROption
{
public:
    VP1IROption(unsigned int line, const std::string& optionString);
    
    virtual void traverse(ACDXIRTraverser*);
};

class FP1IROption: public IROption
{
public:
    FP1IROption(unsigned int line, const std::string& optionString);
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRStatement: public ACDXIRNode
{
public:
    IRStatement(unsigned int line);
    
    virtual void traverse(ACDXIRTraverser*)=0;
    virtual ~IRStatement();
};

class IRSrcOperand;
class IRDstOperand;

class IRInstruction: public IRStatement
{
private:
    std::string                 opCode;
    bool isFixedPoint;

protected:
    IRSrcOperand*               source0;
    IRSrcOperand*               source1;
    IRSrcOperand*               source2;
    IRDstOperand*               dest;

public:
    IRInstruction(unsigned int line, const std::string& opcode);

    void setDest(IRDstOperand *dst)     {   dest = dst;     }
    void setSource0(IRSrcOperand *src)  {   source0 = src;  }
    void setSource1(IRSrcOperand *src)  {   source1 = src;  }
    void setSource2(IRSrcOperand *src)  {   source2 = src;  }
    
    std::string getOpcode() const       {   return opCode;  }
    
    void setIsFixedPoint() {  isFixedPoint = true; }

    bool getIsFixedPoint() const { return isFixedPoint; }

    virtual void traverse(ACDXIRTraverser*);
    virtual ~IRInstruction();
};

class IRSampleInstruction: public IRInstruction
{
private:
    unsigned int textureImageUnit;
    std::string glTextureTarget;

public:
    IRSampleInstruction(unsigned int line, const std::string& opcode);
    
    void setTextureImageUnit(unsigned int textureImageUnit_)   {  this->textureImageUnit = textureImageUnit_; }
    void setTextureTarget(const std::string& glTextureTarget_) {  this->glTextureTarget = glTextureTarget_;   }
    
    unsigned int getTextureImageUnit() const {  return textureImageUnit;  }
    std::string getGLTextureTarget() const   {  return glTextureTarget;   }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRKillInstruction: public IRInstruction
{
private:
    bool isKillSampleInstr;
    unsigned int killSample;

public:
    IRKillInstruction(unsigned int line, const std::string& opcode);
    
    void setKillSample(unsigned int _killSample)   {  killSample = _killSample; }

    void setIsKillSampleInstr() {  isKillSampleInstr = true; }

    bool getIsKillSampleInstr() const { return isKillSampleInstr; }
    
    unsigned int getKillSample() const { return killSample; }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRZExportInstruction: public IRInstruction
{
private:
    bool isExpSampleInstr;
    unsigned int exportSample;

public:
    IRZExportInstruction(unsigned int line, const std::string& opcode);
    
    void setExportSample(unsigned int _exportSample)   {  exportSample = _exportSample; }

    void setIsExpSampleInstr() {  isExpSampleInstr = true; }

    bool getIsExpSampleInstr() const { return isExpSampleInstr; }
    
    unsigned int getExportSample() const { return exportSample; }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRCHSInstruction: public IRInstruction
{
public:
    IRCHSInstruction(unsigned int line, const std::string& opcode);
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRSwizzleComponents;

class IRSwizzleInstruction: public IRInstruction
{
private:
    IRSwizzleComponents* swizzleComps;

public:
    IRSwizzleInstruction(unsigned int line, const std::string& opcode);
    
    void setSwizzleComponents(IRSwizzleComponents* swizzleComps_)   { this->swizzleComps = swizzleComps_; }
    
    IRSwizzleComponents* getSwizzleComponents() const { return swizzleComps;  }
    
    virtual void traverse(ACDXIRTraverser*);    
    virtual ~IRSwizzleInstruction();
};

class IRSwizzleComponents: public ACDXIRNode
{
public:
    enum swzComponent {ZERO, ONE, X, Y, Z, W};

private:    
    swzComponent swizzleComp[4];
    bool swizzleNegFlag[4];

public:
    IRSwizzleComponents();
    
    void setComponent(unsigned int num, swzComponent comp, bool neg)
    { 
        swizzleComp[num] = comp;
        swizzleNegFlag[num] = neg;
    }
    
    swzComponent getComponent(unsigned int num) const { return swizzleComp[num];    }
    bool getNegFlag(unsigned int num) const           { return swizzleNegFlag[num]; }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRDstOperand: public ACDXIRNode
{
private:
    std::string destination;
    std::string writeMask;
    bool isFragmentResultRegister;
    bool isVertexResultRegister;

public:
    IRDstOperand(const std::string& destination);
    
    void setWriteMask(const std::string& writeMask_)                  {  this->writeMask = writeMask_;    }
    void setIsFragmentResultRegister(bool isFragmentResultRegister_)  {  this->isFragmentResultRegister = isFragmentResultRegister_;  }
    void setIsVertexResultRegister(bool isVertexResultRegister_)      {  this->isVertexResultRegister = isVertexResultRegister_;  }
    
    std::string getWriteMask() const         {   return writeMask;  }
    std::string getDestination() const       {   return destination;    }
    bool getIsFragmentResultRegister() const {   return isFragmentResultRegister;   }
    bool getIsVertexResultRegister() const   {   return isVertexResultRegister;     }

    virtual void traverse(ACDXIRTraverser*);
};

class IRParamBinding;

class IRArrayAddressing: public ACDXIRNode
{
private:
    unsigned int arrayAddressOffset;
    bool relativeAddress;
    std::string  relativeAddressReg;
    std::string  relativeAddressComp;
    unsigned int relativeAddressOffset;

public:
    IRArrayAddressing(unsigned int arrayAddressOffset = 0, bool relativeAddress = false);

    void setRelativeAddressing(std::string& relativeAddressReg, std::string& relativeAddressComp,
                               unsigned int relativeAddressOffset);
    
    bool getIsRelativeAddress() const             {  return relativeAddress; }
    unsigned int getArrayAddressOffset() const    {  return arrayAddressOffset;  }
    std::string getRelativeAddressReg() const     {  return relativeAddressReg;  }
    std::string getRelativeAddressComp() const    {  return relativeAddressComp;  }
    unsigned int getRelativeAddressOffset() const {  return relativeAddressOffset;  }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRSrcOperand: public ACDXIRNode
{
private:

    std::string source;
    std::string swizzleMask;
    bool negateFlag;

    bool isFragmentRegister;
    bool isVertexRegister;
    
    bool isParamBinding;
    IRParamBinding* paramBinding;

    bool isArrayAddressing;
    IRArrayAddressing*  arrayAddrInfo;

public:
    IRSrcOperand(const std::string& source);

    void setIsFragmentRegister(bool isFragmentRegister_) { this->isFragmentRegister = isFragmentRegister_;    }
    void setIsVertexRegister(bool isVertexRegister_)     { this->isVertexRegister = isVertexRegister_;    }
    void setSwizzleMask(const std::string& swizzleMask_) { this->swizzleMask = swizzleMask_;  }
    void setNegateFlag()                                { this->negateFlag = true;  }
    void setArrayAddressing(IRArrayAddressing*  arrayAddrInfo_)
    {   
        this->isArrayAddressing = true; 
        this->arrayAddrInfo = arrayAddrInfo_;
    }
    void setParamBinding(IRParamBinding *paramBinding_)
    {   
        this->isParamBinding = true;
        this->paramBinding = paramBinding_;
    }

    std::string getSource() const                 {   return source;    }
    bool getIsFragmentRegister() const            {   return isFragmentRegister;    }
    bool getIsVertexRegister() const              {   return isVertexRegister;    }
    bool getIsParamBinding() const                {   return isParamBinding;  }
    IRParamBinding* getParamBinding() const       {   return paramBinding;  }
    std::string getSwizzleMask() const            {   return swizzleMask; }
    bool getNegateFlag() const                    {   return negateFlag;  }
    bool getIsArrayAddressing() const             {   return isArrayAddressing; }
    IRArrayAddressing* getArrayAddressing() const {   return arrayAddrInfo;   }
    
    virtual void traverse(ACDXIRTraverser*);
    virtual ~IRSrcOperand();
};

class IRNamingStatement: public IRStatement
{
protected:
    std::string name;

public:
    IRNamingStatement(unsigned int line, const std::string& name);
    
    std::string getName() const  {   return name;   }
    
    virtual void traverse(ACDXIRTraverser*)=0;
    virtual ~IRNamingStatement();
};

class IRALIASStatement: public IRNamingStatement
{
private:
    std::string alias;

public:
    IRALIASStatement(unsigned int line, const std::string& name);

    void setAlias(const std::string& alias_) {   this->alias = alias_;    }
    
    std::string getAlias() const {   return alias;   }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRTEMPStatement: public IRNamingStatement
{
private:
    std::list<std::string>* otherTemporariesNames;

public:
    IRTEMPStatement(unsigned int line, const std::string& name);

    void setOtherTemporaries(std::list<std::string>* otherTemporariesNames_) {    this->otherTemporariesNames = otherTemporariesNames_;   }

    std::list<std::string>* getOtherTemporaries() const  { return otherTemporariesNames; }
    
    virtual void traverse(ACDXIRTraverser*);
    virtual ~IRTEMPStatement();
};

class IRADDRESSStatement: public IRNamingStatement
{
private:
    std::list<std::string>* otherAddressRegisters;

public:
    IRADDRESSStatement(unsigned int line, const std::string& name);

    void setOtherAddressRegisters(std::list<std::string>* otherAddressRegisters_) {    this->otherAddressRegisters = otherAddressRegisters_;   }

    std::list<std::string>* getOtherAddressRegisters() const  { return otherAddressRegisters; }
    
    virtual void traverse(ACDXIRTraverser*);
    virtual ~IRADDRESSStatement();
};

class IRATTRIBStatement: public IRNamingStatement
{
private:
    std::string inputAttribute;

public:
    IRATTRIBStatement(unsigned int line, const std::string& name);
    
    void setInputAttribute(const std::string& inputAttribute_)    {   this->inputAttribute = inputAttribute_; }
    
    std::string getInputAttribute() const     {   return inputAttribute;  }
    
    virtual void traverse(ACDXIRTraverser*)=0;
};

class VP1IRATTRIBStatement: public IRATTRIBStatement
{
public:
    VP1IRATTRIBStatement(unsigned int line, const std::string& name);
    
    virtual void traverse(ACDXIRTraverser*);
};

class FP1IRATTRIBStatement: public IRATTRIBStatement
{
public:
    FP1IRATTRIBStatement(unsigned int line, const std::string& name);
    
    virtual void traverse(ACDXIRTraverser*);
};


class IROUTPUTStatement: public IRNamingStatement
{
private:
    std::string outputAttribute;

public:
    IROUTPUTStatement(unsigned int line, const std::string& name);

    void setOutputAttribute(const std::string& outputAttribute_)  { this->outputAttribute = outputAttribute_; }
    
    std::string getOutputAttribute() const    {   return outputAttribute; }
    
    virtual void traverse(ACDXIRTraverser*)=0;
};

class VP1IROUTPUTStatement: public IROUTPUTStatement
{
public:
    VP1IROUTPUTStatement(unsigned int line, const std::string& name);

    virtual void traverse(ACDXIRTraverser*);
};

class FP1IROUTPUTStatement: public IROUTPUTStatement
{
public:
    FP1IROUTPUTStatement(unsigned int line, const std::string& name);

    virtual void traverse(ACDXIRTraverser*);
};


class IRPARAMStatement: public IRNamingStatement
{
    int size;
    bool isMultipleBindings;
    std::list<IRParamBinding *>* paramBindings;

public:
    IRPARAMStatement(unsigned int line, const std::string& name, bool isMultipleBindings=false);

    void setParamBindings(std::list<IRParamBinding *>* paramBindings_)    {   this->paramBindings = paramBindings_;  }
    void setSize(int size_)                                               {   this->size = size_;   }

    int getSize() const                                   {   return size;    }
    bool getIsMultipleBindings() const                    {   return isMultipleBindings;    }
    std::list<IRParamBinding *>* getParamBindings() const {   return paramBindings;   }
    
    virtual void traverse(ACDXIRTraverser*);
    virtual ~IRPARAMStatement();
};

class IRParamBinding: public ACDXIRNode
{
private:
    bool isImplicitBinding;

public:
    IRParamBinding();

    void setIsImplicitBinding(bool isImplicitBinding_) {   this->isImplicitBinding = isImplicitBinding_;   }
    
    bool getIsImplicitBinding() const {   return isImplicitBinding;   }

    // It has to be redefined by subclasses
    virtual unsigned int getVectorsBinded() const = 0;
    
    virtual void traverse(ACDXIRTraverser*) = 0;
};


class IRLocalEnvBinding: public IRParamBinding
{
public:
    enum PType { LOCALPARAM, ENVPARAM };

private:
    PType paramType;
    unsigned int minIndex;
    unsigned int maxIndex;

public:
    IRLocalEnvBinding(PType paramType);
    
    void setIndices(unsigned int minIndex_, unsigned int maxIndex_)   
    {
        this->minIndex = minIndex_;
        this->maxIndex = maxIndex_;
    }
    
    PType getType() const                         {   return paramType;   }
    unsigned int getMinIndex() const              {   return minIndex;    }
    unsigned int getMaxIndex() const              {   return maxIndex;    }
    
    virtual unsigned int getVectorsBinded() const {   return maxIndex-minIndex+1;   }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRConstantBinding: public IRParamBinding
{
private:
    float value1;
    float value2;
    float value3;
    float value4;
    bool isScalarConst;

public:
    IRConstantBinding(float v1, float v2, float v3, float v4, bool isScalarConst = false);

    float getValue1() const                       {   return value1;  }
    float getValue2() const                       {   return value2;  }
    float getValue3() const                       {   return value3;  }
    float getValue4() const                       {   return value4;  }
    bool getIsScalarConst() const                 {   return isScalarConst; }
    
    virtual unsigned int getVectorsBinded() const {   return 1;   }
    
    virtual void traverse(ACDXIRTraverser*);
};

class IRStateBinding: public IRParamBinding
{
private:
    std::string state;
    bool isMatrixState;

public:
    IRStateBinding(const std::string& state, bool isMatrixState = false);
    
    std::string getState() const                  {   return state;   }
    
    virtual unsigned int getVectorsBinded() const {   return isMatrixState?4:1;   }
    
    virtual void traverse(ACDXIRTraverser*);
};

} // namespace GenerationCode

} // namespace acdlib


#endif // ACDX_IRNODE_H
