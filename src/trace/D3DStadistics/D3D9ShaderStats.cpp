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

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

#include "StringUtils.h"
#include "D3D9ShaderStats.h"

using namespace std;

D3D9ShaderStats::D3D9ShaderStats () {
	numDEF = 0;
	numPHASE = 0;
	numADD = 0;
	numBEM = 0;
	numCMP = 0;
	numCND = 0;
	numDP3 = 0;
	numDP4 = 0;
	numLRP = 0;
	numMAD = 0;
	numMOV = 0;
	numMUL = 0;
	numNOP = 0;
	numSUB = 0;
	numTEX = 0;
	numTEXBEM = 0;
	numTEXBEML = 0;
	numTEXCOORD = 0;
	numTEXCRD = 0;
	numTEXDEPTH = 0;
	numTEXDP3 = 0;
	numTEXDP3TEX = 0;
	numTEXKILL = 0;
	numTEXLD = 0;
	numTEXM3X2DEPTH = 0;
	numTEXM3X2PAD = 0;
	numTEXM3X2TEX = 0;
	numTEXM3X3 = 0;
	numTEXM3X3PAD = 0;
	numTEXM3X3TEX = 0;
	numTEXM3X3VSPEC = 0;
	numTEXREG2AR = 0;
	numTEXREG2GB = 0;
	numTEXREG2RGB = 0;
	numABS = 0;
	numEXP = 0;
	numFRC = 0;
	numLOG = 0;
	numPS = 0;
	numRCP = 0;
	numRSQ = 0;
	numTEXLDB = 0;
	numTEXLDP = 0;
	numTEXLDD = 0;
	numTEXLDL = 0;
	numOTHER = 0;
	numDST = 0;
	numLIT = 0;
	numLOGP = 0;
	numM3X2 = 0;
	numM3X3 = 0;
	numM3X4 = 0;
	numM4X3 = 0;
	numM4X4 = 0;
	numMAX = 0;
	numMIN = 0;
	numMOVA = 0;
	numSGE = 0;
	numSLT = 0;
	numCALL = 0;
	numCALLNZ = 0;
	numCRS = 0;
	numDCL = 0;
	numDEFI = 0;
	numDEFB = 0;
	numELSE = 0;
	numENDIF = 0;
	numENDLOOP = 0;
	numENDREP = 0;
	numIF = 0;
	numLABEL = 0;
	numLOOP = 0;
	numNRM = 0;
	numPOW = 0;
	numREP = 0;
	numRET = 0;
	numSGN = 0;
	numSINCOS = 0;
	numDP2ADD = 0;
	numTEXM3X3SPEC = 0;
	numBREAK = 0;
	numBREAKP = 0;
	numSETP = 0;
    numDSX = 0;
    numDSY = 0;
	vectorInstructions = 0;
	scalarInstructions = 0;
	specialInstructions = 0;
    specialD3D9Instructions = 0;
	textureInstructions = 0;
	pixelShaderSlots = 0;
	vertexShaderSlots = 0;
	shaderSlots = 0;

	for (int i = 0; i<10; i++)
		pixelShaderType[i] = 0;
	
	for (int i = 0; i<6; i++)
		vertexShaderType[i] = 0;

	acumShaders = 0;
	acumShaderSlots = 0;
	acumPixelShaderSlots = 0;
	acumVertexShaderSlots = 0;
	acumTextureInstructions = 0;

	for(int i = 0; i<16; i++)
		textureSamplers[i] = 0;

    execSlots = 0;
    cDepth = 0;
}

