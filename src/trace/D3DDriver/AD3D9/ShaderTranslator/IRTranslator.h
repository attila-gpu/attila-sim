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

/**
 * @author Chema Solís
 **/

#ifndef __IR_TRANSLATOR
#define __IR_TRANSLATOR

#include "IR.h"
#include "ShaderGenerate.h"
#include <vector>
#include <map>
#include <stack>
#include <set>

class IRTranslator : public IRVisitor
{
public:

    /**
     *
     *  Visitor function for end nodes.
     *
     *  @param n Pointer to the node to visit.
     *
     */

    void visit(EndIRNode *n);

    /**
     *
     *  Visitor function for declaration (dcl) nodes.
     *
     *  @param n Pointer to the node to visit.
     *
     */
     
    void visit(DeclarationIRNode *n);

    /**
     *
     *  Visitor function for instruction nodes.
     *
     *  @param n Pointer to the node to visit.
     *
     */
     
    void visit(InstructionIRNode *n);


    /**
     *
     *  Visitor function for version nodes.
     *
     *  @param n Pointer to the node to visit.
     *
     */
     
    void visit(VersionIRNode *n);


    /**
     *
     *  Visitor function for definition (def) nodes.
     *
     *  @param n Pointer to the node to visit.
     *
     */
     
    void visit(DefinitionIRNode *n);

    //  Empty functions.
    void begin();
    void visit(IRNode *n);
    void visit(CommentIRNode *n);
    void visit(CommentDataIRNode *n);
    void visit(ParameterIRNode *n);
    void visit(SourceParameterIRNode *n);
    void visit(DestinationParameterIRNode *n);
    void visit(BoolIRNode *n);
    void visit(FloatIRNode *n);
    void visit(IntegerIRNode *n);
    void visit(SamplerInfoIRNode *n);
    void visit(SemanticIRNode *n);
    void visit(RelativeAddressingIRNode *n);
    void end();

    /**
     *  
     *  Builds a NativeShader object storing the shader program
     *  translated to ATTILA bytecode and related information
     *  (register declarations, etc).
     *
     *  @return A pointer to a new NativeShader object storing the translated
     *  shader program and associated information.
     *
     */
     
    NativeShader *build_native_shader();
    
    /**
     *
     *  Get a reference to the list with the translated ATTILA instructions
     *  for the shader program.
     *
     *  @return A reference to the list of ATTILA shader instructions for
     *  the translated shader program.
     *
     */
    std::vector<gpu3d::ShaderInstruction *> &get_instructions();

    /**
     *
     *  Sets the alpha test comparison function for alpha test code generation.
     *
     *  @param alpha_func The alpha test comparison function.
     *
     */
	void setAlphaTest(D3DCMPFUNC alpha_func);
	
	/**
	 *
	 *  Sets if fog is enabled.
	 *
	 *  @param fog_enabled A boolean value storing if fog is enabled.
	 *
	 */
	 
	void setFogEnabled(bool fog_enabled);

    /**
     *
     * Get if the shader has control flow instructions that couldn't be translated.
     *
     *  @return If the shader has control flow instruction that couldn't be translated.
     *
     */
     
    bool getUntranslatedControlFlow();
    
    /**
     *
     *  Get the number of shader instructions that couldn't be translated.
     *
     *  @return The number of instructions that couldn't be translated.
     *
     */
     
    u32bit getUntranslatedInstructions();
         
    /** 
     *
     *  IRTranslator constructor.
     *
     *  @return A new initialized IRTranslator object.
     *
     */
     
    IRTranslator();

    /** 
     *
     *  IRTranslator destructor.
     *
     */
     
    ~IRTranslator();
    
private:

    static const u32bit ALPHA_TEST_CONSTANT_MINUS_ONE = 288;
    static const u32bit ALPHA_TEST_CONSTANT_REF = 289;
    static const u32bit FOG_CONSTANT_COLOR = 290;
    
    static const u32bit BOOLEAN_CONSTANT_START = 256;
    static const u32bit INTEGER_CONSTANT_START = 272;
    
