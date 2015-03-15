#include "Common.h"
#include "ToString.h"


string opcode2str(D3DSHADER_INSTRUCTION_OPCODE_TYPE opc) {
    switch(opc) {
        case D3DSIO_NOP : return "D3DSIO_NOP"; break;
        case D3DSIO_MOV : return "D3DSIO_MOV"; break;
        case D3DSIO_ADD : return "D3DSIO_ADD"; break;
        case D3DSIO_SUB : return "D3DSIO_SUB"; break;
        case D3DSIO_MAD : return "D3DSIO_MAD"; break;
        case D3DSIO_RCP : return "D3DSIO_RCP"; break;
        case D3DSIO_RSQ : return "D3DSIO_RSQ"; break;
        case D3DSIO_DP3 : return "D3DSIO_DP3"; break;
        case D3DSIO_DP4 : return "D3DSIO_DP4"; break;
        case D3DSIO_MIN : return "D3DSIO_MIN"; break;
        case D3DSIO_MAX : return "D3DSIO_MAX"; break;
        case D3DSIO_SLT : return "D3DSIO_SLT"; break;
        case D3DSIO_SGE : return "D3DSIO_SGE"; break;
        case D3DSIO_EXP : return "D3DSIO_EXP"; break;
        case D3DSIO_LOG : return "D3DSIO_LOG"; break;
        case D3DSIO_LIT : return "D3DSIO_LIT"; break;
        case D3DSIO_DST : return "D3DSIO_DST"; break;
        case D3DSIO_LRP : return "D3DSIO_LRP"; break;
        case D3DSIO_FRC : return "D3DSIO_FRC"; break;
        case D3DSIO_M4x4 : return "D3DSIO_M4x4"; break;
        case D3DSIO_M4x3 : return "D3DSIO_M4x3"; break;
        case D3DSIO_M3x4 : return "D3DSIO_M3x4"; break;
        case D3DSIO_M3x3 : return "D3DSIO_M3x3"; break;
        case D3DSIO_M3x2 : return "D3DSIO_M3x2"; break;
        case D3DSIO_CALL : return "D3DSIO_CALL"; break;
        case D3DSIO_CALLNZ : return "D3DSIO_CALLNZ"; break;
        case D3DSIO_LOOP : return "D3DSIO_LOOP"; break;
        case D3DSIO_RET : return "D3DSIO_RET"; break;
        case D3DSIO_ENDLOOP : return "D3DSIO_ENDLOOP"; break;
        case D3DSIO_LABEL : return "D3DSIO_LABEL"; break;
        case D3DSIO_DCL : return "D3DSIO_DCL"; break;
        case D3DSIO_POW : return "D3DSIO_POW"; break;
        case D3DSIO_CRS : return "D3DSIO_CRS"; break;
        case D3DSIO_SGN : return "D3DSIO_SGN"; break;
        case D3DSIO_ABS : return "D3DSIO_ABS"; break;
        case D3DSIO_NRM : return "D3DSIO_NRM"; break;
        case D3DSIO_SINCOS : return "D3DSIO_SINCOS"; break;
        case D3DSIO_REP : return "D3DSIO_REP"; break;
        case D3DSIO_ENDREP : return "D3DSIO_ENDREP"; break;
        case D3DSIO_IF : return "D3DSIO_IF"; break;
        case D3DSIO_IFC : return "D3DSIO_IFC"; break;
        case D3DSIO_ELSE : return "D3DSIO_ELSE"; break;
        case D3DSIO_ENDIF : return "D3DSIO_ENDIF"; break;
        case D3DSIO_BREAK : return "D3DSIO_BREAK"; break;
        case D3DSIO_BREAKC : return "D3DSIO_BREAKC"; break;
        case D3DSIO_MOVA : return "D3DSIO_MOVA"; break;
        case D3DSIO_DEFB : return "D3DSIO_DEFB"; break;
        case D3DSIO_DEFI : return "D3DSIO_DEFI"; break;

        case D3DSIO_TEXCOORD : return "D3DSIO_TEXCOORD"; break;
        case D3DSIO_TEXKILL : return "D3DSIO_TEXKILL"; break;
        case D3DSIO_TEX : return "D3DSIO_TEX"; break;
        case D3DSIO_TEXBEM : return "D3DSIO_TEXBEM"; break;
        case D3DSIO_TEXBEML : return "D3DSIO_TEXBEML"; break;
        case D3DSIO_TEXREG2AR : return "D3DSIO_TEXREG2AR"; break;
        case D3DSIO_TEXREG2GB : return "D3DSIO_TEXREG2GB"; break;
        case D3DSIO_TEXM3x2PAD : return "D3DSIO_TEXM3x2PAD"; break;
        case D3DSIO_TEXM3x2TEX : return "D3DSIO_TEXM3x2TEX"; break;
        case D3DSIO_TEXM3x3PAD : return "D3DSIO_TEXM3x3PAD"; break;
        case D3DSIO_TEXM3x3TEX : return "D3DSIO_TEXM3x3TEX"; break;
        case D3DSIO_RESERVED0 : return "D3DSIO_RESERVED0"; break;
        case D3DSIO_TEXM3x3SPEC : return "D3DSIO_TEXM3x3SPEC"; break;
        case D3DSIO_TEXM3x3VSPEC : return "D3DSIO_TEXM3x3VSPEC"; break;
        case D3DSIO_EXPP : return "D3DSIO_EXPP"; break;
        case D3DSIO_LOGP : return "D3DSIO_LOGP"; break;
        case D3DSIO_CND : return "D3DSIO_CND"; break;
        case D3DSIO_DEF : return "D3DSIO_DEF"; break;
        case D3DSIO_TEXREG2RGB : return "D3DSIO_TEXREG2RGB"; break;
        case D3DSIO_TEXDP3TEX : return "D3DSIO_TEXDP3TEX"; break;
        case D3DSIO_TEXM3x2DEPTH : return "D3DSIO_TEXM3x2DEPTH"; break;
        case D3DSIO_TEXDP3 : return "D3DSIO_TEXDP3"; break;
        case D3DSIO_TEXM3x3 : return "D3DSIO_TEXM3x3"; break;
        case D3DSIO_TEXDEPTH : return "D3DSIO_TEXDEPTH"; break;
        case D3DSIO_CMP : return "D3DSIO_CMP"; break;
        case D3DSIO_BEM : return "D3DSIO_BEM"; break;
        case D3DSIO_DP2ADD : return "D3DSIO_DP2ADD"; break;
        case D3DSIO_DSX : return "D3DSIO_DSX"; break;
        case D3DSIO_DSY : return "D3DSIO_DSY"; break;
        case D3DSIO_TEXLDD : return "D3DSIO_TEXLDD"; break;
        case D3DSIO_SETP : return "D3DSIO_SETP"; break;
        case D3DSIO_TEXLDL : return "D3DSIO_TEXLDL"; break;
        case D3DSIO_BREAKP : return "D3DSIO_BREAKP"; break;

        case D3DSIO_PHASE : return "D3DSIO_PHASE"; break;
        case D3DSIO_COMMENT : return "D3DSIO_COMMENT"; break;
        case D3DSIO_END : return "D3DSIO_END"; break;
        default: return "Unknown"; break;
    }
}

