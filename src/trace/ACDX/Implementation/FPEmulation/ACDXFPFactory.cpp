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

#include "ACDXFPFactory.h"
#include "support.h"
#include <iostream>
//#include "TLShaderImp.h"
#include "ImplementedConstantBindingFunctions.h"
#include <sstream>

#define TEXTURELOOKUPCOLOR(x) "texLookupColor" + x + ""
#define TEXTUREENVIRONMENTCOLOR(x) "state.texenv[" + x + "].color"
#define ZERO_CONSTANT "{0,0,0,0}"
#define ONE_CONSTANT "{1,1,1,1}"
#define ONE_MINUS_CONSTANT "-{1,1,1,1}"

using namespace std;
using namespace acdlib;

ACDXTLShader* ACDXFPFactory::tlshader;
ACDXFPState* ACDXFPFactory::fps;
ACDXCodeSnip ACDXFPFactory::code;

/* Assembles the program parts created by the different construction functions */
ACDXTLShader* ACDXFPFactory::constructFragmentProgram(ACDXFPState& fps)
{
    ACDXFPFactory::fps = &fps;

    tlshader = new ACDXTLShader;

    code.clear();

    code.append("!!ARBfp1.0\n");
    constructInitialization();
    constructFragmentProcessing();
    constructPostProcessingTests();

    code.append("END");

    //code.removeComments();

    tlshader->setCode(code.toString());

    return tlshader; // must be detroyed externally
}

void ACDXFPFactory::constructInitialization()
{
    /* initialize input variables :
     *      position, color (primary or primary and secondary),
     *      fog distance and texture coordinates.
     */

    string text =   "# ------------ Fragment input binding ------------\n"
                    "ATTRIB iColorPrim = fragment.color.primary;\n";

    if (fps->separateSpecular())
            text += "ATTRIB iColorSec = fragment.color.secondary; \n";

    text += "ATTRIB iPos = fragment.position; \n";

    if (fps->anyTextureUnitEnabled())
    {
        for (int i=0; i<fps->maxTextureUnits(); i++)
        {
            if ( fps->isTextureUnitEnabled(i) )
            {
                ACDXFPState::TextureUnit textureUnit = fps->getTextureUnit(i);
                stringstream ss;
                string t;
                ss << i;
                t = ss.str();
                text += "ATTRIB iCoord" + t + " = fragment.texcoord[" + t + "];\n";
                text += "TEMP " TEXTURELOOKUPCOLOR(t) ";\n";

            }
        }
        text     += "TEMP texColorSum;\n";
    }

    if (fps->fogEnabled() ||
       (fps->alphaTestEnabled() && ((fps->alphaTestFunc() != ACDXFPState::ALWAYS) && (fps->alphaTestFunc() != ACDXFPState::NEVER))))
        text += "TEMP finalColor;\n";

    if (fps->alphaTestEnabled() && ((fps->alphaTestFunc() != ACDXFPState::ALWAYS) && (fps->alphaTestFunc() != ACDXFPState::NEVER)))
        text += "TEMP testReg;\n";

    if (fps->fogEnabled())
        text += "TEMP fogFactor;\n"
                "PARAM fogParam = program.local[41];\n";

    text += "# ------------ End Fragment input binding ------------\n";

    code.append(text);
}

void ACDXFPFactory::constructFragmentProcessing()
{
    if (fps->anyTextureUnitEnabled())
        constructTexturing();

    computeColorSum();

    if (fps->fogEnabled())
        computeFogApplication();

    /**
     * Fragment program optimization. Z-value write can be ommited in fragments
     * programs that doesnït generate a Z-value different that bypassing
     * the fragment input depth.
     *
     * ARB_fragment_program Specification Line 242
     *
     * "(7a) If a fragment program does not write a depth value, what should
     * be the final depth value of the fragment?
     *
     * RESOLVED: "Depth fly-over" (using the conventional depth produced
     * by rasterization) should happen whenever a depth-replacing program
     * is not in use.  A depth-replacing program is defined as a program
     * that writes to result.depth in at least one instruction."
     */
    // computeDepthBypass();
}

/* OpenGL Specification 2.0: Base internal format correspondence to components
 *
 *   Base Internal Format  |  RGBA and Depth Values  |  Internal components
 *  -------------------------------------------------------------------------
 *   ALPHA                 |  A                      |  A
 *   DEPTH_COMPONENT       |  Depth                  |  D
 *   LUMINANCE             |  R                      |  L
 *   LUMINANCE_ALPHA       |  R, A                   |  L, A
 *   INTENSITY             |  R                      |  I
 *   RGB                   |  R,G,B                  |  R,G,B
 *   RGBA                  |  R,G,B,A                |  R,G,B,A
 *
 *  Table 3.15: Conversion from RGBA and depth pixel components to internal
 *   texture, table, or filter components.
 *
 */

void ACDXFPFactory::constructPostProcessingTests()
{
    if (fps->alphaTestEnabled() && (fps->alphaTestFunc() != ACDXFPState::ALWAYS))
        constructAlphaTest();
}

