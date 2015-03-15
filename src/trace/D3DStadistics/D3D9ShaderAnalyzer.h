/*****************************************************************************
Class that performs analysis on a shader.

Dependencies: windows.h, map

Author: José Manuel Solís
*****************************************************************************/

#ifndef __D3D9SHADERANALYZER
#define __D3D9SHADERANALYZER

#include <string>

class D3D9ShaderAnalyzer {
public:
	struct D3D9ShaderAnalysis {
		UINT instructionSlots;
		UINT samples;
		std::map< UINT, UINT > samplerUsage;
		D3D9ShaderAnalysis();
	};

	/*********************************************************
	Analyze shader. Returns false if can determine some data from
	analysis.
	*********************************************************/
	bool analyze(DWORD *shaderFunction, D3D9ShaderAnalysis *analysis);
private:
	enum InstructionType {
		TEX,
		TEXBEM,
		TEXBEML,
		TEXLD,
		TEXLDB,
		TEXLDD,
		TEXLDL,
		TEXLDP,
		TEXM3X2TEX,
		TEXM3X3SPEC,
		TEXM3X3TEX,
		TEXM3X3VSPEC,
		TEXREG2AR,
		TEXREG2GB,
		TEXREG2RGB,
		OTHER
	};

	InstructionType getInstructionType(std::string name);
	bool getSamplerNumber(std::string reg, UINT *num);
};

#endif
