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


#include "IRPrinter.h"

#include <cstring>
#include <stdio.h>

using namespace std;

void IRPrinter::begin()
{
}

void IRPrinter::visit(VersionIRNode *n) {

    configure(n);
    *out << n->getOffset() << ": VER     ";
    string typeStr;
    if(n->getType() == VERTEX_SHADER)
        typeStr = "VS";
    else
        typeStr = "PS";
    *out << typeStr << " "
    << D3DSHADER_VERSION_MAJOR(n->getVersion()) << " "
    << D3DSHADER_VERSION_MINOR(n->getVersion()) << endl;
}

void IRPrinter::visit(IRNode *n) {}

void IRPrinter::visit(CommentIRNode *n)
{
    *out << "COMMENT " << endl;

    const vector<IRNode*> commentData = n->getChilds();
    
    char buffer[256];
    char auxBuffer[256];   
    unsigned char c[16];

    int lineElements = 0;
    
    for(u32bit child = 0; child < commentData.size(); child++)
    {
        if (lineElements == 0)
        {
            *out << endl;
            sprintf(buffer, "%06x : ", commentData[child]->getOffset());
        }
            
        c[lineElements + 0] = commentData[child]->getToken().bytes.byte0;
        c[lineElements + 1] = commentData[child]->getToken().bytes.byte1;
        c[lineElements + 2] = commentData[child]->getToken().bytes.byte2;
        c[lineElements + 3] = commentData[child]->getToken().bytes.byte3;
        
        strcpy(auxBuffer, buffer);
        sprintf(buffer, "%s%02x %02x %02x %02x ", auxBuffer, c[lineElements + 0], c[lineElements + 1], c[lineElements + 2], c[lineElements + 3]);

        //  Substitute control characters with spaces.
        c[lineElements + 0] = ((c[lineElements + 0] >= 0x20) && (c[lineElements + 0] < 0x7F)) ? c[lineElements + 0] : ' ';
        c[lineElements + 1] = ((c[lineElements + 1] >= 0x20) && (c[lineElements + 1] < 0x7F)) ? c[lineElements + 1] : ' ';
        c[lineElements + 2] = ((c[lineElements + 2] >= 0x20) && (c[lineElements + 2] < 0x7F)) ? c[lineElements + 2] : ' ';
        c[lineElements + 3] = ((c[lineElements + 3] >= 0x20) && (c[lineElements + 3] < 0x7F)) ? c[lineElements + 3] : ' ';
        
        //  Increment line elements.
        lineElements += 4;
        
        if (lineElements == 16)
        {
            lineElements = 0;
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s   \"", auxBuffer);
            for(u32bit e = 0; e < 16; e++)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%c", auxBuffer, c[e]);
            }                
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s\"", auxBuffer);

            *out << buffer;
        }
    }
    
    if (lineElements > 0)
    {
        for(u32bit e = 0; e < (16 - lineElements); e++)
        {
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s   ", auxBuffer);
        }
        
        strcpy(auxBuffer, buffer);
        sprintf(buffer, "%s   \"", auxBuffer);
        for(u32bit e = 0; e < lineElements; e++)
        {
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s%c", auxBuffer, c[e]);
        }
        strcpy(auxBuffer, buffer);
        sprintf(buffer, "%s\"", auxBuffer);
        
        *out << buffer;        
    }

    *out << endl;
}

void IRPrinter::visit(CommentDataIRNode *n)
{
}

void IRPrinter::visit(InstructionIRNode *n) {
    string compStr;

    *out << n->getOffset() << ": INSTR   "
        << (*opcodeNames.find(n->getOpcode())).second
        << " " << n->getChilds().size();
    if(n->isCoissue())
        *out << " C";
    if(n->isPredicate())
        *out << " P";
    *out << " " << getControlStr(n);
    *out << endl;
}

void IRPrinter::visit(DeclarationIRNode *n) {
        *out << n->getOffset() << ": DECL    "
        << (*opcodeNames.find(n->getOpcode())).second
        << " " << n->getChilds().size();
        *out << endl;
}