void ACDXFPFactory::constructAlphaTest()
{
    string text = "# ------------ Alpha Test ----------------\n";

    
    std::vector<ACDX_STORED_FP_ITEM_ID> vState;
    vState.push_back(ACDX_ALPHA_TEST_REF_VALUE);

    ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_LOCAL,
                                                        42, vState, 
                                                        new AlphaRefValueFunction);
    tlshader->addConstantBinding(cb);

    //tlshader->addLocal(new AlphaTestReferenceValue);

    switch(fps->alphaTestFunc())
    {
        case ACDXFPState::NEVER:
            text += "# Alpha Test Function: NEVER\n"
                    "KIL {-1,-1,-1,-1};\n";
            break;

        case ACDXFPState::LESS:
            text += "# Alpha Test Function: LESS\n"
                    "SGE testReg, finalColor.a, program.local[42];\n"
                    "KIL -testReg;\n"
                    "MOV result.color, finalColor;\n";
            break;

        case ACDXFPState::LEQUAL:
            text += "# Alpha Test Function: LEQUAL\n"
                    "SUB testReg, program.local[42], finalColor.a;\n"
                    "KIL testReg;\n"
                    "MOV result.color, finalColor;\n";
            break;

        case ACDXFPState::EQUAL:
            text += "# Alpha Test Function: EQUAL\n"
                    "SUB testReg, program.local[42], finalColor.a;\n"
                    "ABS testReg, testReg;\n"
                    "KIL -testReg;\n"
                    "MOV result.color, finalColor;\n";
            break;

        case ACDXFPState::GEQUAL:
            text += "# Alpha Test Function: GEQUAL\n"
                    "SUB testReg, finalColor.a, program.local[42];\n"
                    "KIL testReg;\n"
                    "MOV result.color, finalColor;\n";

            break;

        case ACDXFPState::GREATER:
            text += "# Alpha Test Function: GREATER\n"
                    "SGE testReg, -finalColor.a, -program.local[42];\n"
                    "KIL -testReg;\n"
                    "MOV result.color, finalColor;\n";

            break;

        case ACDXFPState::NOTEQUAL:
            text += "# Alpha Test Function: NOTEQUAL\n"
                    "SGE testReg.x, finalColor.a, program.local[42];\n"
                    "SGE testReg.y, -finalColor.a, -program.local[42];\n"
                    "MUL testReg, testReg.x, testReg.y;\n"
                    "KIL -testReg;\n"
                    "MOV result.color, finalColor;\n";

            break;

        default:
            panic("ACDXFPFactory","constructAlphaTest()","Unexpected alpha test function");
    }

    text += "# ------------ End Alpha Test ----------------\n";

    code.append(text);
}