void D3D9ShaderStats::reset(){
	numDEF = 0;
	numPHASE = 0;
	numADD = 0;
	numBEM = 0;
	numCMP = 0;
	numCND = 0;
	numDP3 = 0;
	numDP4 = 0;
	numLRP = 0;
	numMAD = 0;
	numMOV = 0;
	numMUL = 0;
	numNOP = 0;
	numSUB = 0;
	numTEX = 0;
	numTEXBEM = 0;
	numTEXBEML = 0;
	numTEXCOORD = 0;
	numTEXCRD = 0;
	numTEXDEPTH = 0;
	numTEXDP3 = 0;
	numTEXDP3TEX = 0;
	numTEXKILL = 0;
	numTEXLD = 0;
	numTEXM3X2DEPTH = 0;
	numTEXM3X2PAD = 0;
	numTEXM3X2TEX = 0;
	numTEXM3X3 = 0;
	numTEXM3X3PAD = 0;
	numTEXM3X3TEX = 0;
	numTEXM3X3VSPEC = 0;
	numTEXREG2AR = 0;
	numTEXREG2GB = 0;
	numTEXREG2RGB = 0;
	numABS = 0;
	numEXP = 0;
	numFRC = 0;
	numLOG = 0;
	numPS = 0;
	numRCP = 0;
	numRSQ = 0;
	numTEXLDB = 0;
	numTEXLDP = 0;
	numTEXLDD = 0;
	numTEXLDL = 0;
	numOTHER = 0;
	numDST = 0;
	numLIT = 0;
	numLOGP = 0;
	numM3X2 = 0;
	numM3X3 = 0;
	numM3X4 = 0;
	numM4X3 = 0;
	numM4X4 = 0;
	numMAX = 0;
	numMIN = 0;
	numMOVA = 0;
	numSGE = 0;
	numSLT = 0;
	numCALL = 0;
	numCALLNZ = 0;
	numCRS = 0;
	numDCL = 0;
	numDEFI = 0;
	numDEFB = 0;
	numELSE = 0;
	numENDIF = 0;
	numENDLOOP = 0;
	numENDREP = 0;
	numIF = 0;
	numLABEL = 0;
	numLOOP = 0;
	numNRM = 0;
	numPOW = 0;
	numREP = 0;
	numRET = 0;
	numSGN = 0;
	numSINCOS = 0;
	numDP2ADD = 0;
	numTEXM3X3SPEC = 0;
	numBREAK = 0;
	numBREAKP = 0;
	numSETP = 0;
    numDSX = 0;
    numDSY = 0;
	vectorInstructions = 0;
	scalarInstructions = 0;
	specialInstructions = 0;
    specialD3D9Instructions = 0;
	textureInstructions = 0;
	pixelShaderSlots = 0;
	vertexShaderSlots = 0;
	shaderSlots = 0;

	for (int i = 0; i<10; i++)
		pixelShaderType[i] = 0;

	for (int i = 0; i<6; i++)
		vertexShaderType[i] = 0;

	differentShaders.clear();

	acumShaders = 0;
	acumShaderSlots = 0;
	acumPixelShaderSlots = 0;
	acumVertexShaderSlots = 0;
	acumTextureInstructions = 0;

	for(int i = 0; i<16; i++)
		textureSamplers[i] = 0;

    execSlots = 0;
    cDepth = 0;
}

D3D9ShaderStats::InstructionType D3D9ShaderStats::getInstructionType(string name) {
	
	if( name.substr(0,name.find_first_of('_', 0)) == "def") return DEF;
	else if( name.substr(0,name.find_first_of('_', 0)) == "phase") return PHASE;
	else if( name.substr(0,name.find_first_of('_', 0)) == "add") return ADD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "bem") return BEM;
	else if( name.substr(0,name.find_first_of('_', 0)) == "cmp") return CMP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "cnd") return CND;
	else if( name.substr(0,name.find_first_of('_', 0)) == "dp3") return DP3;
	else if( name.substr(0,name.find_first_of('_', 0)) == "dp4") return DP4;
	else if( name.substr(0,name.find_first_of('_', 0)) == "lrp") return LRP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "mad") return MAD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "mov") return MOV;
	else if( name.substr(0,name.find_first_of('_', 0)) == "mul") return MUL;
	else if( name.substr(0,name.find_first_of('_', 0)) == "nop") return NOP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "sub") return SUB;
	else if( name.substr(0,name.find_first_of('_', 0)) == "tex") return TEX;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texbem") return TEXBEM;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texbeml") return TEXBEML;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texcoord") return TEXCOORD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texcrd") return TEXCRD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texdepth") return TEXDEPTH;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texdp3") return TEXDP3;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texdp3tex") return TEXDP3TEX;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texkill") return TEXKILL;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texld") return TEXLD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x2depth") return TEXM3X2DEPTH;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x2pad") return TEXM3X2PAD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x2tex") return TEXM3X2TEX;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x3") return TEXM3X3;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x3pad") return TEXM3X3PAD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x3tex") return TEXM3X3TEX;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texm3x3vspec") return TEXM3X3VSPEC;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texreg2ar") return TEXREG2AR;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texreg2gb") return TEXREG2GB;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texreg2rgb") return TEXREG2RGB;
	else if( name.substr(0,name.find_first_of('_', 0)) == "dst") return DST;
	else if( name.substr(0,name.find_first_of('_', 0)) == "lit") return LIT;
	else if( name.substr(0,name.find_first_of('_', 0)) == "logp") return LOGP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "m3x2") return M3X2;
	else if( name.substr(0,name.find_first_of('_', 0)) == "m3x3") return M3X3;
	else if( name.substr(0,name.find_first_of('_', 0)) == "m3x4") return M3X4;
	else if( name.substr(0,name.find_first_of('_', 0)) == "m4x3") return M4X3;
	else if( name.substr(0,name.find_first_of('_', 0)) == "m4x4") return M4X4;
	else if( name.substr(0,name.find_first_of('_', 0)) == "max") return MAX;
	else if( name.substr(0,name.find_first_of('_', 0)) == "min") return MIN;
	else if( name.substr(0,name.find_first_of('_', 0)) == "mova") return MOVA;
	else if( name.substr(0,name.find_first_of('_', 0)) == "sge") return SGE;
	else if( name.substr(0,name.find_first_of('_', 0)) == "slt") return SLT;
	else if( name.substr(0,name.find_first_of('_', 0)) == "abs") return ABS;
	else if( name.substr(0,name.find_first_of('_', 0)) == "exp") return EXP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "frc") return FRC;
	else if( name.substr(0,name.find_first_of('_', 0)) == "log") return LOG;
	else if( name == "ps") return PS;
	else if( name.substr(0,name.find_first_of('_', 0)) == "rcp") return RCP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "rsq") return RSQ;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texldb") return TEXLDB;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texldp") return TEXLDP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texldd") return TEXLDD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "texldl") return TEXLDL;
	else if( name.substr(0,name.find_first_of('_', 0)) == "call") return CALL;
	else if( name.substr(0,name.find_first_of('_', 0)) == "callnz") return CALLNZ;
	else if( name.substr(0,name.find_first_of('_', 0)) == "crs") return CRS;
	else if( name == "dcl_2d") return DCL_SAMPLER;
	else if( name == "dcl_cube") return DCL_SAMPLER;
	else if( name == "dcl_volume") return DCL_SAMPLER;
	else if( name.substr(0,name.find_first_of('_', 0)) == "dcl") return DCL;
	else if( name.substr(0,name.find_first_of('_', 0)) == "defi") return DEFI;
	else if( name.substr(0,name.find_first_of('_', 0)) == "defb") return DEFB;
	else if( name.substr(0,name.find_first_of('_', 0)) == "else") return ELSE;
	else if( name.substr(0,name.find_first_of('_', 0)) == "endif") return ENDIF;
	else if( name.substr(0,name.find_first_of('_', 0)) == "endloop") return ENDLOOP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "endrep") return ENDREP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "if") return IF;
	else if( name.substr(0,name.find_first_of('_', 0)) == "label") return LABEL;
	else if( name.substr(0,name.find_first_of('_', 0)) == "loop") return LOOP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "nrm") return NRM;
	else if( name.substr(0,name.find_first_of('_', 0)) == "pow") return POW;
	else if( name.substr(0,name.find_first_of('_', 0)) == "rep") return REP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "ret") return RET;
	else if( name.substr(0,name.find_first_of('_', 0)) == "sgn") return SGN;
	else if( name.substr(0,name.find_first_of('_', 0)) == "sincos") return SINCOS;
	else if( name.substr(0,name.find_first_of('_', 0)) == "dp2add") return DP2ADD;
	else if( name.substr(0,name.find_first_of('_', 0)) == "break" ) return BREAK;
	else if( name.substr(0,name.find_first_of('_', 0)) == "breakp" ) return BREAKP;
	else if( name.substr(0,name.find_first_of('_', 0)) == "setp" ) return SETP;
	else if (name.substr(0,name.find_first_of('_', 0)) == "dsx" ) return DSX;
	else if (name.substr(0,name.find_first_of('_', 0)) == "dsy" ) return DSY;
    else if (name == "+") return PLUS;
	else if( name == "ps_1_0" ) return PS10;
	else if( name == "ps_1_1" ) return PS11;
	else if( name == "ps_1_2" ) return PS12;
	else if( name == "ps_1_3" ) return PS13;
	else if( name == "ps_1_4" ) return PS14;
	else if( name == "ps_2_0" ) return PS20;
	else if( name == "ps_2_x" ) return PS2X;
	else if( name == "ps_2_sw" ) return PS2SW;
	else if( name == "ps_3_0" ) return PS30;
	else if( name == "ps_3_sw" ) return PS3SW;
	else if( name == "vs_1_1" ) return VS11;
	else if( name == "vs_2_0" ) return VS20;
	else if( name == "vs_2_x" ) return VS2X;
	else if( name == "vs_2_sw" ) return VS2SW;
	else if( name == "vs_3_0" ) return VS30;
	else if( name == "vs_3_sw" ) return VS3SW;
	else if( name == "//") return COMMENT;
	return OTHER;
}