    static const u32bit IF_ELSE_MIN_INSTR_PER_JUMP = 4;
    
    //  Shader model for the program.
    ShaderType type;                                /**<  Shader type (vertex shader, pixel shader) for the shader program to translate.  */
    DWORD version;                                  /**<  Shader model version for the shader program to translate.  */
	
	//  Fixed function emulation.
	D3DCMPFUNC alpha_func;                          /**<  Alpha test comparison function for alpha test emulation.  */
	bool fog_enabled;                               /**<  Defines if fog is enabled (?).  */
	AlphaTestDeclaration alpha_test_declaration;    /**<  Stores information about alpha test for alpha test emulation.  */
	FogDeclaration fog_declaration;                 /**<  Stores information about fog for fog emulation.  */

    //  Information about the current instruction.
    std::vector<Operand> operands;                  /**<  Stores the operands for the current instruction.  */
    Result result;                                  /**<  Stores the result for the current instruction.  */
    RelativeMode relative;                          /**<  Stores information about relative mode addressing for the current instruction.  */

    /**
     *
     *  Stores information for a IF/ELSE/ENDIF code block.
     *
     */
    struct IFBlockInfo
    {
        u32bit predicateRegister;
        bool predicateNegated;
        bool insideIFBlock;
        bool insideELSEBlock;
        u32bit jumpForIF;
        u32bit jumpForELSE;
    };
    
    /**
     *
     *  Stores information for a REP/ENDREP code block.
     */
    struct REPBlockInfo
    {
        u32bit start;           /**<  Stores the offset of the first instruction in the REP/ENDREP code block.  */
        u32bit iterations;      /**<  Stores the number of iterations of the REP/ENDREP code block.  */
        bool foundBREAK;        /**<  Stores if a BREAK instruction was found inside the REP/ENDREP code block.  */
        u32bit predicateREP;    /**<  Stores the predicate register for the REP block code.  */
        bool jumpBREAK;         /**<  Stores if a jump instruction was generated for the BREAK.  */
        u32bit jumpForBREAK;    /**<  Sotres the position in the instruction stream of the jump instruction for the BREAK instruction.  */
        u32bit tempCounter;     /**<  Stores the temporary register used as iteration counter.  */
        u32bit headerInstr;     /**<  Stores the number of instructions in the REP block header.  */
        u32bit footerInstr;     /**<  Stores the number of instructions in the REP block footer.  */
        
        REPBlockInfo() : start(0), iterations(0), foundBREAK(false), headerInstr(0), footerInstr(0), jumpBREAK(false) {}
    };
    
    //  Information about the shader program.
    bool untranslatedControlFlow;                   /**<  Stores that control flow instructions that couldn't be translated were found.  */
    u32bit untranslatedInstructions;                /**<  Stores the number of instructions that couldn't be translated.  */
    bool error;                                     /**<  Stores if the current instruction couldn't be translated.  */
    bool insideIFBlock;                             /**<  Stores if translating instructions inside an IF code block.  */
    bool insideELSEBlock;                           /**<  Stores if translating instructions inside an ELSE code block.  */
    std::vector<IFBlockInfo> currentIFBlocks;       /**<  Stores information about the current IF/ELSE/ENDIF code blocks.  */
    bool insideREPBlock;                            /**<  Stores if translating instructions inside a REP code block.  */
    std::vector<REPBlockInfo> currentREPBlocks;     /**<  Stores information for the current REP/ENDREP code blocks.  */
    std::vector<PredicationInfo> predication;       /**<  Stores information about predication for the next instructions.  */
    u32bit lastJumpTarget;                          /**<  Stores the address of the last jump target.  */
    
    std::vector<gpu3d::ShaderInstruction*> instructions;    /**<  Stores the list of ATTILA shader instructions for the translated shader program.  */
    