void IRPrinter::visit(DefinitionIRNode *n) {
        *out << n->getOffset() << ": DEF      "
        << (*opcodeNames.find(n->getOpcode())).second
        << " " << n->getChilds().size();
        *out << endl;
}

void IRPrinter::visit(ParameterIRNode *n) {
    *out << "     PARAM   "
        << n->getNRegister()
        << " " << (*paramRegisterTypeNames.find(n->getRegisterType())).second
        << endl;
}

void IRPrinter::visit(SourceParameterIRNode *n) {
    string modifier;
    if(n->getModifier() != D3DSPSM_NONE) {
        modifier = (*paramSourceModNames.find(n->getModifier())).second;
    }

    string regType;
    regType = (*paramRegisterTypeNames.find(n->getRegisterType())).second;

    *out << "     SOURCE  "
        << n->getNRegister()
        << " " <<  regType << " " << getSwizzleStr(n)
        << " " << modifier
        << endl;
}

void IRPrinter::visit(DestinationParameterIRNode *n) {

    string regType;
    regType = (*paramRegisterTypeNames.find(n->getRegisterType())).second;

    *out << "     DEST    "
        << n->getNRegister()
        << " " <<  regType << " " << getWriteMaskStr(n)
        << getShiftScaleStr(n)
        << getDestModifierStr(n) << endl;
}

void IRPrinter::visit(RelativeAddressingIRNode *n) {
	string regType;
	regType = (*paramRegisterTypeNames.find(n->getRegisterType())).second;
	*out << "     RELADDR " << regType << " " << n->getNRegister()
		<< " COMP " << n->getComponentIndex() << endl;
}

void IRPrinter::visit(SamplerInfoIRNode *n) {
    *out << "     SAMPINFO "
        << (*textureTypeNames.find(n->getTextureType())).second << endl;
}

void IRPrinter::visit(SemanticIRNode *n) {
    string usageStr;
    map<DWORD, string>::iterator itU = usageNames.find(n->getUsage());
    if(itU != usageNames.end())
        usageStr = (*itU).second;
    else
        usageStr = "????";
    *out << "     SEMANTIC "
        << n->getUsageIndex() << " " << usageStr << endl;

}

void IRPrinter::visit(BoolIRNode *n) {
    bool value = static_cast<bool>(n->getToken().dword);
    *out << "     " << value << endl;
}

void IRPrinter::visit(FloatIRNode *n) {
    D3DToken d3dtk = n->getToken();
    float value = *reinterpret_cast<float *>(&d3dtk);
    *out << "     " << value << endl;
}

void IRPrinter::visit(IntegerIRNode *n) {
    D3DToken d3dtk = n->getToken();
    int value = *reinterpret_cast<int *>(&d3dtk);
    *out << "     " << value << endl;
}



string IRPrinter::getSwizzleStr(SourceParameterIRNode *n) {
    string result;

    DWORD swizzleX = n->getSwizzle(0);
    DWORD swizzleY = n->getSwizzle(1);
    DWORD swizzleZ = n->getSwizzle(2);
    DWORD swizzleW = n->getSwizzle(3);

    // No swizzle
    if ((swizzleX == D3DVS_X_X) & (swizzleY == D3DVS_Y_Y) &
        (swizzleZ == D3DVS_Z_Z) & (swizzleW == D3DVS_W_W))
        return result;

    if     (D3DVS_X_X == swizzleX) result.append(1, 'x');
    else if(D3DVS_X_Y == swizzleX) result.append(1, 'y');
    else if(D3DVS_X_Z == swizzleX) result.append(1, 'z');
    else if(D3DVS_X_W == swizzleX) result.append(1, 'w');

    if     (D3DVS_Y_X == swizzleY) result.append(1, 'x');
    else if(D3DVS_Y_Y == swizzleY) result.append(1, 'y');
    else if(D3DVS_Y_Z == swizzleY) result.append(1, 'z');
    else if(D3DVS_Y_W == swizzleY) result.append(1, 'w');

    if     (D3DVS_Z_X == swizzleZ) result.append(1, 'x');
    else if(D3DVS_Z_Y == swizzleZ) result.append(1, 'y');
    else if(D3DVS_Z_Z == swizzleZ) result.append(1, 'z');
    else if(D3DVS_Z_W == swizzleZ) result.append(1, 'w');

    if     (D3DVS_W_X == swizzleW) result.append(1, 'x');
    else if(D3DVS_W_Y == swizzleW) result.append(1, 'y');
    else if(D3DVS_W_Z == swizzleW) result.append(1, 'z');
    else if(D3DVS_W_W == swizzleW) result.append(1, 'w');

    return result;
}