void ACDXFPFactory::computeTextureApplication(ACDXFPState::TextureUnit textureUnit, string inputFragmentColorTemp, string outputFragmentColorTemp, string primaryColorRegister)
{
    string comments;

    string text;

    TextureFunctionComputing::CombineFunction RGBOp, ALPHAOp;

    string RGBDestRegister, ALPHADestRegister;

    struct operandRegister
    {
        string name;
        string swizzle;
    };

    /* Fourth source extended for "NV_texture_env_combine4" extension support */
    operandRegister RGBOperandRegister[4];
    operandRegister ALPHAOperandRegister[4];

    unsigned int RGBScaleFactor = 1;
    unsigned int ALPHAScaleFactor = 1;

    bool clamp = false;

    stringstream ss;
    ss << textureUnit.unitId;
    string unit = ss.str();


    /* Texture sampling */

    string target;

    switch(textureUnit.activeTarget)
    {
        case ACDXFPState::TextureUnit::TEXTURE_1D:  target = string("1D"); break;
        case ACDXFPState::TextureUnit::TEXTURE_2D:  target = string("2D"); break;
        case ACDXFPState::TextureUnit::TEXTURE_3D:  target = string("3D"); break;
        case ACDXFPState::TextureUnit::CUBE_MAP:    target = string("CUBE"); break;
    }

    code.append(TextureSampling(textureUnit.unitId,target));

    /* Texture Function implementation */
    
    bool isCombine4NV = false; /* For "NV_texture_env_combine4" extension support */
    
    switch(textureUnit.function)
    {
        case ACDXFPState::TextureUnit::REPLACE:

            comments += "# -- Modulation texture function: REPLACE\n";

            switch(textureUnit.format)
            {
                case ACDXFPState::TextureUnit::ALPHA:

                    comments += "# -- Base Internal Format: ALPHA\n";

                    // MOV output.rgb, input;
                    // MOV output.a, sample;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE:

                    comments += "# -- Base Internal Format: LUMINANCE\n";

                    // MOV output.rgb, sample.rrrr;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE_ALPHA:

                    comments += "# -- Base Internal Format: LUMINANCE_ALPHA\n";

                    // MOV output.rgba, sample.rrra;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rrra";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::DEPTH:

                    comments += "# -- Base Internal Format: DEPTH\n";

                    panic("ACDXFPFactory","computeTextureApplication","Depth component Base Internal Format not supported yet");

                    break;

                case ACDXFPState::TextureUnit::INTENSITY:

                    comments += "# -- Base Internal Format: INTENSITY\n";

                    // MOV output.rgba, sample.rrrr;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[0].swizzle = "rrrr";

                    break;

                case ACDXFPState::TextureUnit::RGB:

                    comments += "# -- Base Internal Format: RGB\n";

                    // MOV output.rgb, sample;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::RGBA:

                    comments += "# -- Base Internal Format: RGBA\n";

                    // MOV output.rgba, sample;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                default:
                    panic("ACDXFPFactory","computeTextureApplication","Unexpected Base Internal Format");
            }
            break;

        case ACDXFPState::TextureUnit::MODULATE:

            comments += "# -- Modulation texture function: MODULATE\n";

            switch(textureUnit.format)
            {
                case ACDXFPState::TextureUnit::ALPHA:

                    comments += "# -- Base Internal Format: ALPHA\n";

                    // MOV output.rgb, input;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE:

                    comments += "# -- Base Internal Format: LUMINANCE\n";

                    // MUL output.rgb, input, sample.rrrr;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::MODULATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE_ALPHA:

                    comments += "# -- Base Internal Format: LUMINANCE_ALPHA\n";

                    // MUL output.rgba, input, sample.rrra;

                    RGBOp = TextureFunctionComputing::MODULATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rrra";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rrra";

                    break;

                case ACDXFPState::TextureUnit::DEPTH:

                    comments += "# -- Base Internal Format: DEPTH\n";

                    panic("ACDXFPFactory","computeTextureApplication","Depth component Base Internal Format not supported yet");

                    break;

                case ACDXFPState::TextureUnit::INTENSITY:

                    comments += "# -- Base Internal Format: INTENSITY\n";

                    // MUL output.rgba, input, sample.rrrr;

                    RGBOp = TextureFunctionComputing::MODULATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rrrr";

                    break;

                case ACDXFPState::TextureUnit::RGB:

                    comments += "# -- Base Internal Format: RGB\n";

                    // MUL output.rgb, input, sample;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::MODULATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::RGBA:

                    comments += "# -- Base Internal Format: RGBA\n";

                    // MUL output.rgba, input, sample;

                    RGBOp = TextureFunctionComputing::MODULATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                default:
                    panic("ACDXFPFactory","computeTextureApplication","Unexpected Base Internal Format");
            }
            break;

        case ACDXFPState::TextureUnit::DECAL:

            comments += "# -- Modulation texture function: DECAL\n";

            switch(textureUnit.format)
            {
                case ACDXFPState::TextureUnit::ALPHA:

                    comments += "# -- Base Internal Format: ALPHA \n";
                    comments += "# -- Warning: undefined result specified\n";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE:

                    comments += "# -- Base Internal Format: LUMINANCE \n";
                    comments += "# -- Warning: undefined result specified\n";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE_ALPHA:

                    comments += "# -- Base Internal Format: LUMINANCE_ALPHA \n";
                    comments += "# -- Warning: undefined result specified\n";

                    break;

                case ACDXFPState::TextureUnit::DEPTH:

                    comments += "# -- Base Internal Format: DEPTH\n";
                    comments += "# -- Warning: undefined result specified\n";

                    panic("ACDXFPFactory","computeTextureApplication","Depth component Base Internal Format not supported yet");

                    break;

                case ACDXFPState::TextureUnit::INTENSITY:

                    comments += "# -- Base Internal Format: INTENSITY \n";
                    comments += "# -- Warning: undefined result specified\n";

                    break;

                case ACDXFPState::TextureUnit::RGB:

                    comments += "# -- Base Internal Format: RGB\n";

                    // MOV output.rgb, sample;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::RGBA:

                    comments += "# -- Base Internal Format: RGBA\n";

                    // INTERPOLATE output.rgb, sample, input, sample.aaaa,;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = inputFragmentColorTemp;
                    RGBOperandRegister[1].swizzle = "rgba";
                    RGBOperandRegister[2].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[2].swizzle = "aaaa";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                default:
                    panic("ACDXFPFactory","computeTextureApplication","Unexpected Base Internal Format");
            }
            break;

        case ACDXFPState::TextureUnit::BLEND:

            comments += "# -- Modulation texture function: BLEND\n";

            switch(textureUnit.format)
            {
                case ACDXFPState::TextureUnit::ALPHA:

                    comments += "# -- Base Internal Format: ALPHA\n";

                    // MOV output.rgb, input;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE:

                    comments += "# -- Base Internal Format: LUMINANCE\n";

                    // INTERPOLATE output.rgb, state.texenv[i].color, input, sample.rrrr;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTUREENVIRONMENTCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = inputFragmentColorTemp;
                    RGBOperandRegister[1].swizzle = "rgba";
                    RGBOperandRegister[2].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[2].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE_ALPHA:

                    comments += "# -- Base Internal Format: LUMINANCE_ALPHA\n";

                    // INTERPOLATE output.rgb, state.texenv[i].color, input, sample.rrrr;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTUREENVIRONMENTCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = inputFragmentColorTemp;
                    RGBOperandRegister[1].swizzle = "rgba";
                    RGBOperandRegister[2].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[2].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::DEPTH:

                    comments += "# -- Base Internal Format: DEPTH\n";

                    panic("ACDXFPFactory","computeTextureApplication","Depth component Base Internal Format not supported yet");

                    break;

                case ACDXFPState::TextureUnit::INTENSITY:

                    comments += "# -- Base Internal Format: INTENSITY\n";

                    // INTERPOLATE output.rgba, state.texenv[i].color, input, sample.rrrr;

                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTUREENVIRONMENTCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = inputFragmentColorTemp;
                    RGBOperandRegister[1].swizzle = "rgba";
                    RGBOperandRegister[2].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[2].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::INTERPOLATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = TEXTUREENVIRONMENTCOLOR(unit);
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[1].swizzle = "rgba";
                    ALPHAOperandRegister[2].name =  TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[2].swizzle = "rrrr";

                    break;

                case ACDXFPState::TextureUnit::RGB:

                    comments += "# -- Base Internal Format: RGB\n";

                    // INTERPOLATE output.rgb, state.texenv[i].color, input, sample;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTUREENVIRONMENTCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = inputFragmentColorTemp;
                    RGBOperandRegister[1].swizzle = "rgba";
                    RGBOperandRegister[2].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[2].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::RGBA:

                    comments += "# -- Base Internal Format: RGBA\n";

                    // INTERPOLATE output.rgb, state.texenv[i].color, input, sample;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = TEXTUREENVIRONMENTCOLOR(unit);
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = inputFragmentColorTemp;
                    RGBOperandRegister[1].swizzle = "rgba";
                    RGBOperandRegister[2].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[2].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                default:
                    panic("ACDXFPFactory","computeTextureApplication","Unexpected Base Internal Format");
            }
            break;

        case ACDXFPState::TextureUnit::ADD:

            comments += "# -- Modulation texture function: ADD\n";

            switch(textureUnit.format)
            {
                case ACDXFPState::TextureUnit::ALPHA:

                    comments += "# -- Base Internal Format: ALPHA\n";

                    // MOV output.rgb, input;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::REPLACE;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE:

                    comments += "# -- Base Internal Format: LUMINANCE\n";

                    // ADD output.rgb, input, sample.rrrr;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::ADD;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::LUMINANCE_ALPHA:

                    comments += "# -- Base Internal Format: LUMINANCE_ALPHA\n";

                    // ADD output.rgb, input, sample.rrrr;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::ADD;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::DEPTH:

                    comments += "# -- Base Internal Format: DEPTH\n";

                    panic("ACDXFPFactory","computeTextureApplication","Depth component Base Internal Format not supported yet");

                    break;

                case ACDXFPState::TextureUnit::INTENSITY:

                    comments += "# -- Base Internal Format: INTENSITY\n";

                    // ADD output, input, sample.rrrr;

                    RGBOp = TextureFunctionComputing::ADD;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rrrr";

                    ALPHAOp = TextureFunctionComputing::ADD;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rrrr";

                    break;

                case ACDXFPState::TextureUnit::RGB:

                    comments += "# -- Base Internal Format: RGB\n";

                    // ADD output.rgb, input, sample;
                    // MOV output.a, input;

                    RGBOp = TextureFunctionComputing::ADD;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";

                    break;

                case ACDXFPState::TextureUnit::RGBA:

                    comments += "# -- Base Internal Format: RGBA\n";

                    // ADD output.rgb, input, sample;
                    // MUL output.a, input, sample;

                    RGBOp = TextureFunctionComputing::ADD;
                    RGBDestRegister = outputFragmentColorTemp;
                    RGBOperandRegister[0].name = inputFragmentColorTemp;
                    RGBOperandRegister[0].swizzle = "rgba";
                    RGBOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    RGBOperandRegister[1].swizzle = "rgba";

                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    ALPHADestRegister = outputFragmentColorTemp;
                    ALPHAOperandRegister[0].name = inputFragmentColorTemp;
                    ALPHAOperandRegister[0].swizzle = "rgba";
                    ALPHAOperandRegister[1].name = TEXTURELOOKUPCOLOR(unit);
                    ALPHAOperandRegister[1].swizzle = "rgba";

                    break;

                default:
                    panic("ACDXFPFactory","computeTextureApplication","Unexpected Base Internal Format");
            }
            break;

        case ACDXFPState::TextureUnit::COMBINE4_NV:
        
            isCombine4NV = true;
            /* Fall into conventional COMBINE branch */
            
        case ACDXFPState::TextureUnit::COMBINE:
        {
            unsigned int combineRGBOperands, combineALPHAOperands;

            clamp = true; /* Final clamp needed for combine texture functions */

            if (isCombine4NV)
                comments += "# -- Modulation texture function: COMBINE4_NV\n";
            else 
                comments += "# -- Modulation texture function: COMBINE\n";

            switch(textureUnit.combineMode.combineRGBFunction)
            {
                case ACDXFPState::TextureUnit::CombineMode::REPLACE:

                    comments += "# -- Combine RGB Function: REPLACE\n";
                    RGBOp = TextureFunctionComputing::REPLACE;
                    combineRGBOperands = 1;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::MODULATE:

                    comments += "# -- Combine RGB Function: MODULATE\n";
                    RGBOp = TextureFunctionComputing::MODULATE;
                    combineRGBOperands = 2;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::ADD:

                    /* For "NV_texture_env_combine4" extension support */
                    if (!isCombine4NV)
                    {
                        comments += "# -- Combine RGB Function: ADD\n";
                        RGBOp = TextureFunctionComputing::ADD;
                        combineRGBOperands = 2;
                    }
                    else // isCombine4NV
                    {
                        comments += "# -- Combine4NV RGB Function: ADD\n";
                        RGBOp = TextureFunctionComputing::ADD_4;
                        combineRGBOperands = 4;
                    }
                    break;

                case ACDXFPState::TextureUnit::CombineMode::ADD_SIGNED:

                    /* For "NV_texture_env_combine4" extension support */
                    if (!isCombine4NV)
                    {
                        comments += "# -- Combine RGB Function: ADD_SIGNED\n";
                        RGBOp = TextureFunctionComputing::ADD_SIGNED;
                        combineRGBOperands = 2;
                    }
                    else // isCombine4NV
                    {
                        comments += "# -- Combine4NV RGB Function: ADD_SIGNED\n";
                        RGBOp = TextureFunctionComputing::ADD_4_SIGNED;
                        combineRGBOperands = 4;
                    }
                    break;

                case ACDXFPState::TextureUnit::CombineMode::INTERPOLATE:

                    comments += "# -- Combine RGB Function: INTERPOLATE\n";
                    RGBOp = TextureFunctionComputing::INTERPOLATE;
                    combineRGBOperands = 3;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::SUBTRACT:

                    comments += "# -- Combine RGB Function: SUBTRACT\n";
                    RGBOp = TextureFunctionComputing::SUBTRACT;
                    combineRGBOperands = 2;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::DOT3_RGB:

                    comments += "# -- Combine RGB Function: DOT3_RGB\n";
                    RGBOp = TextureFunctionComputing::DOT3_RGB;
                    combineRGBOperands = 2;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::DOT3_RGBA:

                    comments += "# -- Combine RGB Function: DOT3_RGBA\n";
                    RGBOp = TextureFunctionComputing::DOT3_RGBA;
                    combineRGBOperands = 2;
                    break;

                /* Next ones for "ATI_texture_env_combine3" extension support */
                case ACDXFPState::TextureUnit::CombineMode::MODULATE_ADD_ATI:

                    comments += "# -- Combine RGB Function: MODULATE_ADD_ATI (\"ATI_texture_env_combine\" ext.)\n";
                    RGBOp = TextureFunctionComputing::MODULATE_ADD;
                    combineRGBOperands = 3;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI:

                    comments += "# -- Combine RGB Function: MODULATE_SIGNED_ADD_ATI (\"ATI_texture_env_combine\" ext.)\n";
                    RGBOp = TextureFunctionComputing::MODULATE_SIGNED_ADD;
                    combineRGBOperands = 3;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI:

                    comments += "# -- Combine RGB Function: MODULATE_SUBTRACT_ATI (\"ATI_texture_env_combine\" ext.)\n";
                    RGBOp = TextureFunctionComputing::MODULATE_SUBTRACT;
                    combineRGBOperands = 3;
                    break;

                default:

                    panic("ACDXFPFactory","computeTextureApplication","Unexpected combine RGB function");
            }

            switch(textureUnit.combineMode.combineALPHAFunction)
            {
                case ACDXFPState::TextureUnit::CombineMode::REPLACE:

                    comments += "# -- Combine ALPHA Function: REPLACE\n";
                    ALPHAOp = TextureFunctionComputing::REPLACE;
                    combineALPHAOperands = 1;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::MODULATE:

                    comments += "# -- Combine ALPHA Function: MODULATE\n";
                    ALPHAOp = TextureFunctionComputing::MODULATE;
                    combineALPHAOperands = 2;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::ADD:

                    /* For "NV_texture_env_combine4" extension support */
                    if (!isCombine4NV)
                    {
                        comments += "# -- Combine ALPHA Function: ADD\n";
                        ALPHAOp = TextureFunctionComputing::ADD;
                        combineALPHAOperands = 2;
                    }
                    else // isCombine4NV
                    {
                        comments += "# -- Combine4NV ALPHA Function: ADD\n";
                        ALPHAOp = TextureFunctionComputing::ADD_4;
                        combineALPHAOperands = 4;
                    }
                    break;
                    
                case ACDXFPState::TextureUnit::CombineMode::ADD_SIGNED:

                    /* For "NV_texture_env_combine4" extension support */
                    if (!isCombine4NV)
                    {
                        comments += "# -- Combine ALPHA Function: ADD_SIGNED\n";
                        ALPHAOp = TextureFunctionComputing::ADD_SIGNED;
                        combineALPHAOperands = 2;
                    }
                    else // isCombine4NV
                    {
                        comments += "# -- Combine4NV ALPHA Function: ADD_SIGNED\n";
                        ALPHAOp = TextureFunctionComputing::ADD_4_SIGNED;
                        combineALPHAOperands = 4;
                    }
                    break;
                    
                case ACDXFPState::TextureUnit::CombineMode::INTERPOLATE:

                    comments += "# -- Combine ALPHA Function: INTERPOLATE\n";
                    ALPHAOp = TextureFunctionComputing::INTERPOLATE;
                    combineALPHAOperands = 3;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::SUBTRACT:

                    comments += "# -- Combine ALPHA Function: SUBTRACT\n";
                    ALPHAOp = TextureFunctionComputing::SUBTRACT;
                    combineALPHAOperands = 2;
                    break;

                /* Next ones for "ATI_texture_env_combine3" extension support */
                case ACDXFPState::TextureUnit::CombineMode::MODULATE_ADD_ATI:

                    comments += "# -- Combine ALPHA Function: MODULATE_ADD_ATI (\"ATI_texture_env_combine\" ext.)\n";
                    ALPHAOp = TextureFunctionComputing::MODULATE_ADD;
                    combineALPHAOperands = 3;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI:

                    comments += "# -- Combine ALPHA Function: MODULATE_SIGNED_ADD_ATI (\"ATI_texture_env_combine\" ext.)\n";
                    ALPHAOp = TextureFunctionComputing::MODULATE_SIGNED_ADD;
                    combineALPHAOperands = 3;
                    break;

                case ACDXFPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI:

                    comments += "# -- Combine ALPHA Function: MODULATE_SUBTRACT_ATI (\"ATI_texture_env_combine\" ext.)\n";
                    ALPHAOp = TextureFunctionComputing::MODULATE_SUBTRACT;
                    combineALPHAOperands = 3;
                    break;

                default:

                    panic("ACDXFPFactory","computeTextureApplication","Unexpected combine ALPHA function");
            }


            RGBDestRegister = outputFragmentColorTemp;
            ALPHADestRegister = outputFragmentColorTemp;

            unsigned int i;

            for (i=0; i<combineRGBOperands; i++)
            {
                stringstream ss2;
                ss2 << i;
                string numOperand = ss2.str();

                switch(textureUnit.combineMode.srcRGB[i])
                {
                    case ACDXFPState::TextureUnit::CombineMode::TEXTURE:

                        comments += "# -- Combine RGB Source" + numOperand + " : TEXTURE\n";
                        RGBOperandRegister[i].name = TEXTURELOOKUPCOLOR(unit);
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::TEXTUREn:
                    {
                        comments += "# -- Combine RGB Source" + numOperand + " : TEXTUREn\n";
                        stringstream ss;
                        ss << textureUnit.combineMode.srcRGB_texCrossbarReference[i];
                        RGBOperandRegister[i].name = TEXTURELOOKUPCOLOR(ss.str());
                        break;
                    }
                    case ACDXFPState::TextureUnit::CombineMode::CONSTANT:

                        comments += "# -- Combine RGB Source" + numOperand + " : CONSTANT\n";
                        RGBOperandRegister[i].name = TEXTUREENVIRONMENTCOLOR(unit);
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::PRIMARY_COLOR:

                        comments += "# -- Combine RGB Source" + numOperand + " : PRIMARY_COLOR\n";
                        RGBOperandRegister[i].name = primaryColorRegister;
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::PREVIOUS:

                        comments += "# -- Combine RGB Source" + numOperand + " : PREVIOUS\n";
                        RGBOperandRegister[i].name = inputFragmentColorTemp;
                        break;

                    /* For "NV_texture_env_combine4" and "ATI_texture_env_combine3" extensions support */
                    case ACDXFPState::TextureUnit::CombineMode::ZERO:

                        comments += "# -- Combine RGB Source" + numOperand + " : ZERO (\"NV_texture_env_combine4\" and \"ATI_texture_env_combine3\" exts.)\n";
                        RGBOperandRegister[i].name = ZERO_CONSTANT;
                        break;

                    /* For "ATI_texture_env_combine3" extension support */
                    case ACDXFPState::TextureUnit::CombineMode::ONE:

                        comments += "# -- Combine RGB Source" + numOperand + " : ONE (\"ATI_texture_env_combine3\" ext.)\n";
                        RGBOperandRegister[i].name = ONE_CONSTANT;
                        break;


                    default:
                        panic("ACDXFPFactory","computeTextureApplication","Unexpected combine RGB source");
                }

                switch(textureUnit.combineMode.operandRGB[i])
                {
                    case ACDXFPState::TextureUnit::CombineMode::SRC_COLOR:

                        comments += "# -- Combine RGB Operand" + numOperand + " : SRC_COLOR\n";
                        RGBOperandRegister[i].swizzle = "rgba";
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR:

                        /* Detect special cases as 1 - ZERO or 1 - 1
                         * that are equivalent to Source = ONE and Source = ZERO respectively 
                         */
                        if (textureUnit.combineMode.srcRGB[i] == ACDXFPState::TextureUnit::CombineMode::ZERO)
                        {
                            RGBOperandRegister[i].name = ONE_CONSTANT;
                            RGBOperandRegister[i].swizzle = "rgba";
                        }
                        else if (textureUnit.combineMode.srcRGB[i] == ACDXFPState::TextureUnit::CombineMode::ONE)
                        {
                            RGBOperandRegister[i].name = ZERO_CONSTANT;
                            RGBOperandRegister[i].swizzle = "rgba";
                        }
                        else
                        {
                            RGBOperandRegister[i].swizzle = "!rgba";
                        }
                        comments += "# -- Combine RGB Operand" + numOperand + " : ONE_MINUS_SRC_COLOR\n";
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::SRC_ALPHA:

                        comments += "# -- Combine RGB Operand" + numOperand + " : SRC_ALPHA\n";
                        RGBOperandRegister[i].swizzle = "aaaa";
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA:

                        /* Detect special cases as 1 - ZERO or 1 - 1
                         * that are equivalent to Source = ONE and Source = ZERO respectively 
                         */
                        if (textureUnit.combineMode.srcRGB[i] == ACDXFPState::TextureUnit::CombineMode::ZERO)
                        {
                            RGBOperandRegister[i].name = ONE_CONSTANT;
                            RGBOperandRegister[i].swizzle = "aaaa";
                        }
                        else if (textureUnit.combineMode.srcRGB[i] == ACDXFPState::TextureUnit::CombineMode::ONE)
                        {
                            RGBOperandRegister[i].name = ZERO_CONSTANT;
                            RGBOperandRegister[i].swizzle = "aaaa";
                        }
                        else
                        {
                            RGBOperandRegister[i].swizzle = "!aaaa";
                        }
                        comments += "# -- Combine RGB Operand" + numOperand + " : ONE_MINUS_SRC_ALPHA\n";
                        break;

                    default:
                        panic("ACDXFPFactory","computeTextureApplication","Unexpected combine RGB operand");
                }
            }

            for (i=0; i<combineALPHAOperands; i++)
            {
                stringstream ss2;
                ss2 << i;
                string numOperand = ss2.str();

                switch(textureUnit.combineMode.srcALPHA[i])
                {
                    case ACDXFPState::TextureUnit::CombineMode::TEXTURE:

                        comments += "# -- Combine ALPHA Source" + numOperand + " : TEXTURE\n";
                        ALPHAOperandRegister[i].name = TEXTURELOOKUPCOLOR(unit);
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::TEXTUREn:
                    {
                        comments += "# -- Combine ALPHA Source" + numOperand + " : TEXTUREn\n";
                        stringstream ss;
                        ss << textureUnit.combineMode.srcALPHA_texCrossbarReference[i];
                        ALPHAOperandRegister[i].name = TEXTURELOOKUPCOLOR(ss.str());
                        break;
                    }
                    case ACDXFPState::TextureUnit::CombineMode::CONSTANT:

                        comments += "# -- Combine ALPHA Source" + numOperand + " : CONSTANT\n";
                        ALPHAOperandRegister[i].name = TEXTUREENVIRONMENTCOLOR(unit);
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::PRIMARY_COLOR:

                        comments += "# -- Combine ALPHA Source" + numOperand + " : PRIMARY_COLOR\n";
                        ALPHAOperandRegister[i].name = primaryColorRegister;
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::PREVIOUS:

                        comments += "# -- Combine ALPHA Source" + numOperand + " : PREVIOUS\n";
                        ALPHAOperandRegister[i].name = inputFragmentColorTemp;
                        break;

                    /* For "NV_texture_env_combine4" and "ATI_texture_env_combine3" extensions support */
                    case ACDXFPState::TextureUnit::CombineMode::ZERO:

                        comments += "# -- Combine ALPHA Source" + numOperand + " : ZERO (\"NV_texture_env_combine4\" and \"ATI_texture_env_combine3\" exts.)\n";
                        ALPHAOperandRegister[i].name = ZERO_CONSTANT;
                        break;

                    /* For "ATI_texture_env_combine3" extension support */
                    case ACDXFPState::TextureUnit::CombineMode::ONE:

                        comments += "# -- Combine ALPHA Source" + numOperand + " : ONE (\"ATI_texture_env_combine3\" ext.)\n";
                        ALPHAOperandRegister[i].name = ONE_CONSTANT;
                        break;

                    default:
                        panic("ACDXFPFactory","computeTextureApplication","Unexpected combine ALPHA source");
                }

                switch(textureUnit.combineMode.operandALPHA[i])
                {
                    case ACDXFPState::TextureUnit::CombineMode::SRC_COLOR:

                        comments += "# -- Combine ALPHA Operand" + numOperand + " : SRC_COLOR\n";
                        ALPHAOperandRegister[i].swizzle = "rgba";
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR:

                        /* Detect special cases as 1 - ZERO or 1 - 1
                         * that are equivalent to Source = ONE and Source = ZERO respectively 
                         */
                        if (textureUnit.combineMode.srcALPHA[i] == ACDXFPState::TextureUnit::CombineMode::ZERO)
                        {
                            ALPHAOperandRegister[i].name = ONE_CONSTANT;
                            ALPHAOperandRegister[i].swizzle = "rgba";
                        }
                        else if (textureUnit.combineMode.srcALPHA[i] == ACDXFPState::TextureUnit::CombineMode::ONE)
                        {
                            ALPHAOperandRegister[i].name = ZERO_CONSTANT;
                            ALPHAOperandRegister[i].swizzle = "rgba";
                        }
                        else
                        {
                            ALPHAOperandRegister[i].swizzle = "!rgba";
                        }
                        comments += "# -- Combine ALPHA Operand" + numOperand + " : ONE_MINUS_SRC_COLOR\n";
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::SRC_ALPHA:

                        comments += "# -- Combine ALPHA Operand" + numOperand + " : SRC_ALPHA\n";
                        ALPHAOperandRegister[i].swizzle = "aaaa";
                        break;

                    case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA:

                        /* Detect special cases as 1 - ZERO or 1 - 1
                         * that are equivalent to Source = ONE and Source = ZERO respectively 
                         */
                        if (textureUnit.combineMode.srcALPHA[i] == ACDXFPState::TextureUnit::CombineMode::ZERO)
                        {
                            ALPHAOperandRegister[i].name = ONE_CONSTANT;
                            ALPHAOperandRegister[i].swizzle = "aaaa";
                        }
                        else if (textureUnit.combineMode.srcALPHA[i] == ACDXFPState::TextureUnit::CombineMode::ONE)
                        {
                            ALPHAOperandRegister[i].name = ZERO_CONSTANT;
                            ALPHAOperandRegister[i].swizzle = "aaaa";
                        }
                        else
                        {
                            ALPHAOperandRegister[i].swizzle = "!aaaa";
                        }
                        comments += "# -- Combine ALPHA Operand" + numOperand + " : ONE_MINUS_SRC_ALPHA\n";
                        break;

                    default:
                        panic("ACDXFPFactory","computeTextureApplication","Unexpected combine ALPHA operand");
                }

            }

            RGBScaleFactor = textureUnit.combineMode.rgbScale;
            ALPHAScaleFactor = textureUnit.combineMode.alphaScale;

            stringstream ss, ss2;
            ss << RGBScaleFactor;
            ss2 << ALPHAScaleFactor;

            comments += "# -- Combine RGB Scale Factor " + ss.str() + "\n";
            comments += "# -- Combine ALPHA Scale Factor " + ss2.str() + "\n";

            break;
        }
        default:
            panic("ACDXFPFactory","computeTextureApplication","Unexpected Texture Function");

    }

    code.append(comments);
    code.append(TextureFunctionComputing(textureUnit.unitId, RGBOp, ALPHAOp,
                                         RGBDestRegister, ALPHADestRegister,
                                         RGBOperandRegister[0].name, ALPHAOperandRegister[0].name,
                                         RGBOperandRegister[1].name, ALPHAOperandRegister[1].name,
                                         RGBOperandRegister[2].name, ALPHAOperandRegister[2].name,
                                         RGBOperandRegister[3].name, ALPHAOperandRegister[3].name,
                                         RGBOperandRegister[0].swizzle, ALPHAOperandRegister[0].swizzle,
                                         RGBOperandRegister[1].swizzle, ALPHAOperandRegister[1].swizzle,
                                         RGBOperandRegister[2].swizzle, ALPHAOperandRegister[2].swizzle,
                                         RGBOperandRegister[3].swizzle, ALPHAOperandRegister[3].swizzle,
                                         RGBScaleFactor, ALPHAScaleFactor,
                                         clamp));


}