    //  Information about register availability.
    std::set<GPURegisterId> availableConst;         /**<  Stores the currently available ATTILA constant registers.  */
    std::set<GPURegisterId> availableTemp;          /**<  Stores the currently available ATTILA temporary registers.  */
    std::set<GPURegisterId> availableInput;         /**<  Stores the currently available ATTILA input registers.  */
    std::set<GPURegisterId> availableOutput;        /**<  Stores the currently available ATTILA ouput registers.  */
    std::set<GPURegisterId> availableSamplers;      /**<  Stores the currently available ATTILA sampler/texture registers.  */
    std::set<GPURegisterId> availablePredicates;    /**<  Stores the currently available ATTILA predicate registers.  */

    //  Information about register declaration.    
    std::list<ConstRegisterDeclaration> constDeclaration;       /**<  Stores the constant registers declared by the shader program.  */
    std::list<InputRegisterDeclaration> inputDeclaration;       /**<  Stores the input registers declared by the shader program.  */
    std::list<OutputRegisterDeclaration> outputDeclaration;     /**<  Stores the output registers declared by the shader program.  */
    std::list<SamplerDeclaration> samplerDeclaration;           /**<  Stores the sampler/texture registers declared by the shader program.  */

    //  Information about register mapping.
    std::map<D3DUsageId, GPURegisterId> inputUsage;         /**<  Maps D3D9 input registers to ATTILA registers (usage restrictions).  */
    std::map<D3DUsageId, GPURegisterId> outputUsage;        /**<  Maps D3D9 output registes to ATTILA registers (usage restrictions).  */
    std::map<D3DRegisterId, GPURegisterId> registerMap;     /**<  Current mapping of D3D9 registers to ATTILA registers.  */

    //  Static tables for instruction translation.    
    std::map<D3DSHADER_INSTRUCTION_OPCODE_TYPE, gpu3d::ShOpcode> opcodeMap;     /**<  Maps D3D9 opcodes to ATTILA opcodes.  */
    std::map<DWORD, gpu3d::MaskMode> maskModeMap;                               /**<  Maps D3D9 result mask modes to the corresponding ATTILA result mask modes.  */
    std::map<DWORD, gpu3d::SwizzleMode> swizzleModeMap;                         /**<  Maps D3D9 operand swizzle modes to the corresponding ATTILA operand swizzle modes.  */

    /**
     *
     *  Deletes the ATTILA shader instructions for the translated shader program.
     *
     */
    void delete_instructions();
    
    /**
     *
     *  Generates extra code for fixed function emulation (alpha test and fog).
     *
     *  @param color_temp Identifier of an ATTILA temporary register that stores
     *  the color output from the translated shader program.
     *  @param color_out Identifier of the an ATTILA output register where the
     *  final color output must be written.
     * 
     */
     
	void generate_extra_code(GPURegisterId color_temp, GPURegisterId color_out);
    
    /**
     *
     *  Visitor for source parameters node of instruction nodes.
     *
     *  @param n Pointer to the source parameter node.
     *
     */

    void visitInstructionSource(SourceParameterIRNode *n);

    /**
     *
     *  Visitor for destination parameters node of instruction nodes.
     *
     *  @param n Pointer to the destination parameter node.
     *
     */
     
    void visitInstructionDestination(DestinationParameterIRNode *n);  

    /**
     *
     *  Used to select a modified component of the result as a placeholder
     *  for temporal results in the emulation of complex shader instructions.    
     *  The returned Operand object has all the flags cleared.
     *
     *  @param result An instruction result from which to extract a written component.
     *  @param temp A reference to an instruction operand which swizzle mode will be
     *  set to a broadcast of the selected result written component.
     *
     */
     
    void selectComponentFromResult(Result result, Operand &temp);
    
    /**
     *
     *  Select the component swizzled to a given component.
     *
     *  @param component The swizzled component to select in the input operand.
     *  @param in The input operand from which the swizzled component will be selected.
     *  @param out The output operand which swizzle mode will be set to a broadcast
     *  to the selected input operand swizzled component.
     *
     */
     
