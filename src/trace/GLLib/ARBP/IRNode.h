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

#ifndef IRNODE_H
    #define IRNODE_H
#include <list>
#include <string>

namespace libgl
{

namespace GenerationCode
{

/* Forwarding of traverser base class */

class IRTraverser;

/**
 *  Abstract base class for the Intermediate Representation(IR) tree
 */

class IRNode
{
protected:
    unsigned int line;

public:
    IRNode();
    IRNode(unsigned int line);

    void setLine(unsigned int line) { this->line = line; }
    
    unsigned int getLine() const { return line; }

    virtual void traverse(IRTraverser*) = 0;
    virtual ~IRNode();
};

/**
 *  The class for the tree root. It manages a list of options
 *  specified in the program and the list of statements (declarations
 *  and instructions).
 */

class IROption;
class IRStatement;

class IRProgram: public IRNode
{
private:
    std::string headerString;
    std::list<IROption *>*    programOptions;
    std::list<IRStatement *>* programStatements;

public:
    IRProgram();
    IRProgram(const std::string& headerstring);

    void setHeaderString(const std::string& headerString)   {   this->headerString = headerString;  }
    void addOptionList(std::list<IROption *>* programOptions)   {   this->programOptions = programOptions;  }
    void addStatementList(std::list<IRStatement *>* programStatements)  {   this->programStatements = programStatements;    }

    std::string getHeaderString() const { return headerString; }

    void dump(std::ostream& os);
    
    friend std::ostream& operator<<(std::ostream& os, IRProgram *ir)
    {
        ir->dump(os);
        return os;
    }

    virtual void traverse(IRTraverser*);
    virtual ~IRProgram();
};

/**
 *  The class that represents a program option. The option
 *  can have a associated value. At moment this value is a
 *  positive integer number.
 */

class IROption: public IRNode
{
private:
    std::string optionString;

public:
    IROption(unsigned int line, const std::string& optionString);
    
    std::string getOptionString() const { return optionString; }
    
    virtual void traverse(IRTraverser*)= 0;
};

class VP1IROption: public IROption
{
public:
    VP1IROption(unsigned int line, const std::string& optionString);
    
    virtual void traverse(IRTraverser*);
};

class FP1IROption: public IROption
{
public:
    FP1IROption(unsigned int line, const std::string& optionString);
    
    virtual void traverse(IRTraverser*);
};

class IRStatement: public IRNode
{
public:
    IRStatement(unsigned int line);
    
    virtual void traverse(IRTraverser*)=0;
    virtual ~IRStatement();
};

class IRSrcOperand;
class IRDstOperand;

class IRInstruction: public IRStatement
{
private:
    std::string                 opCode;

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
    
    virtual void traverse(IRTraverser*);
    virtual ~IRInstruction();
};

class IRSampleInstruction: public IRInstruction
{
private:
    unsigned int textureImageUnit;
    std::string glTextureTarget;

public:
    IRSampleInstruction(unsigned int line, const std::string& opcode);
    
    void setTextureImageUnit(unsigned int textureImageUnit)   {  this->textureImageUnit = textureImageUnit; }
    void setTextureTarget(const std::string& glTextureTarget) {  this->glTextureTarget = glTextureTarget;   }
    
    unsigned int getTextureImageUnit() const {  return textureImageUnit;  }
    std::string getGLTextureTarget() const   {  return glTextureTarget;   }
    
    virtual void traverse(IRTraverser*);
};

class IRSwizzleComponents;

class IRSwizzleInstruction: public IRInstruction
{
private:
    IRSwizzleComponents* swizzleComps;

public:
    IRSwizzleInstruction(unsigned int line, const std::string& opcode);
    
    void setSwizzleComponents(IRSwizzleComponents* swizzleComps)   { this->swizzleComps = swizzleComps; }
    
    IRSwizzleComponents* getSwizzleComponents() const { return swizzleComps;  }
    
    virtual void traverse(IRTraverser*);    
    virtual ~IRSwizzleInstruction();
};

class IRSwizzleComponents: public IRNode
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
    
    virtual void traverse(IRTraverser*);
};

class IRDstOperand: public IRNode
{
private:
    std::string destination;
    std::string writeMask;
    bool isFragmentResultRegister;
    bool isVertexResultRegister;

public:
    IRDstOperand(const std::string& destination);
    
    void setWriteMask(const std::string& writeMask)                  {  this->writeMask = writeMask;    }
    void setIsFragmentResultRegister(bool isFragmentResultRegister)  {  this->isFragmentResultRegister = isFragmentResultRegister;  }
    void setIsVertexResultRegister(bool isVertexResultRegister)      {  this->isVertexResultRegister = isVertexResultRegister;  }
    
    std::string getWriteMask() const         {   return writeMask;  }
    std::string getDestination() const       {   return destination;    }
    bool getIsFragmentResultRegister() const {   return isFragmentResultRegister;   }
    bool getIsVertexResultRegister() const   {   return isVertexResultRegister;     }

    virtual void traverse(IRTraverser*);
};

class IRParamBinding;

class IRArrayAddressing: public IRNode
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
    
    virtual void traverse(IRTraverser*);
};

class IRSrcOperand: public IRNode
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

    void setIsFragmentRegister(bool isFragmentRegister) { this->isFragmentRegister = isFragmentRegister;    }
    void setIsVertexRegister(bool isVertexRegister)     { this->isVertexRegister = isVertexRegister;    }
    void setSwizzleMask(const std::string& swizzleMask) { this->swizzleMask = swizzleMask;  }
    void setNegateFlag()                                { this->negateFlag = true;  }
    void setArrayAddressing(IRArrayAddressing*  arrayAddrInfo)
    {   
        this->isArrayAddressing = true; 
        this->arrayAddrInfo = arrayAddrInfo;
    }
    void setParamBinding(IRParamBinding *paramBinding)
    {   
        this->isParamBinding = true;
        this->paramBinding = paramBinding;
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
    
    virtual void traverse(IRTraverser*);
    virtual ~IRSrcOperand();
};