void D3D9ShaderStats::saveShader(DWORD *shaderFunction, int ident) {

	ID3DXBuffer *disassembledShader;
	string disassembledStr;

	if(S_OK != D3DXDisassembleShader(shaderFunction, false, 0, &disassembledShader)) return;

	disassembledStr.append((char *)disassembledShader->GetBufferPointer());
	disassembledShader->Release();

	char filename[256];
	sprintf(filename, "Shader %d.txt", ident);
	ofstream f;
    f.open(filename);

    if ( !f.is_open() )
        return; 

    f << disassembledStr;
    f << "\n";
    f.close();

}

bool D3D9ShaderStats::analyze(void *sh, DWORD *shaderFunction) {

	differentShaders.push_back(sh);
	ID3DXBuffer *disassembledShader;
	if(S_OK != D3DXDisassembleShader(shaderFunction, false, 0, &disassembledShader))
		return false;
	string disassembledStr;
	disassembledStr.append((char *)disassembledShader->GetBufferPointer());

	disassembledShader->Release();

	vector <string> lines = explode(disassembledStr, "\n");
	string line;
	vector <string> words;

    ofstream f;
    f.open("otherlog.txt", ios::app);

	bool isPixelShader = false;
	for(int i = 0; i < (lines.size() - 1); i ++) {
		line = lines[i];
		words = explode_by_any(line," \t,"); //
		if(words.size() > 0) {
			bool sample = false;
			InstructionType t = getInstructionType(words[0]);

            if (words.size() == 2 && words[1] == "oDepth") {
                /*f << "--------------------------" << "\n";
                f << disassembledStr << "\n";*/
                cDepth = 1;
            }
            else if (words.size() > 2) {
                if (words[1] == "oDepth" || words[2] == "oDepth") {
                    /*f << "--------------------------" << "\n";
                    f << disassembledStr << "\n";*/
                    cDepth = 1;
                }
            }

            if (t == PLUS) t = getInstructionType(words[1]);

			switch( t ) {
				case DEF:	numDEF++; break;
				case PHASE: numPHASE++; break;
				case ADD:	numADD++;
                            vectorInstructions++;
                            break;
				case BEM:	numBEM++; // Apply a fake bump environment-map transform.
                            specialInstructions++;
                            break;
				case CMP:	numCMP++;
                            vectorInstructions++;
                            break;
				case CND:	numCND++; // Conditionally chooses between src1 and src2, based on the comparison src0 > 0.5.
                            vectorInstructions++;
                            break;
				case DP3:	numDP3++;
                            vectorInstructions++;
                            break;
				case DP4:	numDP4++;
                            vectorInstructions++;
                            break;
				case LRP:	numLRP++;
                            vectorInstructions++;
                            break;
				case MAD:	numMAD++;
                            vectorInstructions++;
                            break;
				case MOV:	numMOV++;
                            vectorInstructions++;
                            break;
				case MUL:	numMUL++;
                            vectorInstructions++;
                            break;
				case NOP:	numNOP++; break;
				case SUB:	numSUB++;
                            vectorInstructions++;
                            break;
				case TEX:	numTEX++;
							textureSamplers[getNumStage(words[1])]++;
							textureInstructions++;
							break;
				case TEXBEM:	numTEXBEM++; // Apply a fake bump environment-map transform.
								textureSamplers[getNumStage(words[1])]++;
								textureInstructions++;
								break;
				case TEXBEML:	numTEXBEML++; // Apply a fake bump environment-map transform with luminance correction.
								textureSamplers[getNumStage(words[1])]++;
								textureInstructions++;
								break;
				case TEXCOORD:	numTEXCOORD++;
                                vectorInstructions++;
                                break;
				case TEXCRD:	numTEXCRD++;
                                vectorInstructions++;
                                break;
				case TEXDEPTH:	numTEXDEPTH++;
                                specialD3D9Instructions++;
                                break;
				case TEXDP3:	numTEXDP3++;
                                vectorInstructions++;
                                break;
				case TEXDP3TEX: numTEXDP3TEX++;
								textureSamplers[getNumStage(words[1])]++;
								textureInstructions++;
								break;
				case TEXKILL:	numTEXKILL++;
                                specialD3D9Instructions++;
                                break;
				case TEXLD:		numTEXLD++; 
								textureInstructions++;
								if (pixelShaderType[4] != 0)
									textureSamplers[getNumStage(words[1])]++;
								else 
									textureSamplers[getNumStage(words[3])]++;
								break;
				case TEXM3X2DEPTH:	numTEXM3X2DEPTH++;
                                    specialD3D9Instructions++;
                                    break;
				case TEXM3X2PAD:	numTEXM3X2PAD++;
                                    vectorInstructions++;
                                    break;
				case TEXM3X2TEX:	numTEXM3X2TEX++; //OK
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case TEXM3X3:		numTEXM3X3++; 
                                    vectorInstructions++;
                                    break;
				case TEXM3X3PAD:	numTEXM3X3PAD++;
                                    vectorInstructions++;
                                    break;
				case TEXM3X3TEX:	numTEXM3X3TEX++; //OK
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case TEXM3X3VSPEC:	numTEXM3X3VSPEC++; //OK
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case TEXREG2AR:		numTEXREG2AR++; //OK
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case TEXREG2GB:		numTEXREG2GB++; //OK
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case TEXREG2RGB:	numTEXREG2RGB++; //OK
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case ABS:	numABS++;
                            vectorInstructions++;
                            break;
				case EXP:	numEXP++;
                            scalarInstructions++;
                            break;
				case FRC:	numFRC++;
                            vectorInstructions++;
                            break;
				case LOG:	numLOG++;
                            scalarInstructions++;
                            break;
				case PS:	numPS++; break;
				case RCP:	numRCP++;
                            scalarInstructions++;
                            break;
				case RSQ:	numRSQ++;
                            scalarInstructions++;
                            break;
				case TEXLDB:	numTEXLDB++; 
								textureSamplers[getNumStage(words[3])]++; // swizzle in ps_3_0
								textureInstructions++;
								break;
				case TEXLDP:	numTEXLDP++; 
								textureSamplers[getNumStage(words[3])]++; // swizzle in ps_3_0
								textureInstructions++;
								break;
				case OTHER:		numOTHER++;
                                f << "Inst: " << line << "\n";
                                break;
				case TEXLDD:	numTEXLDD++; 
								textureSamplers[getNumStage(words[3])]++; // swizzle???
								textureInstructions++;
								break;
				case TEXLDL:	numTEXLDL++;
								textureSamplers[getNumStage(words[3])]++; // swizzle
								textureInstructions++;
								break;
				case TEXM3X3SPEC:	numTEXM3X3SPEC++; 
									textureSamplers[getNumStage(words[1])]++;
									textureInstructions++;
									break;
				case DST:	numDST++;
                            vectorInstructions++;
                            break;
				case LIT:	numLIT++;
                            specialInstructions++;
                            break;
				case LOGP:	numLOGP++; 
                            scalarInstructions++;
                            break;
				case M3X2:	//numM3X2++;
                            numDP3 += 2;
                            vectorInstructions += 2;
                            break;
				case M3X3:	//numM3X3++;
                            numDP3 += 3;
                            vectorInstructions += 3;
                            break;
				case M3X4:	//numM3X4++;
                            numDP3 += 4;
                            vectorInstructions += 4;
                            break;
				case M4X3:	//numM4X3++;
                            numDP4 += 3;
                            vectorInstructions += 3;
                            break;
				case M4X4:	//numM4X4++;
                            numDP4 += 4;
                            vectorInstructions += 4;
                            break;
				case MAX:	numMAX++;
                            vectorInstructions++;
                            break;
				case MIN:	numMIN++;
                            vectorInstructions++;
                            break;
				case MOVA:	numMOVA++; // Move data from a floating point register to the Address Register, a0.
                            vectorInstructions++;
                            break;
				case SGE:	numSGE++;
                            vectorInstructions++;
                            break;
				case SLT:	numSLT++;
                            vectorInstructions++;
                            break;
				case CALL:	numCALL++; break;
				case CALLNZ:	numCALLNZ++; break;
				case CRS:	numCRS++;
                            vectorInstructions++;
                            break;
				case DCL:	numDCL++; break;
				case DCL_SAMPLER:	numDCL++; break;
				case DEFI:	numDEFI++; break;
				case DEFB:	numDEFB++; break;
				case ELSE:	numELSE++; break;
				case ENDIF:		numENDIF++; break;
				case ENDLOOP:	numENDLOOP++; break;
				case ENDREP:	numENDREP++; break;
				case IF:	numIF++; break;
				case LABEL:	numLABEL++; break;
				case LOOP:	numLOOP++; break;
				case NRM:	numNRM++;
                            vectorInstructions++;
                            break;
				case POW:	numPOW++;
                            scalarInstructions++;
                            break;
				case REP:	numREP++; break;
				case RET:	numRET++; break;
				case SGN:	numSGN++;
                            vectorInstructions++;
                            break;
				case SINCOS:	numSINCOS++;
                                specialInstructions++;
                                break;
				case DP2ADD:	numDP2ADD++;
                                vectorInstructions++;
                                break;
                case BREAK:     numBREAK++;
                                break;
                case BREAKP:    numBREAKP++;
                                break;
                case SETP:      numSETP++;
                                break;
                case DSX:       numDSX++;
                                specialD3D9Instructions++;
                                break;
                case DSY:       numDSY++;
                                specialD3D9Instructions++;
                                break;

				case PS10:	pixelShaderType[0]++; 
							isPixelShader = true;
							break;
				case PS11:	pixelShaderType[1]++; 
							isPixelShader = true;
							break;
				case PS12:	pixelShaderType[2]++; 
							isPixelShader = true;
							break;
				case PS13:	pixelShaderType[3]++; 
							isPixelShader = true;
							break;
				case PS14:	pixelShaderType[4]++; 
							isPixelShader = true; 
							break;
				case PS20:	pixelShaderType[5]++; 
							isPixelShader = true;
							break;
				case PS2X:	pixelShaderType[6]++; 
							isPixelShader = true; 
							break;
				case PS2SW:	pixelShaderType[7]++; 
							isPixelShader = true; 
							break;
				case PS30:	pixelShaderType[8]++; 
							isPixelShader = true;
							break;
				case PS3SW:	pixelShaderType[9]++; 
							isPixelShader = true; 
							break;

				case VS11:	vertexShaderType[0]++;break;
				case VS20:	vertexShaderType[1]++;break;
				case VS2X:	vertexShaderType[2]++;break;
				case VS2SW:	vertexShaderType[3]++;break;
				case VS30:	vertexShaderType[4]++;break;
				case VS3SW:	vertexShaderType[5]++;break;
				case COMMENT:break;
			}
		}
	}
    f.close();

	line = lines[lines.size() - 1];
	words = explode(line, " ");

	if(words.size() < 3) return false;

	int aux = atoi(words[2].c_str()); 

	if (aux == 0) return false;

	shaderSlots = aux;

    execSlots = shaderSlots;

	if(isPixelShader) pixelShaderSlots = aux;
	else vertexShaderSlots = aux;

	if (textureInstructions != 0) 
		acumTextureInstructions++;

	acumShaders++;
	acumShaderSlots++;

	if (isPixelShader) acumPixelShaderSlots++;
	else acumVertexShaderSlots++;

	return true;
}