    void selectSwizzledComponent(u32bit component, Operand in, Operand &out);
    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  Initializations
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void initializeRegisters();
    void initializeOpcodes();

    
    ///////////////////////////////////////////////////////////////////////////
    //
    //  Register management.
    //
    ///////////////////////////////////////////////////////////////////////////

    
    /**
     *
     *  Declares a D3D9 constant register, reserves an ATTILA constant register and
     *  maps the D3D9 constant register to the ATTILA constant register.
     *
     *  @param d3dReg The D3D9 constant register identifier.
     *
     *  @return The identifier of the ATTILA constant register to which the D3D9 constant register
     *  has been mapped to.
     *
     */
     
    GPURegisterId declareMapAndReserveConst(D3DRegisterId d3dreg);
    
    /**
     *
     *  Declares a D3D9 constant register, reserves an ATTILA constant register,
     *  maps the D3D9 constant register to the ATTILA constant register and sets
     *  the value of the constant register.
     *
     *  @param d3dReg The D3D9 constant register identifier.
     *  @param value Value of the constant register.
     *
     *  @return The identifier of the ATTILA constant register to which the D3D9 constant register
     *  has been mapped to.
     *
     */

    GPURegisterId declareMapAndReserveConst(D3DRegisterId d3dreg, ConstValue value);

    /** 
     *
     *  Checks if a D3D9 register is mapped to an ATTILA register.
     *
     *  @param d3dreg Identifier of the D3D9 register.
     *
     *  @return Returns TRUE if the D3D9 register is mapped to an ATTILA register, FALSE otherwise.
     *
     */
     
    bool isMapped(D3DRegisterId d3dreg);
    
    /**
     *
     *  Reserves an ATTILA register and maps the D3D9 register to the ATTILA register.
     *
     *  @param d3dreg Identifier of the D3D9 register.
     *
     *  @return The identifier of the ATTILA register to which the D3D9 register has been mapped to.
     *
     */
     
    GPURegisterId mapAndReserveRegister(D3DRegisterId d3dreg);
    
    /**
     *
     *  Maps a D3D9 register to an ATTILA register.
     *
     *  @param d3dreg Identifier of the D3D9 register.
     *  @param gpureg Identifier of the ATTILA register.
     *
     */
     
    void mapRegister(D3DRegisterId d3dreg, GPURegisterId gpureg);

    /**
     *
     *  Removes existing mapping of a given D3D9 register to an ATTILA register.
     *
     *  @param d3dreg Identifier of the D3D9 register.
     *
     *  @return The identifier of the ATTILA register to which the D3D9 register has been mapped to.
     *
     */
     
    GPURegisterId unmapRegister(D3DRegisterId d3dreg);    

    /**
     *
     *  Gets the mapping to an ATTILA register of a D3D9 register.
     *
     *  @param d3dreg Identifier of the D3D9 register.
     *
     *  @return The identifier of ATTILA register to which the D3D9 register has been mapped to.
     *
     */
     
    GPURegisterId mappedRegister(D3DRegisterId d3dreg);

    /**
     *
     *  Reserves an ATTILA temporary register
     *
     *  @return The identifier of the reserved ATTILA temporary register.
     *     
     */
     
    GPURegisterId reserveTemp();

    /**
     *
     *  Reserves an ATTILA temporary register
     *
     *  @param gpu_temp_register Identifier of the ATTILA temporary register to reserve.
     *
     *  @return The identifier of the reserved ATTILA temporary register.
     *     
     */

    GPURegisterId reserveTemp(GPURegisterId gpu_temp_register);

    /**
     *
     *  Reserves an ATTILA temporary register and maps the D3D9 register to the ATTILA temporary register.
     *
     *  @param d3d_register Identifier of the D3D9 register.
     *
     *  @return The identifier of the reserved ATTILA temporary register.
     *     
     */
          
    GPURegisterId reserveAndMapTemp(D3DRegisterId d3d_register);
    
    /**
     *
     *  Releases an ATTILA temporary register.
     *
     *  @param reg Identifier of the ATTILA temporary register to release.
     *
     */
     
    void releaseTemp(GPURegisterId reg);

    /**
     *
     *  Declares a D3D9 input register, reserves an ATTILA register and maps the D3D9 input register
     *  to the ATTILA register.
     *
     *  @param usage Defines the usage of the D3D9 input register.
     *  @param d3d_register Identifier of the D3D9 input register.
     *
     *  @return Identifier of the ATTILA register reserved and to which the D3D9 input register
     *  was mapped to.
     *
     */
     
