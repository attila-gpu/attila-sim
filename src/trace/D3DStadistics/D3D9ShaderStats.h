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

#ifndef __D3D9SHADERANALYZER
#define __D3D9SHADERANALYZER

#include <map>
#include <list>
#include <string>
#include <fstream>

////////////////////////////////////////////////////////////
////////		D3D9ShaderStats Declaration			////////
////////////////////////////////////////////////////////////

class D3D9ShaderStats {
	public:
		D3D9ShaderStats::D3D9ShaderStats ();
		void saveShader(DWORD *shaderFunction, int ident);
		bool analyze(void *sh, DWORD *shaderFunction);
		void addShaderStats(D3D9ShaderStats* second, unsigned long long exec);
        void printShaderStats (std::ofstream* f);
		void reset();
		int isUse( int numSampler);
        std::list<void*> differentShaders;
		
	private:

		enum InstructionType {
			DEF,
			PHASE,
			ADD,
			BEM,
			CMP,
			CND,
			DP3,
			DP4,
			LRP,
			MAD,
			MOV,
			MUL,
			NOP,
			SUB,
			TEX,
			TEXBEM,
			TEXBEML,
			TEXCOORD,
			TEXCRD,
			TEXDEPTH,
			TEXDP3,
			TEXDP3TEX,
			TEXKILL,
			TEXLD,
			TEXM3X2DEPTH,
			TEXM3X2PAD,
			TEXM3X2TEX,
			TEXM3X3,
			TEXM3X3PAD,
			TEXM3X3TEX,
			TEXM3X3VSPEC,
			TEXREG2AR,
			TEXREG2GB,
			TEXREG2RGB,
			ABS,
			EXP,
			FRC,
			LOG,
			PS,
			RCP,
			RSQ,
			TEXLDB,
			TEXLDP,
			TEXLDD,
			TEXLDL,
			TEXM3X3SPEC,
			DST,
			LIT,
			LOGP,
			M3X2,
			M3X3,
			M3X4,
			M4X3,
			M4X4,
			MAX,
			MIN,
			MOVA,
			SGE,
			SLT,
			CALL,
			CALLNZ,
			CRS,
			DCL,
			DCL_SAMPLER,
			DEFI,
			DEFB,
			ELSE,
			ENDIF,
			ENDLOOP,
			ENDREP,
			IF,
			LABEL,
			LOOP,
			NRM,
			POW,
			REP,
			RET,
			SGN,
			SINCOS,
			DP2ADD,
			COMMENT,
			BREAK,
			BREAKP,
			SETP,
			PS10,
			PS11,
			PS12,
			PS13,
			PS14,
			PS20,
			PS2X,
			PS2SW,
			PS30,
			PS3SW,
			VS11,
			VS20,
			VS2X,
			VS2SW,
			VS30,
			VS3SW,
            DSX,
            DSY,
			OTHER,
            PLUS
		};

		unsigned long long numDEF;
		unsigned long long numPHASE;
		unsigned long long numADD;
		unsigned long long numBEM;
		unsigned long long numCMP;
		unsigned long long numCND;
		unsigned long long numDP3;
		unsigned long long numDP4;
		unsigned long long numLRP;
		unsigned long long numMAD;
		unsigned long long numMOV;
		unsigned long long numMUL;
		unsigned long long numNOP;
		unsigned long long numSUB;
		unsigned long long numTEX;
		unsigned long long numTEXBEM;
		unsigned long long numTEXBEML;
		unsigned long long numTEXCOORD;
		unsigned long long numTEXCRD;
		unsigned long long numTEXDEPTH;
		unsigned long long numTEXDP3;
		unsigned long long numTEXDP3TEX;
		unsigned long long numTEXKILL;
		unsigned long long numTEXLD;
		unsigned long long numTEXM3X2DEPTH;
		unsigned long long numTEXM3X2PAD;
		unsigned long long numTEXM3X2TEX;
		unsigned long long numTEXM3X3;
		unsigned long long numTEXM3X3PAD;
		unsigned long long numTEXM3X3TEX;
		unsigned long long numTEXM3X3VSPEC;
		unsigned long long numTEXREG2AR;
		unsigned long long numTEXREG2GB;
		unsigned long long numTEXREG2RGB;
		unsigned long long numABS;
		unsigned long long numEXP;
		unsigned long long numFRC;
		unsigned long long numLOG;
		unsigned long long numPS;
		unsigned long long numRCP;
		unsigned long long numRSQ;
		unsigned long long numTEXLDB;
		unsigned long long numTEXLDP;
		unsigned long long numTEXLDD;
		unsigned long long numTEXLDL;
		unsigned long long numTEXM3X3SPEC;
		unsigned long long numOTHER;
		unsigned long long numDST;
		unsigned long long numLIT;
		unsigned long long numLOGP;
		unsigned long long numM3X2;
		unsigned long long numM3X3;
		unsigned long long numM3X4;
		unsigned long long numM4X3;
		unsigned long long numM4X4;
		unsigned long long numMAX;
		unsigned long long numMIN;
		unsigned long long numMOVA;
		unsigned long long numSGE;
		unsigned long long numSLT;
		unsigned long long numCALL;
		unsigned long long numCALLNZ;
		unsigned long long numCRS;
		unsigned long long numDCL;
		unsigned long long numDEFI;
		unsigned long long numDEFB;
		unsigned long long numELSE;
		unsigned long long numENDIF;
		unsigned long long numENDLOOP;
		unsigned long long numENDREP;
		unsigned long long numIF;
		unsigned long long numLABEL;
		unsigned long long numLOOP;
		unsigned long long numNRM;
		unsigned long long numPOW;
		unsigned long long numREP;
		unsigned long long numRET;
		unsigned long long numSGN;
		unsigned long long numSINCOS;
		unsigned long long numDP2ADD;
		unsigned long long numBREAK;
		unsigned long long numBREAKP;
		unsigned long long numSETP;
        unsigned long long numDSX;
        unsigned long long numDSY;


		unsigned long long pixelShaderSlots;
		unsigned long long vertexShaderSlots;
		unsigned long long pixelShaderType [10];
		unsigned long long vertexShaderType [6];

		InstructionType getInstructionType(std::string name);
        unsigned long long textureSamplers[16];
        int getNumStage(std::string stage);
		unsigned long long shaderSlots;
		unsigned long long acumShaders;
		unsigned long long acumShaderSlots;
		unsigned long long acumPixelShaderSlots;
		unsigned long long acumVertexShaderSlots;
		unsigned long long acumTextureInstructions;

        unsigned long long vectorInstructions;
        unsigned long long scalarInstructions;
        unsigned long long specialInstructions;
        unsigned long long specialD3D9Instructions;
		unsigned long long textureInstructions;

        unsigned long long execSlots;

        unsigned long long cDepth;

};

#endif