void D3D9ShaderStats::printShaderStats (ofstream* f) {

	*f << numDEF << ";";
	*f << numDEFI << ";";
	*f << numDEFB << ";";
	*f << numDCL << ";";
	*f << numPS << ";";
	*f << numLABEL << ";";
	*f << numNOP << ";";
	*f << numPHASE << ";";
	*f << numCALL << ";";
	*f << numCALLNZ << ";";
	*f << numELSE << ";";
	*f << numENDIF << ";";
	*f << numENDLOOP << ";";
	*f << numENDREP << ";";
	*f << numIF << ";";
	*f << numLOOP << ";";
	*f << numREP << ";";
	*f << numRET << ";";
	*f << numADD << ";";
	*f << numBEM << ";";
	*f << numCMP << ";";
	*f << numCND << ";";
	*f << numDP3 << ";";
	*f << numDP4 << ";";
	*f << numLRP << ";";
	*f << numMAD << ";";
	*f << numMOV << ";";
	*f << numMUL << ";";
	*f << numSUB << ";";
	*f << numTEX << ";";
	*f << numTEXBEM << ";";
	*f << numTEXBEML << ";";
	*f << numTEXCOORD << ";";
	*f << numTEXCRD << ";";
	*f << numTEXDEPTH << ";";
	*f << numTEXDP3 << ";";
	*f << numTEXDP3TEX << ";";
	*f << numTEXKILL << ";";
	*f << numTEXLD << ";";
	*f << numTEXM3X2DEPTH << ";";
	*f << numTEXM3X2PAD << ";";
	*f << numTEXM3X2TEX << ";";
	*f << numTEXM3X3 << ";";
	*f << numTEXM3X3PAD << ";";
	*f << numTEXM3X3TEX << ";";
	*f << numTEXM3X3VSPEC << ";";
	*f << numTEXREG2AR << ";";
	*f << numTEXREG2GB << ";";
	*f << numTEXREG2RGB << ";";
	*f << numABS << ";";
	*f << numEXP << ";";
	*f << numFRC << ";";
	*f << numLOG << ";";
	*f << numRCP << ";";
	*f << numRSQ << ";";
	*f << numTEXLDB << ";";
	*f << numTEXLDP << ";";
	*f << numTEXLDD << ";";
	*f << numTEXLDL << ";";
	*f << numTEXM3X3SPEC << ";";
	*f << numDST << ";";
	*f << numLIT << ";";
	*f << numLOGP << ";";
	*f << numM3X2 << ";";
	*f << numM3X3 << ";";
	*f << numM3X4 << ";";
	*f << numM4X3 << ";";
	*f << numM4X4 << ";";
	*f << numMAX << ";";
	*f << numMIN << ";";
	*f << numMOVA << ";";
	*f << numSGE << ";";
	*f << numSLT << ";";
	*f << numCRS << ";";
	*f << numNRM << ";";
	*f << numPOW << ";";
	*f << numSGN << ";";
	*f << numSINCOS << ";";
	*f << numDP2ADD << ";";
    *f << numBREAK << ";";
	*f << numBREAKP << ";";
	*f << numSETP << ";";
    *f << numDSX << ";";
    *f << numDSY << ";";
	*f << numOTHER << ";";
	*f << ((acumShaderSlots == 0) ? 0 : (double)shaderSlots / (double)acumShaderSlots) << ";";
	*f << ((acumPixelShaderSlots == 0) ? 0 : (double)pixelShaderSlots / (double)acumPixelShaderSlots) << ";";
	*f << ((acumVertexShaderSlots == 0) ? 0 : (double)vertexShaderSlots /(double)acumVertexShaderSlots) << ";";
    *f << execSlots << ";";
	*f << pixelShaderType[0] << ";";
	*f << pixelShaderType[1] << ";";
	*f << pixelShaderType[2] << ";";
	*f << pixelShaderType[3] << ";";
	*f << pixelShaderType[4] << ";";
	*f << pixelShaderType[5] << ";";
	*f << pixelShaderType[6] << ";";
	*f << pixelShaderType[7] << ";";
	*f << pixelShaderType[8] << ";";
	*f << pixelShaderType[9] << ";";
	*f << vertexShaderType[0] << ";";
	*f << vertexShaderType[1] << ";";
	*f << vertexShaderType[2] << ";";
	*f << vertexShaderType[3] << ";";
	*f << vertexShaderType[4] << ";";
	*f << vertexShaderType[5] << ";";
	*f << ((acumTextureInstructions == 0) ? 0 : (double)textureInstructions / (double)acumTextureInstructions) << ";";
	*f << differentShaders.size() << ";";
    *f << vectorInstructions << ";";
    *f << scalarInstructions << ";";
    *f << specialInstructions << ";";
    *f << specialD3D9Instructions << ";";
    *f << textureInstructions << ";";
    *f << cDepth << ";";

}