class IRNamingStatement: public IRStatement
{
protected:
    std::string name;

public:
    IRNamingStatement(unsigned int line, const std::string& name);
    
    std::string getName() const  {   return name;   }
    
    virtual void traverse(IRTraverser*)=0;
    virtual ~IRNamingStatement();
};

class IRALIASStatement: public IRNamingStatement
{
private:
    std::string alias;

public:
    IRALIASStatement(unsigned int line, const std::string& name);

    void setAlias(const std::string& alias) {   this->alias = alias;    }
    
    std::string getAlias() const {   return alias;   }
    
    virtual void traverse(IRTraverser*);
};

class IRTEMPStatement: public IRNamingStatement
{
private:
    std::list<std::string>* otherTemporariesNames;

public:
    IRTEMPStatement(unsigned int line, const std::string& name);

    void setOtherTemporaries(std::list<std::string>* otherTemporariesNames) {    this->otherTemporariesNames = otherTemporariesNames;   }

    std::list<std::string>* getOtherTemporaries() const  { return otherTemporariesNames; }
    
    virtual void traverse(IRTraverser*);
    virtual ~IRTEMPStatement();
};

class IRADDRESSStatement: public IRNamingStatement
{
private:
    std::list<std::string>* otherAddressRegisters;

public:
    IRADDRESSStatement(unsigned int line, const std::string& name);

    void setOtherAddressRegisters(std::list<std::string>* otherAddressRegisters) {    this->otherAddressRegisters = otherAddressRegisters;   }

    std::list<std::string>* getOtherAddressRegisters() const  { return otherAddressRegisters; }
    
    virtual void traverse(IRTraverser*);
    virtual ~IRADDRESSStatement();
};

class IRATTRIBStatement: public IRNamingStatement
{
private:
    std::string inputAttribute;

public:
    IRATTRIBStatement(unsigned int line, const std::string& name);
    
    void setInputAttribute(const std::string& inputAttribute)    {   this->inputAttribute = inputAttribute; }
    
    std::string getInputAttribute() const     {   return inputAttribute;  }
    
    virtual void traverse(IRTraverser*)=0;
};

class VP1IRATTRIBStatement: public IRATTRIBStatement
{
public:
    VP1IRATTRIBStatement(unsigned int line, const std::string& name);
    
    virtual void traverse(IRTraverser*);
};

class FP1IRATTRIBStatement: public IRATTRIBStatement
{
public:
    FP1IRATTRIBStatement(unsigned int line, const std::string& name);
    
    virtual void traverse(IRTraverser*);
};


class IROUTPUTStatement: public IRNamingStatement
{
private:
    std::string outputAttribute;

public:
    IROUTPUTStatement(unsigned int line, const std::string& name);

    void setOutputAttribute(const std::string& outputAttribute)  { this->outputAttribute = outputAttribute; }
    
    std::string getOutputAttribute() const    {   return outputAttribute; }
    
    virtual void traverse(IRTraverser*)=0;
};

class VP1IROUTPUTStatement: public IROUTPUTStatement
{
public:
    VP1IROUTPUTStatement(unsigned int line, const std::string& name);

    virtual void traverse(IRTraverser*);
};

class FP1IROUTPUTStatement: public IROUTPUTStatement
{
public:
    FP1IROUTPUTStatement(unsigned int line, const std::string& name);

    virtual void traverse(IRTraverser*);
};


class IRPARAMStatement: public IRNamingStatement
{
    int size;
    bool isMultipleBindings;
    std::list<IRParamBinding *>* paramBindings;

public:
    IRPARAMStatement(unsigned int line, const std::string& name, bool isMultipleBindings=false);

    void setParamBindings(std::list<IRParamBinding *>* paramBindings)    {   this->paramBindings = paramBindings;  }
    void setSize(int size)                                               {   this->size = size;   }

    int getSize() const                                   {   return size;    }
    bool getIsMultipleBindings() const                    {   return isMultipleBindings;    }
    std::list<IRParamBinding *>* getParamBindings() const {   return paramBindings;   }
    
    virtual void traverse(IRTraverser*);
    virtual ~IRPARAMStatement();
};

class IRParamBinding: public IRNode
{
private:
    bool isImplicitBinding;

public:
    IRParamBinding();

    void setIsImplicitBinding(bool isImplicitBinding) {   this->isImplicitBinding = isImplicitBinding;   }
    
    bool getIsImplicitBinding() const {   return isImplicitBinding;   }

    // It has to be redefined by subclasses
    virtual unsigned int getVectorsBinded() const = 0;
    
    virtual void traverse(IRTraverser*) = 0;
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
    
    void setIndices(unsigned int minIndex, unsigned int maxIndex)   
    {
        this->minIndex = minIndex;
        this->maxIndex = maxIndex;
    }
    
    PType getType() const                         {   return paramType;   }
    unsigned int getMinIndex() const              {   return minIndex;    }
    unsigned int getMaxIndex() const              {   return maxIndex;    }
    
    virtual unsigned int getVectorsBinded() const {   return maxIndex-minIndex+1;   }
    
    virtual void traverse(IRTraverser*);
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
    
    virtual void traverse(IRTraverser*);
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
    
    virtual void traverse(IRTraverser*);
};

} // namespace GenerationCode

} // namespace libgl


#endif