string renderstate2string(D3DRENDERSTATETYPE state) {
    switch(state) {
        case D3DRS_ZENABLE: return "D3DRS_ZENABLE"; break;
        case D3DRS_FILLMODE: return "D3DRS_FILLMODE"; break;
        case D3DRS_SHADEMODE: return "D3DRS_SHADEMODE"; break;
        case D3DRS_ZWRITEENABLE: return "D3DRS_ZWRITEENABLE"; break;
        case D3DRS_ALPHATESTENABLE: return "D3DRS_ALPHATESTENABLE"; break;
        case D3DRS_LASTPIXEL: return "D3DRS_LASTPIXEL"; break;
        case D3DRS_SRCBLEND: return "D3DRS_SRCBLEND"; break;
        case D3DRS_DESTBLEND: return "D3DRS_DESTBLEND"; break;
        case D3DRS_CULLMODE: return "D3DRS_CULLMODE"; break;
        case D3DRS_ZFUNC: return "D3DRS_ZFUNC"; break;
        case D3DRS_ALPHAREF: return "D3DRS_ALPHAREF"; break;
        case D3DRS_ALPHAFUNC: return "D3DRS_ALPHAFUNC"; break;
        case D3DRS_DITHERENABLE: return "D3DRS_DITHERENABLE"; break;
        case D3DRS_ALPHABLENDENABLE: return "D3DRS_ALPHABLENDENABLE"; break;
        case D3DRS_FOGENABLE: return "D3DRS_FOGENABLE"; break;
        case D3DRS_SPECULARENABLE: return "D3DRS_SPECULARENABLE"; break;
        case D3DRS_FOGCOLOR: return "D3DRS_FOGCOLOR"; break;
        case D3DRS_FOGTABLEMODE: return "D3DRS_FOGTABLEMODE"; break;
        case D3DRS_FOGSTART: return "D3DRS_FOGSTART"; break;
        case D3DRS_FOGEND: return "D3DRS_FOGEND"; break;
        case D3DRS_FOGDENSITY: return "D3DRS_FOGDENSITY"; break;
        case D3DRS_RANGEFOGENABLE: return "D3DRS_RANGEFOGENABLE"; break;
        case D3DRS_STENCILENABLE: return "D3DRS_STENCILENABLE"; break;
        case D3DRS_STENCILFAIL: return "D3DRS_STENCILFAIL"; break;
        case D3DRS_STENCILZFAIL: return "D3DRS_STENCILZFAIL"; break;
        case D3DRS_STENCILPASS: return "D3DRS_STENCILPASS"; break;
        case D3DRS_STENCILFUNC: return "D3DRS_STENCILFUNC"; break;
        case D3DRS_STENCILREF: return "D3DRS_STENCILREF"; break;
        case D3DRS_STENCILMASK: return "D3DRS_STENCILMASK"; break;
        case D3DRS_STENCILWRITEMASK: return "D3DRS_STENCILWRITEMASK"; break;
        case D3DRS_TEXTUREFACTOR: return "D3DRS_TEXTUREFACTOR"; break;
        case D3DRS_WRAP0: return "D3DRS_WRAP0"; break;
        case D3DRS_WRAP1: return "D3DRS_WRAP1"; break;
        case D3DRS_WRAP2: return "D3DRS_WRAP2"; break;
        case D3DRS_WRAP3: return "D3DRS_WRAP3"; break;
        case D3DRS_WRAP4: return "D3DRS_WRAP4"; break;
        case D3DRS_WRAP5: return "D3DRS_WRAP5"; break;
        case D3DRS_WRAP6: return "D3DRS_WRAP6"; break;
        case D3DRS_WRAP7: return "D3DRS_WRAP7"; break;
        case D3DRS_CLIPPING: return "D3DRS_CLIPPING"; break;
        case D3DRS_LIGHTING: return "D3DRS_LIGHTING"; break;
        case D3DRS_AMBIENT: return "D3DRS_AMBIENT"; break;
        case D3DRS_FOGVERTEXMODE: return "D3DRS_FOGVERTEXMODE"; break;
        case D3DRS_COLORVERTEX: return "D3DRS_COLORVERTEX"; break;
        case D3DRS_LOCALVIEWER: return "D3DRS_LOCALVIEWER"; break;
        case D3DRS_NORMALIZENORMALS: return "D3DRS_NORMALIZENORMALS"; break;
        case D3DRS_DIFFUSEMATERIALSOURCE: return "D3DRS_DIFFUSEMATERIALSOURCE"; break;
        case D3DRS_SPECULARMATERIALSOURCE: return "D3DRS_SPECULARMATERIALSOURCE"; break;
        case D3DRS_AMBIENTMATERIALSOURCE: return "D3DRS_AMBIENTMATERIALSOURCE"; break;
        case D3DRS_EMISSIVEMATERIALSOURCE: return "D3DRS_EMISSIVEMATERIALSOURCE"; break;
        case D3DRS_VERTEXBLEND: return "D3DRS_VERTEXBLEND"; break;
        case D3DRS_CLIPPLANEENABLE: return "D3DRS_CLIPPLANEENABLE"; break;
        case D3DRS_POINTSIZE: return "D3DRS_POINTSIZE"; break;
        case D3DRS_POINTSIZE_MIN: return "D3DRS_POINTSIZE_MIN"; break;
        case D3DRS_POINTSPRITEENABLE: return "D3DRS_POINTSPRITEENABLE"; break;
        case D3DRS_POINTSCALEENABLE: return "D3DRS_POINTSCALEENABLE"; break;
        case D3DRS_POINTSCALE_A: return "D3DRS_POINTSCALE_A"; break;
        case D3DRS_POINTSCALE_B: return "D3DRS_POINTSCALE_B"; break;
        case D3DRS_POINTSCALE_C: return "D3DRS_POINTSCALE_C"; break;
        case D3DRS_MULTISAMPLEANTIALIAS: return "D3DRS_MULTISAMPLEANTIALIAS"; break;
        case D3DRS_MULTISAMPLEMASK: return "D3DRS_MULTISAMPLEMASK"; break;
        case D3DRS_PATCHEDGESTYLE: return "D3DRS_PATCHEDGESTYLE"; break;
        case D3DRS_DEBUGMONITORTOKEN: return "D3DRS_DEBUGMONITORTOKEN"; break;
        case D3DRS_POINTSIZE_MAX: return "D3DRS_POINTSIZE_MAX"; break;
        case D3DRS_INDEXEDVERTEXBLENDENABLE: return "D3DRS_INDEXEDVERTEXBLENDENABLE"; break;
        case D3DRS_COLORWRITEENABLE: return "D3DRS_COLORWRITEENABLE"; break;
        case D3DRS_TWEENFACTOR: return "D3DRS_TWEENFACTOR"; break;
        case D3DRS_BLENDOP: return "D3DRS_BLENDOP"; break;
        case D3DRS_POSITIONDEGREE: return "D3DRS_POSITIONDEGREE"; break;
        case D3DRS_NORMALDEGREE: return "D3DRS_NORMALDEGREE"; break;
        case D3DRS_SCISSORTESTENABLE: return "D3DRS_SCISSORTESTENABLE"; break;
        case D3DRS_SLOPESCALEDEPTHBIAS: return "D3DRS_SLOPESCALEDEPTHBIAS"; break;
        case D3DRS_ANTIALIASEDLINEENABLE: return "D3DRS_ANTIALIASEDLINEENABLE"; break;
        case D3DRS_MINTESSELLATIONLEVEL: return "D3DRS_MINTESSELLATIONLEVEL"; break;
        case D3DRS_MAXTESSELLATIONLEVEL: return "D3DRS_MAXTESSELLATIONLEVEL"; break;
        case D3DRS_ADAPTIVETESS_X: return "D3DRS_ADAPTIVETESS_X"; break;
        case D3DRS_ADAPTIVETESS_Y: return "D3DRS_ADAPTIVETESS_Y"; break;
        case D3DRS_ADAPTIVETESS_Z: return "D3DRS_ADAPTIVETESS_Z"; break;
        case D3DRS_ADAPTIVETESS_W: return "D3DRS_ADAPTIVETESS_W"; break;
        case D3DRS_ENABLEADAPTIVETESSELLATION: return "D3DRS_ENABLEADAPTIVETESSELLATION"; break;
        case D3DRS_TWOSIDEDSTENCILMODE: return "D3DRS_TWOSIDEDSTENCILMODE"; break;
        case D3DRS_CCW_STENCILFAIL: return "D3DRS_CCW_STENCILFAIL"; break;
        case D3DRS_CCW_STENCILZFAIL: return "D3DRS_CCW_STENCILZFAIL"; break;
        case D3DRS_CCW_STENCILPASS: return "D3DRS_CCW_STENCILPASS"; break;
        case D3DRS_CCW_STENCILFUNC: return "D3DRS_CCW_STENCILFUNC"; break;
        case D3DRS_COLORWRITEENABLE1: return "D3DRS_COLORWRITEENABLE1"; break;
        case D3DRS_COLORWRITEENABLE2: return "D3DRS_COLORWRITEENABLE2"; break;
        case D3DRS_COLORWRITEENABLE3: return "D3DRS_COLORWRITEENABLE3"; break;
        case D3DRS_BLENDFACTOR: return "D3DRS_BLENDFACTOR"; break;
        case D3DRS_SRGBWRITEENABLE: return "D3DRS_SRGBWRITEENABLE"; break;
        case D3DRS_DEPTHBIAS: return "D3DRS_DEPTHBIAS"; break;
        case D3DRS_WRAP8: return "D3DRS_WRAP8"; break;
        case D3DRS_WRAP9: return "D3DRS_WRAP9"; break;
        case D3DRS_WRAP10: return "D3DRS_WRAP10"; break;
        case D3DRS_WRAP11: return "D3DRS_WRAP11"; break;
        case D3DRS_WRAP12: return "D3DRS_WRAP12"; break;
        case D3DRS_WRAP13: return "D3DRS_WRAP13"; break;
        case D3DRS_WRAP14: return "D3DRS_WRAP14"; break;
        case D3DRS_WRAP15: return "D3DRS_WRAP15"; break;
        case D3DRS_SEPARATEALPHABLENDENABLE: return "D3DRS_SEPARATEALPHABLENDENABLE"; break;
        case D3DRS_SRCBLENDALPHA: return "D3DRS_SRCBLENDALPHA"; break;
        case D3DRS_DESTBLENDALPHA: return "D3DRS_DESTBLENDALPHA"; break;
        case D3DRS_BLENDOPALPHA: return "D3DRS_BLENDOPALPHA"; break;
        default: return "unknown";
    }
}