void ACDXFPFactory::computeTextureApplicationTemporaries()
{
    string text;
    
    bool temporaryForRGBOperand[4];
    bool temporaryForALPHAOperand[4];
    
    int i;
    for (i=0; i<4; i++)
    {
        temporaryForRGBOperand[i] = false;
        temporaryForALPHAOperand[i] = false;
    }
    
    for (i=0; i<fps->maxTextureUnits(); i++)
    {
        if ( fps->isTextureUnitEnabled(i) )
        {
            ACDXFPState::TextureUnit textureUnit = fps->getTextureUnit(i);
            
            if (textureUnit.function == ACDXFPState::TextureUnit::COMBINE || 
                textureUnit.function == ACDXFPState::TextureUnit::COMBINE4_NV)
            {
                unsigned int combineRGBOperands, combineALPHAOperands;
                
                bool isCombine4NV = (textureUnit.function == ACDXFPState::TextureUnit::COMBINE4_NV);
                
                switch(textureUnit.combineMode.combineRGBFunction)
                {
                    case ACDXFPState::TextureUnit::CombineMode::REPLACE: combineRGBOperands = 1; break;
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE: combineRGBOperands = 2; break;
                    case ACDXFPState::TextureUnit::CombineMode::ADD: if (!isCombine4NV)
                                                                    combineRGBOperands = 2;
                                                                 else // isCombine4NV
                                                                 {
                                                                    combineRGBOperands = 4;
                                                                    temporaryForRGBOperand[2] = true;                                                                    
                                                                 }
                                                                 break;
                                                                 
                    case ACDXFPState::TextureUnit::CombineMode::ADD_SIGNED: if (!isCombine4NV)
                                                                        {
                                                                            combineRGBOperands = 2;
                                                                            temporaryForRGBOperand[1] = true;
                                                                        }
                                                                        else // isCombine4NV
                                                                        {
                                                                            combineRGBOperands = 4;
                                                                            temporaryForRGBOperand[2] = true;                                                                    
                                                                        }
                                                                        break;
                                                                        
                    case ACDXFPState::TextureUnit::CombineMode::INTERPOLATE: combineRGBOperands = 3; break;
                    case ACDXFPState::TextureUnit::CombineMode::SUBTRACT: combineRGBOperands = 2; break;
                    case ACDXFPState::TextureUnit::CombineMode::DOT3_RGB: combineRGBOperands = 2; 
                                                                      temporaryForRGBOperand[0] = true;
                                                                      temporaryForRGBOperand[1] = true;
                                                                      break;
                    case ACDXFPState::TextureUnit::CombineMode::DOT3_RGBA: combineRGBOperands = 2; 
                                                                       temporaryForRGBOperand[0] = true;
                                                                       temporaryForRGBOperand[1] = true;
                                                                       break;
                    
                    /* For "ATI_texture_env_combine3" extension support */
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE_ADD_ATI:        combineRGBOperands = 3; break;
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI: combineRGBOperands = 3; break;
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI:   combineRGBOperands = 3; break;
                    
                    default:
                        panic("ACDXFPFactory","computeTextureApplication","Unexpected combine RGB function");
                }
                
                switch(textureUnit.combineMode.combineALPHAFunction)
                {
                    case ACDXFPState::TextureUnit::CombineMode::REPLACE: combineALPHAOperands = 1; break;
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE: combineALPHAOperands = 2; break;
                    case ACDXFPState::TextureUnit::CombineMode::ADD: if (!isCombine4NV)
                                                                    combineALPHAOperands = 2;
                                                                 else // isCombine4NV
                                                                 {
                                                                    combineALPHAOperands = 4;
                                                                    temporaryForALPHAOperand[2] = true;                                                                    
                                                                 }
                                                                 break;
                                                                 
                    case ACDXFPState::TextureUnit::CombineMode::ADD_SIGNED: if (!isCombine4NV)
                                                                        {
                                                                            combineALPHAOperands = 2;
                                                                            temporaryForALPHAOperand[1] = true;
                                                                        }
                                                                        else // isCombine4NV
                                                                        {
                                                                            combineALPHAOperands = 4;
                                                                            temporaryForALPHAOperand[2] = true;                                                                    
                                                                        }
                                                                        break;
                                                                        
                    case ACDXFPState::TextureUnit::CombineMode::INTERPOLATE: combineALPHAOperands = 3; break;
                    case ACDXFPState::TextureUnit::CombineMode::SUBTRACT: combineALPHAOperands = 2; break;

                       /* For "ATI_texture_env_combine3" extension support */
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE_ADD_ATI:        combineALPHAOperands = 3; break;
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI: combineALPHAOperands = 3; break;
                    case ACDXFPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI:   combineALPHAOperands = 3; break;

                    default:
                        panic("ACDXFPFactory","computeTextureApplication","Unexpected combine ALPHA function");
                }
            
                unsigned int j;
                
                for (j=0; j<combineRGBOperands; j++)
                {
                    switch(textureUnit.combineMode.operandRGB[j])
                    {
                        case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR: 
                        case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA: 
                            temporaryForRGBOperand[j] = true; break;
                        default: break; // Not required temporaries
                    }
                }

                for (j=0; j<combineALPHAOperands; j++)
                {
                    switch(textureUnit.combineMode.operandALPHA[j])
                    {
                        case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR: 
                        case ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA: 
                            temporaryForALPHAOperand[j] = true; break;
                        default: break; // Not required temporaries
                    }
                }

            }
            else if (textureUnit.function == ACDXFPState::TextureUnit::COMBINE4_NV)
            {
            
            }
            // else not required temporaries
        }
    }
    
    for (i=0; i<4; i++)
    {
        stringstream ss;
        ss << i;
        if (temporaryForRGBOperand[i]) text += "TEMP operand" + ss.str() + "RGB;\n";
        if (temporaryForALPHAOperand[i]) text += "TEMP operand" + ss.str() + "ALPHA;\n";
    }
    
    code.append(text);
}