string IRPrinter::getWriteMaskStr(DestinationParameterIRNode *n) {
    string result;
    DWORD mask = n->getWriteMask();

    // No write mask
    if( mask == D3DSP_WRITEMASK_ALL )
        return result;

    if( mask & D3DSP_WRITEMASK_0 )
        result.append(1, 'x');
    if( mask & D3DSP_WRITEMASK_1 )
        result.append(1, 'y');
    if( mask & D3DSP_WRITEMASK_2 )
        result.append(1, 'z');
    if( mask & D3DSP_WRITEMASK_3 )
        result.append(1, 'w');

    return result;
}

string IRPrinter::getControlStr(InstructionIRNode *n) {
    string result;
    DWORD spec = n->getSpecificControls();
    D3DSHADER_COMPARISON comp = n->getComparison();
    map<D3DSHADER_COMPARISON, string>::iterator itC;


    switch(n->getOpcode()) {
        case D3DSIO_TEX:
            if((type == PIXEL_SHADER) & (version >= 0x0200)) {
                if(spec == D3DSI_TEXLD_PROJECT)
                    result = "PROJECT";
                else if(spec == D3DSI_TEXLD_BIAS)
                    result = "BIAS";
            }
            break;
        //case D3DSIO_BREAK:
        case D3DSIO_BREAKC:
        //case D3DSIO_BREAKP:
        case D3DSIO_CALL:
        case D3DSIO_CALLNZ:
        //case D3DSIO_ELSE:
        //case D3DSIO_ENDIF:
        //case D3DSIO_ENDREP:
        //case D3DSIO_IF:
        case D3DSIO_IFC:
        //case D3DSIO_LABEL:
        case D3DSIO_REP:
        case D3DSIO_SETP:
        case D3DSIO_LOOP:
        case D3DSIO_RET:
        //case D3DSIO_ENDLOOP:
            itC = comparisonNames.find(comp);
            if(itC != comparisonNames.end())
                result = (*itC).second;
            break;
    }
    return result;
}

string IRPrinter::getShiftScaleStr(DestinationParameterIRNode *n) {
    string result;
    DWORD shift = n->getShiftScale();
    if(shift == 0)
        return result;

    if(shift == 0x01000000)
        result = " 2X";
    else if(shift == 0x02000000)
        result = " 4X";
    else if(shift == 0x03000000)
        result = " 8X";
    else if(shift == 0x0F000000)
        result = " D2";
    else if(shift == 0x0E000000)
        result = " D4";
    else if(shift == 0x0D000000)
        result = " D8";

    return result;

}


void IRPrinter::configure(VersionIRNode *n) {

    type = n->getType();
    version = n->getVersion();

    if(n->getType() == VERTEX_SHADER)
        paramRegisterTypeNames[D3DSPR_ADDR] = "ADDR";
    else
        paramRegisterTypeNames[D3DSPR_TEXTURE] = "TEXTURE";

    if((n->getType() == VERTEX_SHADER) & (n->getVersion() >= 0x0300))
        paramRegisterTypeNames[D3DSPR_TEXCRDOUT] = "TEXCOORDOUT";
    else
        paramRegisterTypeNames[D3DSPR_OUTPUT] = "OUTPUT";
}