string registertype2string(D3DSHADER_PARAM_REGISTER_TYPE type) {
    switch (type) {
        case D3DSPR_TEMP: return "D3DSPR_TEMP"; break;
        case D3DSPR_INPUT: return "D3DSPR_INPUT"; break;
        case D3DSPR_CONST: return "D3DSPR_CONST"; break;
        case D3DSPR_ADDR: return "D3DSPR_ADDR or D3DSPR_TEXTURE"; break;
        case D3DSPR_RASTOUT: return "D3DSPR_RASTOUT"; break;
        case D3DSPR_ATTROUT: return "D3DSPR_ATTROUT"; break;
        case D3DSPR_TEXCRDOUT: return "D3DSPR_TEXCRDOUT or D3DSPR_OUTPUT"; break;
        case D3DSPR_CONSTINT: return "D3DSPR_CONSTINT"; break;
        case D3DSPR_COLOROUT: return "D3DSPR_COLOROUT"; break;
        case D3DSPR_DEPTHOUT: return "D3DSPR_DEPTHOUT"; break;
        case D3DSPR_SAMPLER: return "D3DSPR_SAMPLER"; break;
        case D3DSPR_CONST2: return "D3DSPR_CONST2"; break;
        case D3DSPR_CONST3: return "D3DSPR_CONST3"; break;
        case D3DSPR_CONST4: return "D3DSPR_CONST4"; break;
        case D3DSPR_CONSTBOOL: return "D3DSPR_CONSTBOOL"; break;
        case D3DSPR_LOOP: return "D3DSPR_LOOP"; break;
        case D3DSPR_TEMPFLOAT16: return "D3DSPR_TEMPFLOAT16"; break;
        case D3DSPR_MISCTYPE: return "D3DSPR_MISCTYPE"; break;
        case D3DSPR_LABEL: return "D3DSPR_LABEL"; break;
        case D3DSPR_PREDICATE: return "D3DSPR_PREDICATE"; break;
        default : return "unknown";
    }
}