void ACDXFPFactory::constructTexturing()
{
    string text;
    stringstream ss;
    string t;

    bool firstTextureApplied = false;

    text = "# ------------ Texture application code ------------\n";

    code.append(text);

    computeTextureApplicationTemporaries();
    
    for (int i=0; i<fps->maxTextureUnits(); i++)
    {
        if ( fps->isTextureUnitEnabled(i) )
        {
            ACDXFPState::TextureUnit textureUnit = fps->getTextureUnit(i);

            if (!firstTextureApplied)
                computeTextureApplication(textureUnit, string("iColorPrim"), string("texColorSum"), string("iColorPrim"));
            else
                computeTextureApplication(textureUnit, string("texColorSum"), string("texColorSum"), string("iColorPrim"));

            firstTextureApplied = true;
        }
    }

    text = "# ---------- End Texture application ------------\n";

    code.append(text);
}

void ACDXFPFactory::computeColorSum()
{
    string text;
    string writeRegister;

    text = "# -------------- Color sum code --------------\n";

    code.append(text);

    if (fps->fogEnabled() ||
       (fps->alphaTestEnabled() && ((fps->alphaTestFunc() != ACDXFPState::ALWAYS) && (fps->alphaTestFunc() != ACDXFPState::NEVER))))
        writeRegister = "finalColor";
    else
        // Avoids unnecesary final copy to fragment output color
        writeRegister = "result.color";


    if (fps->anyTextureUnitEnabled())
    {
        if (fps->separateSpecular())
            text = "ADD " + writeRegister + ", texColorSum, iColorSec;\n";
        else
            text = "MOV " + writeRegister + ", texColorSum;\n";
    }
    else
    {
        if (fps->separateSpecular())
            text = "ADD  " + writeRegister + ", iColorPrim, iColorSec;\n";
        else
            text = "MOV  " + writeRegister + ", iColorPrim;\n";
    }

    text += "# -------------- End Color sum ---------------\n";

    code.append(text);
}