std::string IRPrinter::getDestModifierStr(DestinationParameterIRNode *n) {
    string result;
    DWORD modifier = n->getModifier();
    if (modifier == D3DSPDM_NONE)
        return result;

    if (modifier & D3DSPDM_SATURATE)
        result.append(" SAT");
    if (modifier & D3DSPDM_PARTIALPRECISION)
        result.append(" PP");
    if (modifier & D3DSPDM_MSAMPCENTROID)
        result.append(" CENTROID");

    return result;
}


IRPrinter::IRPrinter(ostream *o) : out(o) {

    opcodeNames[D3DSIO_NOP]             = "NOP";
    opcodeNames[D3DSIO_MOV]             = "MOV";
    opcodeNames[D3DSIO_ADD]             = "ADD";
    opcodeNames[D3DSIO_SUB]             = "SUB";
    opcodeNames[D3DSIO_MAD]             = "MAD";
    opcodeNames[D3DSIO_MUL]             = "MUL";
    opcodeNames[D3DSIO_RCP]             = "RCP";
    opcodeNames[D3DSIO_RSQ]             = "RSQ";
    opcodeNames[D3DSIO_DP3]             = "DP3";
    opcodeNames[D3DSIO_DP4]             = "DP4";
    opcodeNames[D3DSIO_MIN]             = "MIN";
    opcodeNames[D3DSIO_MAX]             = "MAX";
    opcodeNames[D3DSIO_SLT]             = "SLT";
    opcodeNames[D3DSIO_SGE]             = "SGE";
    opcodeNames[D3DSIO_EXP]             = "EXP";
    opcodeNames[D3DSIO_LOG]             = "LOG";
    opcodeNames[D3DSIO_LIT]             = "LIT";
    opcodeNames[D3DSIO_DST]             = "DST";
    opcodeNames[D3DSIO_LRP]             = "LRP";
    opcodeNames[D3DSIO_FRC]             = "FRC";
    opcodeNames[D3DSIO_M4x4]            = "M4x4";
    opcodeNames[D3DSIO_M4x3]            = "M4x3";
    opcodeNames[D3DSIO_M3x4]            = "M3x4";
    opcodeNames[D3DSIO_M3x3]            = "M3x3";
    opcodeNames[D3DSIO_M3x2]            = "M3x2";
    opcodeNames[D3DSIO_CALL]            = "CALL";
    opcodeNames[D3DSIO_CALLNZ]          = "CALLNZ";
    opcodeNames[D3DSIO_LOOP]            = "LOOP";
    opcodeNames[D3DSIO_RET]             = "RET";
    opcodeNames[D3DSIO_ENDLOOP]         = "ENDLOOP";
    opcodeNames[D3DSIO_LABEL]           = "LABEL";
    opcodeNames[D3DSIO_DCL]             = "DCL";
    opcodeNames[D3DSIO_POW]             = "POW";
    opcodeNames[D3DSIO_CRS]             = "CRS";
    opcodeNames[D3DSIO_SGN]             = "SGN";
    opcodeNames[D3DSIO_ABS]             = "ABS";
    opcodeNames[D3DSIO_NRM]             = "NRM";
    opcodeNames[D3DSIO_SINCOS]          = "SINCOS";
    opcodeNames[D3DSIO_REP]             = "REP";
    opcodeNames[D3DSIO_ENDREP]          = "ENDREP";
    opcodeNames[D3DSIO_IF]              = "IF";
    opcodeNames[D3DSIO_IFC]             = "IF";
    opcodeNames[D3DSIO_ELSE]            = "ELSE";
    opcodeNames[D3DSIO_ENDIF]           = "ENDIF";
    opcodeNames[D3DSIO_BREAK]           = "BREAK";
    opcodeNames[D3DSIO_BREAKC]          = "BREAKC";
    opcodeNames[D3DSIO_MOVA]            = "MOVA";
    opcodeNames[D3DSIO_DEFB]            = "DEFB";
    opcodeNames[D3DSIO_DEFI]            = "DEFI";
    opcodeNames[D3DSIO_TEXCOORD]        = "TEXCOORD";
    opcodeNames[D3DSIO_TEXKILL]         = "TEXKILL";
    opcodeNames[D3DSIO_TEX]             = "TEX";
    opcodeNames[D3DSIO_TEXBEM]          = "TEXBEM";
    opcodeNames[D3DSIO_TEXBEML]         = "TEXBEML";
    opcodeNames[D3DSIO_TEXREG2AR]       = "TEXREG2AR";
    opcodeNames[D3DSIO_TEXREG2GB]       = "TEXREG2GB";
    opcodeNames[D3DSIO_TEXM3x2PAD]      = "TEXM3x2PAD";
    opcodeNames[D3DSIO_TEXM3x2TEX]      = "TEXM3x2TEX";
    opcodeNames[D3DSIO_TEXM3x3PAD]      = "TEXM3x3PAD";
    opcodeNames[D3DSIO_TEXM3x3TEX]      = "TEXM3x3TEX";
    opcodeNames[D3DSIO_RESERVED0]       = "RESERVED0";
    opcodeNames[D3DSIO_TEXM3x3SPEC]     = "TEXM3x3SPEC";
    opcodeNames[D3DSIO_TEXM3x3VSPEC]    = "TEXM3x3VSPEC";
    opcodeNames[D3DSIO_EXPP]            = "EXPP";
    opcodeNames[D3DSIO_LOGP]            = "LOGP";
    opcodeNames[D3DSIO_CND]             = "CND";
    opcodeNames[D3DSIO_DEF]             = "DEF";
    opcodeNames[D3DSIO_TEXREG2RGB]      = "TEXREG2RGB";
    opcodeNames[D3DSIO_TEXDP3TEX]       = "TEXDP3TEX";
    opcodeNames[D3DSIO_TEXM3x2DEPTH]    = "TEXM3x2DEPTH";
    opcodeNames[D3DSIO_TEXDP3]          = "TEXDP3";
    opcodeNames[D3DSIO_TEXM3x3]         = "TEXM3x3";
    opcodeNames[D3DSIO_TEXDEPTH]        = "TEXDEPTH";
    opcodeNames[D3DSIO_CMP]             = "CMP";
    opcodeNames[D3DSIO_BEM]             = "BEM";
    opcodeNames[D3DSIO_DP2ADD]          = "DP2ADD";
    opcodeNames[D3DSIO_DSX]             = "DSX";
    opcodeNames[D3DSIO_DSY]             = "DSY";
    opcodeNames[D3DSIO_TEXLDD]          = "TEXLDD";
    opcodeNames[D3DSIO_SETP]            = "SETP";
    opcodeNames[D3DSIO_TEXLDL]          = "TEXLDL";
    opcodeNames[D3DSIO_BREAKP]          = "BREAKP";
    opcodeNames[D3DSIO_PHASE]           = "PHASE";
    opcodeNames[D3DSIO_COMMENT]         = "COMMENT";
    opcodeNames[D3DSIO_END]             = "END";

    paramRegisterTypeNames[D3DSPR_TEMP       ] = "TEMP";
    paramRegisterTypeNames[D3DSPR_INPUT      ] = "INPUT";
    paramRegisterTypeNames[D3DSPR_CONST      ] = "CONST";
    paramRegisterTypeNames[D3DSPR_ADDR       ] = "ADDR";
    paramRegisterTypeNames[D3DSPR_RASTOUT    ] = "RASTOUT";
    paramRegisterTypeNames[D3DSPR_ATTROUT    ] = "ATTROUT";
    paramRegisterTypeNames[D3DSPR_TEXCRDOUT  ] = "TEXCRDOUT";
    paramRegisterTypeNames[D3DSPR_CONSTINT   ] = "CONSTINT";
    paramRegisterTypeNames[D3DSPR_COLOROUT   ] = "COLOROUT";
    paramRegisterTypeNames[D3DSPR_DEPTHOUT   ] = "DEPTHOUT";
    paramRegisterTypeNames[D3DSPR_SAMPLER    ] = "SAMPLER";
    paramRegisterTypeNames[D3DSPR_CONST2     ] = "CONST2";
    paramRegisterTypeNames[D3DSPR_CONST3     ] = "CONST3";
    paramRegisterTypeNames[D3DSPR_CONST4     ] = "CONST4";
    paramRegisterTypeNames[D3DSPR_CONSTBOOL  ] = "CONSTBOOL";
    paramRegisterTypeNames[D3DSPR_LOOP       ] = "LOOP";
    paramRegisterTypeNames[D3DSPR_TEMPFLOAT16] = "TEMPFLOAT16";
    paramRegisterTypeNames[D3DSPR_MISCTYPE   ] = "MISCTYPE";
    paramRegisterTypeNames[D3DSPR_LABEL      ] = "LABEL";
    paramRegisterTypeNames[D3DSPR_PREDICATE  ] = "PREDICATE";

    paramSourceModNames[ D3DSPSM_NONE    ] = "NONE";
    paramSourceModNames[ D3DSPSM_NEG     ] = "NEG";
    paramSourceModNames[ D3DSPSM_BIAS    ] = "BIAS";
    paramSourceModNames[ D3DSPSM_BIASNEG ] = "BIASNEG";
    paramSourceModNames[ D3DSPSM_SIGN    ] = "SIGN";
    paramSourceModNames[ D3DSPSM_SIGNNEG ] = "SIGNNEG";
    paramSourceModNames[ D3DSPSM_COMP    ] = "COMP";
    paramSourceModNames[ D3DSPSM_X2      ] = "X2";
    paramSourceModNames[ D3DSPSM_X2NEG   ] = "X2NEG";
    paramSourceModNames[ D3DSPSM_DZ      ] = "DZ";
    paramSourceModNames[ D3DSPSM_DW      ] = "DW";
    paramSourceModNames[ D3DSPSM_ABS     ] = "ABS";
    paramSourceModNames[ D3DSPSM_ABSNEG  ] = "ABSNEG";
    paramSourceModNames[ D3DSPSM_NOT     ] = "NOT";

    comparisonNames[ D3DSPC_RESERVED0 ] = "RESERVED0";
    comparisonNames[ D3DSPC_GT ] = "GT";
    comparisonNames[ D3DSPC_EQ ] = "EQ";
    comparisonNames[ D3DSPC_GE ] = "GE";
    comparisonNames[ D3DSPC_LT ] = "LT";
    comparisonNames[ D3DSPC_NE ] = "NE";
    comparisonNames[ D3DSPC_LE ] = "LE";
    comparisonNames[ D3DSPC_RESERVED1 ] = "RESERVED1";

    textureTypeNames[ D3DSTT_UNKNOWN ] = "UNKNOWN";
    textureTypeNames[ D3DSTT_2D ] = "2D";
    textureTypeNames[ D3DSTT_CUBE ] = "CUBE";
    textureTypeNames[ D3DSTT_VOLUME ] = "VOLUME";

    usageNames[0] = "POSITION";
    usageNames[1] = "BLENDWEIGHT";
    usageNames[2] = "BLENDINDICES";
    usageNames[3] = "NORMAL";
    usageNames[4] = "PSIZE";
    usageNames[5] = "TEXCOORD";
    usageNames[6] = "TANGENT";
    usageNames[7] = "BINORMAL";
    usageNames[8] = "TESSFACTOR";
    usageNames[9] = "POSITIONT";
    usageNames[10] = "COLOR";
    usageNames[11] = "FOG";
    usageNames[12] = "DEPTH";
    usageNames[13] = "SAMPLE";
}

void IRPrinter::visit(EndIRNode *n)
{
}