    GPURegisterId declareMapAndReserveInput(D3DUsageId usage, D3DRegisterId d3d_register);

    /**
     *
     *  Declares a D3D9 output register, reserves an ATTILA register and maps the D3D9 output register
     *  to the ATTILA register.
     *
     *  @param usage Defines the usage of the D3D9 output register.
     *  @param d3d_register Identifier of the D3D9 output register.
     *
     *  @return Identifier of the ATTILA register reserved and to which the D3D9 output register
     *  was mapped to.
     *
     */

    GPURegisterId declareMapAndReserveOutput(D3DUsageId usage, D3DRegisterId d3d_register);

    /**
     *
     *  Declares a D3D9 sampler/texture register, reserves an ATTILA texture register and maps the D3D9 texture register
     *  to the ATTILA texture register.
     *
     *  @param type Texture type (1D, 2D, CUBE, 3D, ...).
     *  @param d3d_register Identifier of the D3D9 texture register.
     *
     *  @return Identifier of the ATTILA texture register reserved and to which the D3D9 texture register
     *  was mapped to.
     *
     */

    GPURegisterId declareMapAndReserveSampler(D3DSAMPLER_TEXTURE_TYPE type, D3DRegisterId d3d_register);

    /**
     *
     *  Reserves an ATTILA predicate register.
     *
     *  @return The identifier of the reserved ATTILA predicate register.  
     *
     */
     
    GPURegisterId reservePredicate();

    /**
     *
     *  Reserves an ATTILA predicate register.
     *
     *  @param gpu_pred_register The identifier of the ATTILA predicate register to reserve.
     *
     *  @return The identifier of the reserved ATTILA predicate register.  
     *
     */

    GPURegisterId reservePredicate(GPURegisterId gpu_pred_register);
    
    /**
     *
     *  Reserves an ATTILA predicate register and maps the D3D9 predicate register to the
     *  ATTILA predicate register.
     *
     *  @param d3d_register The identifier of the D3D9 predicate register.
     *
     *  @return The identifier of the reserved ATTILA predicate register and to which the
     *  D3D9 predicate register has been mapped to.  
     *
     */

    GPURegisterId reserveAndMapPredicate(D3DRegisterId d3d_register);

    /**
     *
     *  Releases an ATTILA predicate register.
     *
     *  @param reg The identifier of the ATTILA predicate register to release.
     *
     */

    void releasePredicate(GPURegisterId reg);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  Instruction generation.
    //
    ///////////////////////////////////////////////////////////////////////////

    /**
     *
     *  Generates the ATTILA shader instructions to compute a predicate based
     *  on the IF comparison operation.
     *
     *  @param compareOp The IF comparison operation.
     *
     */
     
    void computeIFPredication(D3DSHADER_COMPARISON compareOp);

    /**
     *
     *  Generates the ATTILA shader instructions to compute a predicate based
     *  on the BREAKC comparison operation.
     *
     *  @param compareOp The BREAKC comparison operation.
     *  @param predReg Predicate register into which store the result of the comparison.
     *
     */
     
    void computeBREAKCPredication(D3DSHADER_COMPARISON compareOp, u32bit predReg);
    
    /**
     *
     *  Generate code for the start of IFC/ELSE/ENDIF code block.
     *  Computes the IF condition predicate, updates the predication stack and the IF/ELSE/ENDIF block stack.
     *
     *  @param comparisonMode The D3D9 shader comparison mode defined for the IFC instruction.
     *
     */
    
    void generateCodeForIFC(D3DSHADER_COMPARISON comparisonMode);

    /**
     *
     *  Generate code for the start of IFB/ELSE/ENDIF code block.
     *  Loads the IFB boolean constant to a predicate, updates the predication stack and the IF/ELSE/ENDIF block stack.
     *
     */
    
    void generateCodeForIFB();