void ACDXFPFactory::computeFogApplication()
{
    code.append("# -------------- Fog application code --------------\n");
    computeFogFactor();
    computeFogCombination();
    code.append("# ------------ End fog application code -------------\n");
}

void ACDXFPFactory::computeDepthBypass()
{
    string text = "MOV result.depth.z, iPos.z;\n";
    code.append(text);
}

void ACDXFPFactory::computeFogFactor()
{
    if (fps->fogMode() == ACDXFPState::LINEAR)
    {
        std::vector<ACDX_STORED_FP_ITEM_ID> vState;
        vState.push_back(ACDX_FOG_LINEAR_START);
        vState.push_back(ACDX_FOG_LINEAR_END);

        ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_LOCAL, 
                                                            41, vState, 
                                                            new LinearFogParamsFunction);
        tlshader->addConstantBinding(cb);

        //tlshader->addLocal(new LinearFogParams);
        code.append(LinearFogFactorComputing());
    }
    else if (fps->fogMode() == ACDXFPState::EXP)
    {
        std::vector<ACDX_STORED_FP_ITEM_ID> vState;
        vState.push_back(ACDX_FOG_DENSITY);

        ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_LOCAL, 
                                                            41, vState, 
                                                            new ExponentialFogParamsFunction);

        /* CAUTION: The Binding Function is new() allocated and not deleted anyway */
        
        tlshader->addConstantBinding(cb);

        //tlshader->addLocal(new ExponentialFogParams);
        code.append(ExponentialFogFactorComputing());
    }
    else if (fps->fogMode() == ACDXFPState::EXP2)
    {
        std::vector<ACDX_STORED_FP_ITEM_ID> vState;
        vState.push_back(ACDX_FOG_DENSITY);

        ACDXConstantBinding* cb = ACDXCreateConstantBinding(ACDX_BINDING_TARGET_LOCAL, 
                                                            41, vState, 
                                                            new SecondOrderExponentialFogParamsFunction);
        
        /* CAUTION: The Binding Function is new() allocated and not deleted anyway */

        tlshader->addConstantBinding(cb);

        //tlshader->addLocal(new SecondOrderExponentialFogParams);
        code.append(SecondOrderExponentialFogFactorComputing());
    }
}

void ACDXFPFactory::computeFogCombination()
{
    string writeRegister;
    string text;

    if (fps->alphaTestEnabled())
    {
         text = "# Final fog combination\n"
               "LRP finalColor.rgb, fogFactor.x, finalColor, state.fog.color;\n";
    }
    else
    {
         text = "# Final fog combination\n"
               "LRP result.color.rgb, fogFactor.x, finalColor, state.fog.color;\n"
               "MOV result.color.a, finalColor;\n";
    }

    code.append(text);
}

ACDXTLShader* ACDXFPFactory::constructSeparatedAlphaTest(ACDXFPState& fps)
{
    ACDXFPFactory::fps = &fps;

    tlshader = new ACDXTLShader;

    code.clear();

    code.append("!!ARBfp1.0\n");
    code.append("TEMP finalColor, testReg;\n");
    
    if (fps.alphaTestEnabled() && (fps.alphaTestFunc() != ACDXFPState::ALWAYS))
        constructAlphaTest();
    else
        code.append("MOV result.color, finalColor;\n");
        
    code.append("END");

    //code.removeComments();

    tlshader->setCode(code.toString());

    return tlshader; // must be detroyed externally
}