void D3D9ShaderStats::addShaderStats(D3D9ShaderStats* second, unsigned long long exec) {

    /*ofstream f;
        
            f.open("shLog.txt", ios::app);
            f << "++principi\n";
            f.close();*/

	if (second != NULL) {
		numDEF = numDEF + second->numDEF * exec;
		numPHASE = numPHASE + second->numPHASE * exec;
		numADD = numADD + second->numADD * exec;
		numBEM = numBEM + second->numBEM * exec;
		numCMP = numCMP + second->numCMP * exec;
		numCND = numCND + second->numCND * exec;
		numDP3 = numDP3 + second->numDP3 * exec;
		numDP4 = numDP4 + second->numDP4 * exec;
		numLRP = numLRP + second->numLRP * exec;
		numMAD = numMAD + second->numMAD * exec;
		numMOV = numMOV + second->numMOV * exec;
		numMUL = numMUL + second->numMUL * exec;
		numNOP = numNOP + second->numNOP * exec;
		numSUB = numSUB + second->numSUB * exec;
		numTEX = numTEX + second->numTEX * exec;
		numTEXBEM = numTEXBEM + second->numTEXBEM * exec;
		numTEXBEML = numTEXBEML + second->numTEXBEML * exec;
		numTEXCOORD = numTEXCOORD + second->numTEXCOORD * exec;
		numTEXCRD = numTEXCRD + second->numTEXCRD * exec;
		numTEXDEPTH = numTEXDEPTH + second->numTEXDEPTH * exec;
		numTEXDP3 = numTEXDP3 + second->numTEXDP3 * exec;
		numTEXDP3TEX = numTEXDP3TEX + second->numTEXDP3TEX * exec;
		numTEXKILL = numTEXKILL + second->numTEXKILL * exec;
		numTEXLD = numTEXLD + second->numTEXLD * exec;
		numTEXM3X2DEPTH = numTEXM3X2DEPTH + second->numTEXM3X2DEPTH * exec;
		numTEXM3X2PAD = numTEXM3X2PAD + second->numTEXM3X2PAD * exec;
		numTEXM3X2TEX = numTEXM3X2TEX + second->numTEXM3X2TEX * exec;
		numTEXM3X3 = numTEXM3X3 + second->numTEXM3X3 * exec;
		numTEXM3X3PAD = numTEXM3X3PAD + second->numTEXM3X3PAD * exec;
		numTEXM3X3TEX = numTEXM3X3TEX + second->numTEXM3X3TEX * exec;
		numTEXM3X3VSPEC = numTEXM3X3VSPEC + second->numTEXM3X3VSPEC * exec;
		numTEXREG2AR = numTEXREG2AR + second->numTEXREG2AR * exec;
		numTEXREG2GB = numTEXREG2GB + second->numTEXREG2GB * exec;
		numTEXREG2RGB = numTEXREG2RGB + second->numTEXREG2RGB * exec;
		numABS = numABS + second->numABS * exec;
		numEXP = numEXP + second->numEXP * exec;
		numFRC = numFRC + second->numFRC * exec;
		numLOG = numLOG + second->numLOG * exec;
		numPS = numPS + second->numPS * exec;
		numRCP = numRCP + second->numRCP * exec;
		numRSQ = numRSQ + second->numRSQ * exec;
		numTEXLDB = numTEXLDB + second->numTEXLDB * exec;
		numTEXLDP = numTEXLDP + second->numTEXLDP * exec;
		numTEXLDD = numTEXLDD + second->numTEXLDD * exec;
		numTEXLDL = numTEXLDL + second->numTEXLDL * exec;
		numOTHER = numOTHER + second->numOTHER * exec;
		numDST = numDST + second->numDST * exec;
		numLIT = numLIT + second->numLIT * exec;
		numLOGP = numLOGP + second->numLOGP * exec;
		numM3X2 = numM3X2 + second->numM3X2 * exec;
		numM3X3 = numM3X3 + second->numM3X3 * exec;
		numM3X4 = numM3X4 + second->numM3X4 * exec;
		numM4X3 = numM4X3 + second->numM4X3 * exec;
		numM4X4 = numM4X4 + second->numM4X4 * exec;
		numMAX = numMAX + second->numMAX * exec;
		numMIN = numMIN + second->numMIN * exec;
		numMOVA = numMOVA + second->numMOVA * exec;
		numSGE = numSGE + second->numSGE * exec;
		numSLT = numSLT + second->numSLT * exec;
		numCALL = numCALL + second->numCALL * exec;
		numCALLNZ = numCALLNZ + second->numCALLNZ * exec;
		numCRS = numCRS + second->numCRS * exec;
		numDCL = numDCL + second->numDCL * exec;
		numDEFI = numDEFI + second->numDEFI * exec;
		numDEFB = numDEFB + second->numDEFB * exec;
		numELSE = numELSE + second->numELSE * exec;
		numENDIF = numENDIF + second->numENDIF * exec;
		numENDLOOP = numENDLOOP + second->numENDLOOP * exec;
		numENDREP = numENDREP + second->numENDREP * exec;
		numIF = numIF + second->numIF * exec;
		numLABEL = numLABEL + second->numLABEL * exec;
		numLOOP = numLOOP + second->numLOOP * exec;
		numNRM = numNRM + second->numNRM * exec;
		numPOW = numPOW + second->numPOW * exec;
		numREP = numREP + second->numREP * exec;
		numRET = numRET + second->numRET * exec;
		numSGN = numSGN + second->numSGN * exec;
		numSINCOS = numSINCOS + second->numSINCOS * exec;
		numDP2ADD = numDP2ADD + second->numDP2ADD * exec;
		numBREAK = numBREAK + second->numBREAK * exec;
		numBREAKP = numBREAKP + second->numBREAKP * exec;
		numSETP = numSETP + second->numSETP * exec;	
        numDSX = numDSX + second->numDSX * exec;
        numDSY = numDSY + second->numDSY * exec;
		shaderSlots = shaderSlots + second->shaderSlots;
		pixelShaderSlots = pixelShaderSlots + second->pixelShaderSlots;
		vertexShaderSlots = vertexShaderSlots + second->vertexShaderSlots;
        execSlots = execSlots + second->execSlots * exec;
		pixelShaderType[0] = pixelShaderType[0] + second->pixelShaderType[0] * exec;
		pixelShaderType[1] = pixelShaderType[1] + second->pixelShaderType[1] * exec;
		pixelShaderType[2] = pixelShaderType[2] + second->pixelShaderType[2] * exec;
		pixelShaderType[3] = pixelShaderType[3] + second->pixelShaderType[3] * exec;
		pixelShaderType[4] = pixelShaderType[4] + second->pixelShaderType[4] * exec;
		pixelShaderType[5] = pixelShaderType[5] + second->pixelShaderType[5] * exec;
		pixelShaderType[6] = pixelShaderType[6] + second->pixelShaderType[6] * exec;
		pixelShaderType[7] = pixelShaderType[7] + second->pixelShaderType[7] * exec;
		pixelShaderType[8] = pixelShaderType[8] + second->pixelShaderType[8] * exec;
		pixelShaderType[9] = pixelShaderType[9] + second->pixelShaderType[9] * exec;
		vertexShaderType[0] = vertexShaderType[0] + second->vertexShaderType[0] * exec;
		vertexShaderType[1] = vertexShaderType[1] + second->vertexShaderType[1] * exec;
		vertexShaderType[2] = vertexShaderType[2] + second->vertexShaderType[2] * exec;
		vertexShaderType[3] = vertexShaderType[3] + second->vertexShaderType[3] * exec;
		vertexShaderType[4] = vertexShaderType[4] + second->vertexShaderType[4] * exec;
		vertexShaderType[5] = vertexShaderType[5] + second->vertexShaderType[5] * exec;

		vectorInstructions = vectorInstructions + second->vectorInstructions * exec;
		scalarInstructions = scalarInstructions + second->scalarInstructions * exec;
		specialInstructions = specialInstructions + second->specialInstructions * exec;
		specialD3D9Instructions = specialD3D9Instructions + second->specialD3D9Instructions * exec;
		textureInstructions = textureInstructions + second->textureInstructions * exec;

        cDepth = cDepth + second->cDepth * exec;

        
            /*f.open("shLog.txt", ios::app);
            f << "++fi1part\n";
            f.close();*/

		list<void *>::iterator it;
		list <void *> aux;

            /*f.open("shLog.txt", ios::app);
            f << "++DifferentShadersSize: " << differentShaders.size() << "\n";
            f << "++secondDifferentShadersSize: " << second->differentShaders.size() << "\n";
            f.close();*/

        for ( it=(second->differentShaders).begin() ; it != (second->differentShaders).end(); it++ ) {
            /*f.open("shLog.txt", ios::app);
            f << "++for:\n";
            f.close();
            f.open("shLog.txt", ios::app);
            f << "  ++ " << &(*it) << "\n";
            f.close();*/
            aux.push_back(*it);
        }
        

		differentShaders.merge(aux);
		differentShaders.unique();
        
            /*f.open("shLog.txt", ios::app);
            f << "++fi2part\n";
            f.close();*/

		if((second->shaderSlots) != 0) acumShaderSlots += second->acumShaderSlots;
		if((second->pixelShaderSlots) != 0) acumPixelShaderSlots += second->acumPixelShaderSlots;
		if((second->vertexShaderSlots) != 0) acumVertexShaderSlots += second->acumVertexShaderSlots;
		if((second->textureInstructions) != 0) acumTextureInstructions += second->acumTextureInstructions;

		acumShaders += second->acumShaders;

		/*map<int, int>::iterator it2;
		for ( it2=(second->textureSamplers).begin() ; it2 != (second->textureSamplers).end(); it2++ )
			textureSamplers[second->textureSamplers[(*it2).first]] += (*it2).second;*/

        for (int i = 0; i < 16; i++)
            textureSamplers[i] += second->textureSamplers[i];
	}

}

int D3D9ShaderStats::getNumStage(string stage) {

	if (stage == "t0") return 0;
	else if (stage == "t1") return 1;
	else if (stage == "t2") return 2;
	else if (stage == "t3") return 3;
	else if (stage == "t4") return 4;
	else if (stage == "t5") return 5;
	else if (stage == "t6") return 6;
	else if (stage == "s0") return 0;
	else if (stage == "s1") return 1;
	else if (stage == "s2") return 2;
	else if (stage == "s3") return 3;
	else if (stage == "s4") return 4;
	else if (stage == "s5") return 5;
	else if (stage == "s6") return 6;
	else if (stage == "s7") return 7;
	else if (stage == "s8") return 8;
	else if (stage == "s9") return 9;
	else if (stage == "s10") return 10;
	else if (stage == "s11") return 11;
	else if (stage == "s12") return 12;
	else if (stage == "s13") return 13;
	else if (stage == "s14") return 14;
	else if (stage == "s15") return 15;
	else return -1;
}

int D3D9ShaderStats::isUse(int numSampler) {

    if (numSampler < 16)
	    return textureSamplers[numSampler];
    else
        return 0;

}
