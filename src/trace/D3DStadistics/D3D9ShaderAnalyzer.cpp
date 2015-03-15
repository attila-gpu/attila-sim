#include "StdAfx.h"
#include <map>
#include "D3D9ShaderAnalyzer.h"
#include <string>
#include <vector>
#include <fstream>
#include "StringUtils.h"

using namespace std;

D3D9ShaderAnalyzer::D3D9ShaderAnalysis::D3D9ShaderAnalysis():
  instructionSlots(0), samples(0) {}


bool D3D9ShaderAnalyzer::getSamplerNumber(string reg, UINT *num) {
	if(reg.size() != 2) return false;

	const char *creg = reg.c_str();

	if((creg[0] == 't') | (creg[0] == 's')) {
		*num = atoi(&creg[1]);
		return true;
	}
	else
		return false;
}

D3D9ShaderAnalyzer::InstructionType D3D9ShaderAnalyzer::getInstructionType(string name) {
	
	if( name == "tex") return TEX;
	else if( name == "texbem") return TEXBEM;
	else if( name == "texbeml") return TEXBEML;
	else if( name == "texld") return TEXLD;
	else if( name == "texldb") return TEXLDB;
	else if( name == "texldd") return TEXLDD;
	else if( name == "texldl") return TEXLDL;
	else if( name == "texldp") return TEXLDP;
	else if( name == "texm3x2tex") return TEXM3X2TEX;
	else if( name == "texm3x3spec") return TEXM3X3SPEC;
	else if( name == "texm3x3tex") return TEXM3X3TEX;
	else if( name == "texm3x3vspec") return TEXM3X3VSPEC;
	else if( name == "texreg2ar") return TEXREG2AR;
	else if( name == "texreg2gb") return TEXREG2GB;
	else if( name == "texreg2rgb") return TEXREG2RGB;
	else return OTHER;
}

bool D3D9ShaderAnalyzer::analyze(DWORD *shaderFunction, D3D9ShaderAnalysis *analysis) {
	// Disassemble shader

	ID3DXBuffer *disassembledShader;
	if(S_OK != D3DXDisassembleShader(shaderFunction, false, 0, &disassembledShader))
		return false;
	string disassembledStr;
	disassembledStr.append((char *)disassembledShader->GetBufferPointer());

	disassembledShader->Release();

	// Divide it into lines.
	vector <string> lines = explode(disassembledStr, "\n");
	string line;
	vector <string> words;

	// Sampler usage section

	analysis->samples = 0;
	analysis->samplerUsage.clear();

	for(int i = 0; i < (lines.size() - 1); i ++) {
		line = lines[i];
		words = explode_by_any(line," \t,");
		if(words.size() > 0) {
			UINT samplerNumber;
			bool sample = false;
			InstructionType t = getInstructionType(words[0]);
			switch( t ) {
				case TEX :
				case TEXBEM :
				case TEXBEML :
				case TEXLD :
				case TEXM3X2TEX :
				case TEXM3X3SPEC :
				case TEXM3X3TEX :
				case TEXM3X3VSPEC :
				case TEXREG2AR :
				case TEXREG2GB :
				case TEXREG2RGB :
					if( words.size() > 1)
						if(getSamplerNumber(words[1], &samplerNumber))
							sample = true;
					// textld in PS_2_0 and up takes another form
					if( t == TEXLD )
						if( words.size() > 3)
							if(getSamplerNumber(words[3], &samplerNumber))
								sample = true;
					break;
				case TEXLDB :
				case TEXLDD :
				case TEXLDL :
				case TEXLDP :
					if( words.size() > 3)
						if(getSamplerNumber(words[3], &samplerNumber))
							sample = true;
					break;
			}

			if (sample) {
				analysis->samples ++;
				if(analysis->samplerUsage.find(samplerNumber) != analysis->samplerUsage.end())
					analysis->samplerUsage[samplerNumber] ++;
				else
					analysis->samplerUsage[samplerNumber] = 1;
			}
		}
	}
	
	// D3DXDisassembleShader puts a comment
	// in the last line telling about approximate instruction slots used.
	line = lines[lines.size() - 1];
	// Approximate count can be get from the third word
	words = explode(line, " ");
	if(words.size() < 3)
		return false;
	int c = atoi(words[2].c_str()); 
	if (c == 0)
		return false;
	analysis->instructionSlots = c;

	return true;
}