    /**
     *
     *  Generate code for an ELSE instruction.
     *  Patch the code at the start of the IF/ELSE/ENDIF block.
     *  Update predication and IF/ELSE/ENDIF block state.
     *
     */

    void generateCodeForELSE();    

    /**
     *
     *  Generate code for an ENDIF instruction.
     *  Patch the code at the start of the IF and ELSE sides of the code block.
     *  Update predication and IF/ELSE/ENDIF block state.
     *
     */
   
    void generateCodeForENDIF();
    
    /**
     *
     *  Generate code at the start of a REP/ENDREP block.
     *  Update predication and REP/ENDREP block state.
     *
     */
     
    void generateCodeForREP();
    
    /**
     *
     *  Generate code at the end of a REP/ENDREP block.
     *  Patch code for BREAK if required.
     *  Update predication and REP/ENDREP block state.
     *
     */

    void generateCodeForENDREP();

    /**
     *
     *  Generate a code for a BREAK instruction.
     *  Update REP/ENDREP block state.
     *
     */
     
    void generateCodeForBREAK();
    
    /**
     *
     *  Generate a code for a BREAKC instruction.
     *  Update REP/ENDREP block state.
     *
     *  @param comparisonMode The D3D shader comparison mode defined for the BREAKC instruction.
     *
     */
     
    void generateCodeForBREAKC(D3DSHADER_COMPARISON comparisonMode);
    
    /**
     *
     *  Generates a MOV shader instruction to copy the source register to the
     *  destination register.
     *
     *  @param dest Identifier of the destination ATTILA register.
     *  @param src Identifier of the source ATTILA register.
     *
     */
    void generateMov(GPURegisterId dest, GPURegisterId src);
    
    /**
     *
     *  Generates a NOP shader instruction.
     *
     */
     
    void generateNOP();

    /**
     *
     *  Translates the D3D9 result write mask mode to the corresponding ATTILA result write mask mode.
     *
     *  @param d3dmm The D3D9 result write mask mode.
     *
     *  @return The corresponding ATTILA result write mask mode.
     *
     */
     
    gpu3d::MaskMode nativeMaskMode(DWORD d3dmm);
    
    /**
     *
     *  Translated the D3D9 operand swizzle mode to the corresponding ATTILA operand swizzle mode.
     *
     *  @param d3dswizz The D3D9 operand swizzle mode.
     *
     *  @return The corresponding ATTILA operand swizzle mode.
     *
     */
     
    gpu3d::SwizzleMode nativeSwizzleMode(DWORD d3dswizz);



    ///////////////////////////////////////////////////////////////////////////
    //
    //  Specific instruction translations.
    //
    ///////////////////////////////////////////////////////////////////////////
    
    /**
     *
     *  Generates the translation of a M4X4 D3D9 shader instruction to ATTILA shader instructions.
     *
     *  @param n Pointer to the shader instruction node.
     *
     */
     
    void emulateM4X4(InstructionIRNode *n);
    
    /**
     *
     *  Generates the translation of a TEXLDL1314 D3D9 shader instruction to ATTILA shader instructions.
     *
     *  @param n Pointer to the shader instruction node.
     *
     */

    void emulateTEXLD1314(InstructionIRNode *n);

    /**
     *
     *  Generates the translation of a SUB D3D9 shader instruction to ATTILA shader instructions.
     *
     */
     
    void emulateSUB();

    /**
     *
     *  Generates the translation of a LRP D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateLRP();

    /**
     *
     *  Generates the translation of a POW D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulatePOW();

    /**
     *
     *  Generates the translation of a NRM D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateNRM();

    /**
     *
     *  Generates the translation of a CMP D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateCMP();

    /**
     *
     *  Generates the translation of a DP2ADD D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateDP2ADD();

    /**
     *
     *  Generates the translation of a ABS D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateABS();

    /**
     *
     *  Generates the translation of a SINCOS D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateSINCOS();

    /**
     *
     *  Generates the translation of a TEXKILL D3D9 shader instruction to ATTILA shader instructions.
     *
     */

    void emulateTEXKILL();

};

#endif