string samplerstate2string(D3DSAMPLERSTATETYPE state) {
    switch(state) {
        case D3DSAMP_ADDRESSU: return "D3DSAMP_ADDRESSU"; break;
        case D3DSAMP_ADDRESSV: return "D3DSAMP_ADDRESSV"; break;
        case D3DSAMP_ADDRESSW: return "D3DSAMP_ADDRESSW"; break;
        case D3DSAMP_BORDERCOLOR: return "D3DSAMP_BORDERCOLOR"; break;
        case D3DSAMP_MAGFILTER: return "D3DSAMP_MAGFILTER"; break;
        case D3DSAMP_MINFILTER: return "D3DSAMP_MINFILTER"; break;
        case D3DSAMP_MIPFILTER: return "D3DSAMP_MIPFILTER"; break;
        case D3DSAMP_MIPMAPLODBIAS: return "D3DSAMP_MIPMAPLODBIAS"; break;
        case D3DSAMP_MAXMIPLEVEL: return "D3DSAMP_MAXMIPLEVEL"; break;
        case D3DSAMP_MAXANISOTROPY: return "D3DSAMP_MAXANISOTROPY"; break;
        case D3DSAMP_SRGBTEXTURE: return "D3DSAMP_SRGBTEXTURE"; break;
        case D3DSAMP_ELEMENTINDEX: return "D3DSAMP_ELEMENTINDEX"; break;
        case D3DSAMP_DMAPOFFSET: return "D3DSAMP_DMAPOFFSET"; break;
        default: return "unknown";
    }
}


