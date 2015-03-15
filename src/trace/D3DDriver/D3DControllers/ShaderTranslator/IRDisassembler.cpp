#include "IRDisassembler.h"
using namespace std;

void IRDisassembler::begin()
{
    relAddressNodeVisited = false;
}

void IRDisassembler::visit(VersionIRNode *n)
{
    configure(n);

    char buffer[100];
    sprintf(buffer, "%06x: %s_%d_%d", n->getOffset(), (n->getType() == VERTEX_SHADER) ? "vs" : "ps",
        D3DSHADER_VERSION_MAJOR(n->getVersion()), D3DSHADER_VERSION_MINOR(n->getVersion()));
    
    *out << endl;
    *out << buffer;    
}

void IRDisassembler::visit(IRNode *n) {}

void IRDisassembler::visit(CommentIRNode *n)
{
    char buffer[256];
    char auxBuffer[256];
    sprintf(buffer, "%06x: Comment Block.", n->getOffset());
    *out << endl;
    *out << buffer;
    
    const vector<IRNode*> commentData = n->getChilds();
    
    unsigned char c[16];

    int lineElements = 0;
    
    for(u32bit child = 0; child < commentData.size(); child++)
    {
        if (lineElements == 0)
        {
            *out << endl;
            sprintf(buffer, "%06x : ", n->getOffset());
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
}

void IRDisassembler::visit(CommentDataIRNode *n)
{
}

void IRDisassembler::visit(InstructionIRNode *n)
{
    /*string compStr;

    *out << n->getOffset() << ": INSTR   "
        << (*opcodeNames.find(n->getOpcode())).second
        << " " << n->getChilds().size();
    if(n->isCoissue())
        *out << " C";
    if(n->isPredicate())
        *out << " P";
    *out << " " << getControlStr(n);
    *out << endl;*/
    
    string opcode = (*opcodeNames.find(n->getOpcode())).second;
    string complement = getControlStr(n);
    
    char buffer[256];
    if (complement.size() > 0)    
        sprintf(buffer, "%06x : %s%s", n->getOffset(), opcode.c_str(), complement.c_str());
    else        
        sprintf(buffer, "%06x : %s", n->getOffset(), opcode.c_str());
        
    *out << endl;
    *out << buffer;
    
    preAppendComma = false;
}

void IRDisassembler::visit(DeclarationIRNode *n)
{
    char buffer[100];
    string opcode = (*opcodeNames.find(n->getOpcode())).second;
    sprintf(buffer, "%06x : %s", n->getOffset(), opcode.c_str());
    *out << endl;
    *out << buffer;
    
    preAppendComma = false;
}

void IRDisassembler::visit(DefinitionIRNode *n)
{
    char buffer[100];
    string opcode = (*opcodeNames.find(n->getOpcode())).second;
    sprintf(buffer, "%06x : %s", n->getOffset(), opcode.c_str());
    *out << endl;
    *out << buffer;

    preAppendComma = false;
}

void IRDisassembler::visit(ParameterIRNode *n)
{
    panic("IRDisassembler", "visit(ParameterIRNode)", "Only supported in derived classes.");
}

void IRDisassembler::visit(SourceParameterIRNode *n)
{
    char buffer[100];
    char auxBuffer[100];
    switch(n->getRegisterType())
    {
        case D3DSPR_TEMP: 
            sprintf(buffer, "r");
            break;
        
        case D3DSPR_INPUT:
            sprintf(buffer, "v");
            break;
            
        case D3DSPR_CONST:
        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
            sprintf(buffer, "c");
            break;
        case D3DSPR_CONSTBOOL:
            sprintf(buffer, "b");
            break;
        case D3DSPR_CONSTINT:
            sprintf(buffer, "i");
            break;
        case D3DSPR_ADDR:
            
            if (type == VERTEX_SHADER)
                sprintf(buffer, "a");
            else
                sprintf(buffer, "t");
                
            break;
                               
        case D3DSPR_SAMPLER:
            sprintf(buffer, "s");
            break;
        
        case D3DSPR_PREDICATE:
            sprintf(buffer, "p");
            break;
            
        case D3DSPR_LOOP:
            sprintf(buffer, "aL");
            break;

        case D3DSPR_MISCTYPE:
            
            //  Use a specific name based on the register offset.
            
            break;
            
        case D3DSPR_TEXCRDOUT:
        case D3DSPR_COLOROUT:            
        case D3DSPR_DEPTHOUT:
        case D3DSPR_RASTOUT:
        case D3DSPR_ATTROUT:
            panic("IRDisassembler", "visit(SourceParameterIRNode)", "Output register used as source.");
            break;
            
        case D3DSPR_TEMPFLOAT16:
        case D3DSPR_LABEL:
            sprintf(buffer, "UNK");
            break;
        default:
            panic("IRDisassembler", "visit(SourceParameterIRNode)", "Unknown register type");
            break;
    }
    
    switch(n->getRegisterType())
    {
        case D3DSPR_TEMP: 
        case D3DSPR_INPUT:
        case D3DSPR_CONST:
        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
        case D3DSPR_CONSTBOOL:
        case D3DSPR_CONSTINT:
        case D3DSPR_ADDR:
        case D3DSPR_SAMPLER:
        case D3DSPR_PREDICATE:
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s%d", auxBuffer, n->getNRegister());
            break;
            
        case D3DSPR_MISCTYPE:

            //  Select the specific name.
            switch (n->getNRegister())
            {
                case D3DSMO_POSITION:
                    sprintf(buffer, "vPos");
                    break;
                case D3DSMO_FACE:
                    sprintf(buffer, "vFace");
                    break;
                default:
                    panic("IRDisassembler", "visit(SourceParameterIRNode)",  "Undefined misc register.");
                    break;
            }

            break;
            
        case D3DSPR_LOOP:
            //  Do not write number.
            break;

        case D3DSPR_ATTROUT:
        case D3DSPR_TEXCRDOUT:
        case D3DSPR_COLOROUT:            
        case D3DSPR_RASTOUT:
        case D3DSPR_DEPTHOUT:
            panic("IRDisassembler", "visit(SourceParameterIRNode)", "Output register used as source.");
            break;
            
        case D3DSPR_TEMPFLOAT16:
        case D3DSPR_LABEL:
            //  Not supported.
            
            break;
            
        default:
            panic("IRDisassembler", "visit(SourceParameterIRNode)", "Unknown register type");
            break;
    }

    string swizzle = getSwizzleStr(n);
    
    relAddressStr.clear();
    RelativeAddressingIRNode *relAddressNode = n->getRelativeAddressing();
    if (relAddressNode != NULL)
    {
        visit(relAddressNode);
        relAddressNodeVisited = true;
    }           
    
    switch(n->getModifier())
    {
        case D3DSPSM_NONE:
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }
            
            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            
            break;
        case D3DSPSM_NEG:
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "-%s", auxBuffer);
            
            break;
            
        case D3DSPSM_BIAS:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s_bias", auxBuffer);
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            
            break;
            
        case D3DSPSM_BIASNEG:
        
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "-%s_bias", auxBuffer);
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }

            break;
            
        case D3DSPSM_SIGN:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s_sign", auxBuffer);

            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            
            break;
            
        case D3DSPSM_SIGNNEG:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "-%s_sign", auxBuffer);

            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            break;
            
        case D3DSPSM_COMP:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "1-%s", auxBuffer);

            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            break;
            
        case D3DSPSM_X2:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s_x2", auxBuffer);
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            break;
            
        case D3DSPSM_X2NEG:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "-%s_x2", auxBuffer);
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            break;
            
        case D3DSPSM_DZ:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s_dz", auxBuffer);

            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            break;
            
        case D3DSPSM_DW:
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s_dw", auxBuffer);

            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            break;
            
        case D3DSPSM_ABS:
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "|%s|", auxBuffer);
            break;
            
        case D3DSPSM_ABSNEG:
            
            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "-|%s|", auxBuffer);
            break;
            
        case D3DSPSM_NOT:

            if (relAddressStr.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
            }

            if (swizzle.size() > 0)
            {
                strcpy(auxBuffer, buffer);
                sprintf(buffer, "%s.%s", auxBuffer, swizzle.c_str());
            }
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "!%s", auxBuffer);
            break;

        default:
            panic("IRDisassembler", "visit(SourceParameterIRNode)", "Unknown source modifier.");
            break;
    }
    
    strcpy(auxBuffer, buffer);
    if (preAppendComma)
        sprintf(buffer, ", %s", auxBuffer);
    else
        sprintf(buffer, " %s", auxBuffer);
    
    *out << buffer;

    preAppendComma = true;   
}

void IRDisassembler::visit(DestinationParameterIRNode *n)
{
    char buffer[100];
    char auxBuffer[100];
    
    switch(n->getRegisterType())
    {
        case D3DSPR_TEMP: 
            sprintf(buffer, "r");
            break;
        
        case D3DSPR_INPUT:
            sprintf(buffer, "v");
            break;
            
        case D3DSPR_CONST:
        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
            sprintf(buffer, "c");
            break;
            
        case D3DSPR_CONSTBOOL:
            sprintf(buffer, "b");
            break;
            
        case D3DSPR_CONSTINT:
            sprintf(buffer, "i");
            break;
            
        case D3DSPR_ADDR:

            if (type == VERTEX_SHADER)
                sprintf(buffer, "a");
            else
                sprintf(buffer, "t");

            break;
            
        case D3DSPR_ATTROUT:
            sprintf(buffer, "oD");
            break;
            
        case D3DSPR_TEXCRDOUT:
        
            if ((type == VERTEX_SHADER) && (version >= 0x0300))
                sprintf(buffer, "o");
            else
                sprintf(buffer, "oT");
                
            break;
            
        case D3DSPR_COLOROUT:            
            sprintf(buffer, "oC");
            break;
            
        case D3DSPR_DEPTHOUT:
            sprintf(buffer, "oDepth");
            break;
        
        case D3DSPR_SAMPLER:
            sprintf(buffer, "s");
            break;
        
        case D3DSPR_PREDICATE:
            sprintf(buffer, "p");
            break;
            
        case D3DSPR_LOOP:
            sprintf(buffer, "aL");
            break;

        case D3DSPR_RASTOUT:        
        case D3DSPR_MISCTYPE:
        
            //  Use specific names based on the register offset.
            
            break;

        case D3DSPR_TEMPFLOAT16:
        case D3DSPR_LABEL:
            sprintf(buffer, "UNK");
            break;
            
        default:
            panic("IRDisassembler", "visit(DestinationParameterIRNode)", "Unknown register type");
            break;
    }
    
    switch(n->getRegisterType())
    {
        case D3DSPR_TEMP: 
        case D3DSPR_INPUT:
        case D3DSPR_CONST:
        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
        case D3DSPR_CONSTBOOL:
        case D3DSPR_CONSTINT:
        case D3DSPR_ADDR:
        case D3DSPR_ATTROUT:
        case D3DSPR_TEXCRDOUT:
        case D3DSPR_COLOROUT:            
        case D3DSPR_SAMPLER:
        case D3DSPR_PREDICATE:
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s%d", auxBuffer, n->getNRegister());
            break;
            
        case D3DSPR_RASTOUT:
        
            //  Select the specific name.
            switch(n->getNRegister())
            {
                case D3DSRO_POSITION:
                    sprintf(buffer, "oPos");
                    break;
                case D3DSRO_FOG:
                    sprintf(buffer, "oFog");
                    break;
                case D3DSRO_POINT_SIZE:
                    sprintf(buffer, "oPts");
                    break;
                default:
                    panic("IRDisassembler", "visit(DestinationParameterIRNode)",  "Undefined raster out register.");
                    break;
            }
            
            break;

        case D3DSPR_MISCTYPE:
        
            //  Select the specific name.
            switch (n->getNRegister())
            {
                case D3DSMO_POSITION:
                    sprintf(buffer, "vPos");
                    break;
                case D3DSMO_FACE:
                    sprintf(buffer, "vFace");
                    break;
                default:
                    panic("IRDisassembler", "visit(DestinationParameterIRNode)",  "Undefined misc register.");
                    break;
            }

            break;

        case D3DSPR_DEPTHOUT:
        case D3DSPR_LOOP:
            
            //  Do not write a register number.
            
            break;

        case D3DSPR_TEMPFLOAT16:
        case D3DSPR_LABEL:
            //  Not supported.
            
            break;
            
        default:
            panic("IRDisassembler", "visit(DestinationParameterIRNode)", "Unknown register type");
            break;
    }

          
    relAddressStr.clear();
    RelativeAddressingIRNode *relAddressNode = n->getRelativeAddressing();
    if (relAddressNode != NULL)
    {
        visit(relAddressNode);
        relAddressNodeVisited = true;
    }

    if (relAddressStr.size() > 0)
    {
        strcpy(auxBuffer, buffer);
        sprintf(buffer, "%s%s", auxBuffer, relAddressStr.c_str());
    }

    string writeMask = getWriteMaskStr(n);
        
    if (writeMask.size() > 0)
    {
        strcpy(auxBuffer, buffer);
        sprintf(buffer, "%s.%s", auxBuffer, writeMask.c_str());
    }
    
    string destModifier = getDestModifierStr(n);
    string shiftScaler = getShiftScaleStr(n);

    strcpy(auxBuffer, buffer);
    sprintf(buffer, "%s%s %s", destModifier.c_str(), shiftScaler.c_str(), auxBuffer);

    *out << buffer;
    
    preAppendComma = true;        
}

void IRDisassembler::visit(RelativeAddressingIRNode *n)
{
    if (!relAddressNodeVisited)
    {
        char buffer[100];
        char auxBuffer[100];
        if (n->getRegisterType() == D3DSPR_ADDR)
        {
            sprintf(buffer, "[a%d", n->getNRegister());
            switch(n->getComponentIndex())
            {
                case 0:
                    strcpy(auxBuffer, buffer);
                    sprintf(buffer, "%s.x", auxBuffer);
                    break;
                case 1:
                    strcpy(auxBuffer, buffer);
                    sprintf(buffer, "%s.y", auxBuffer);
                    break;
                case 2:
                    strcpy(auxBuffer, buffer);
                    sprintf(buffer, "%s.z", auxBuffer);
                    break;
                case 3:
                    strcpy(auxBuffer, buffer);
                    sprintf(buffer, "%s.w", auxBuffer);
                    break;
                default:
                    panic("IRPrinter", "visit(RelativeAddressignNode)", "Unknown component index.");
                    break;
            }
            strcpy(auxBuffer, buffer);
            sprintf(buffer, "%s]", auxBuffer);
        }
        else
        {
            sprintf(buffer, "REALADDR %d %d %d", n->getRegisterType(), n->getNRegister(), n->getComponentIndex());
        }
        
        relAddressStr.assign(buffer);
    }

    relAddressNodeVisited = false;
}

void IRDisassembler::visit(SamplerInfoIRNode *n)
{
    char buffer[100];
    string textureType = (*textureTypeNames.find(n->getTextureType())).second;
    
    if (textureType.size() > 0)
    {   
        sprintf(buffer, "_%s", textureType.c_str());
        *out << buffer;
    }    
}

void IRDisassembler::visit(SemanticIRNode *n)
{
    char buffer[100];
    string usageStr;
    map<DWORD, string>::iterator itU = usageNames.find(n->getUsage());
    if(itU != usageNames.end())
        usageStr = (*itU).second;
    else
        usageStr = "";

    if (usageStr.size() > 0)
    {
        switch(n->getUsage())
        {
            case 5:
                sprintf(buffer, "_%s%d", usageStr.c_str(), n->getUsageIndex());
                break;
            default:
                sprintf(buffer, "_%s", usageStr.c_str());
                break;
        }
      
        *out << buffer;
    }
}

void IRDisassembler::visit(BoolIRNode *n)
{
    bool value = static_cast<bool>(n->getToken().dword);
    
    char buffer[100];
    
    if (preAppendComma)
        sprintf(buffer, ", %s", (value == 0) ? "false" : "true");
    else
        sprintf(buffer, " %s", (value == 0) ? "false" : "true");

    *out << buffer;
    
    preAppendComma = true;
}

void IRDisassembler::visit(FloatIRNode *n)
{
    D3DToken d3dtk = n->getToken();
    float value = *reinterpret_cast<float *>(&d3dtk);

    char buffer[100];

    if (preAppendComma)
        sprintf(buffer, ", %f", value);
    else
        sprintf(buffer, " %f", value);

    *out << buffer;
    
    preAppendComma = true;
}

void IRDisassembler::visit(IntegerIRNode *n)
{
    D3DToken d3dtk = n->getToken();
    int value = *reinterpret_cast<int *>(&d3dtk);

    char buffer[100];

    if (preAppendComma)
        sprintf(buffer, ", %d", value);
    else
        sprintf(buffer, " %d", value);

    *out << buffer;
    
    preAppendComma = true;   
}



string IRDisassembler::getSwizzleStr(SourceParameterIRNode *n)
{
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

    if ((result[0] == result[1]) && (result[1] == result[2]) && (result[2] == result[3]))
    {
        result.erase(1, 3);
    }
    
    return result;
}



string IRDisassembler::getWriteMaskStr(DestinationParameterIRNode *n)
{
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

string IRDisassembler::getControlStr(InstructionIRNode *n)
{
    string result;
    DWORD spec = n->getSpecificControls();
    D3DSHADER_COMPARISON comp = n->getComparison();
    map<D3DSHADER_COMPARISON, string>::iterator itC;


    switch(n->getOpcode()) {
        case D3DSIO_TEX:
            if((type == PIXEL_SHADER) & (version >= 0x0200))
            {
                if(spec == D3DSI_TEXLD_PROJECT)
                    result = "p";
                else if(spec == D3DSI_TEXLD_BIAS)
                    result = "b";
            }
            break;
        //case D3DSIO_BREAK:
        //case D3DSIO_BREAKP:
        //case D3DSIO_ELSE:
        //case D3DSIO_ENDIF:
        //case D3DSIO_ENDREP:
        //case D3DSIO_IF:
        //case D3DSIO_LABEL:
        //case D3DSIO_ENDLOOP:

        case D3DSIO_BREAKC:
        case D3DSIO_CALL:
        case D3DSIO_CALLNZ:
        case D3DSIO_IFC:
        case D3DSIO_REP:
        case D3DSIO_SETP:
        case D3DSIO_LOOP:
        case D3DSIO_RET:
            itC = comparisonNames.find(comp);
            if(itC != comparisonNames.end())
                result = (*itC).second;
            break;
    }
    return result;
}

string IRDisassembler::getShiftScaleStr(DestinationParameterIRNode *n)
{
    string result;
    DWORD shift = n->getShiftScale();
    if(shift == 0)
        return result;

    if(shift == 0x01000000)
        result = "_2x";
    else if(shift == 0x02000000)
        result = "_4x";
    else if(shift == 0x03000000)
        result = "_8x";
    else if(shift == 0x0F000000)
        result = "_d2";
    else if(shift == 0x0E000000)
        result = "_d4";
    else if(shift == 0x0D000000)
        result = "_d8";

    return result;

}


void IRDisassembler::configure(VersionIRNode *n)
{
    type = n->getType();
    version = n->getVersion();
    
    if ((type == PIXEL_SHADER) && (version < 0x0104))
        opcodeNames[D3DSIO_TEX] = "tex";

}

std::string IRDisassembler::getDestModifierStr(DestinationParameterIRNode *n)
{
    string result;
    DWORD modifier = n->getModifier();
    if (modifier == D3DSPDM_NONE)
        return result;

    if (modifier & D3DSPDM_SATURATE)
        result.append("_sat");
    if (modifier & D3DSPDM_PARTIALPRECISION)
        result.append("_pp");
    if (modifier & D3DSPDM_MSAMPCENTROID)
        result.append("_centroid");

    return result;
}


IRDisassembler::IRDisassembler(ostream *o) : out(o)
{
    opcodeNames[D3DSIO_NOP]             = "nop";
    opcodeNames[D3DSIO_MOV]             = "mov";
    opcodeNames[D3DSIO_ADD]             = "add";
    opcodeNames[D3DSIO_SUB]             = "sub";
    opcodeNames[D3DSIO_MAD]             = "mad";
    opcodeNames[D3DSIO_MUL]             = "mul";
    opcodeNames[D3DSIO_RCP]             = "rcp";
    opcodeNames[D3DSIO_RSQ]             = "rsq";
    opcodeNames[D3DSIO_DP3]             = "dp3";
    opcodeNames[D3DSIO_DP4]             = "dp4";
    opcodeNames[D3DSIO_MIN]             = "min";
    opcodeNames[D3DSIO_MAX]             = "max";
    opcodeNames[D3DSIO_SLT]             = "slt";
    opcodeNames[D3DSIO_SGE]             = "sge";
    opcodeNames[D3DSIO_EXP]             = "exp";
    opcodeNames[D3DSIO_LOG]             = "log";
    opcodeNames[D3DSIO_LIT]             = "lit";
    opcodeNames[D3DSIO_DST]             = "dst";
    opcodeNames[D3DSIO_LRP]             = "lrp";
    opcodeNames[D3DSIO_FRC]             = "frc";
    opcodeNames[D3DSIO_M4x4]            = "M4x4";
    opcodeNames[D3DSIO_M4x3]            = "M4x3";
    opcodeNames[D3DSIO_M3x4]            = "M3x4";
    opcodeNames[D3DSIO_M3x3]            = "M3x3";
    opcodeNames[D3DSIO_M3x2]            = "M3x2";
    opcodeNames[D3DSIO_CALL]            = "call";
    opcodeNames[D3DSIO_CALLNZ]          = "callnz";
    opcodeNames[D3DSIO_LOOP]            = "loop";
    opcodeNames[D3DSIO_RET]             = "ret";
    opcodeNames[D3DSIO_ENDLOOP]         = "endloop";
    opcodeNames[D3DSIO_LABEL]           = "label";
    opcodeNames[D3DSIO_DCL]             = "dcl";
    opcodeNames[D3DSIO_POW]             = "pow";
    opcodeNames[D3DSIO_CRS]             = "crs";
    opcodeNames[D3DSIO_SGN]             = "sgn";
    opcodeNames[D3DSIO_ABS]             = "abs";
    opcodeNames[D3DSIO_NRM]             = "nrm";
    opcodeNames[D3DSIO_SINCOS]          = "sincos";
    opcodeNames[D3DSIO_REP]             = "rep";
    opcodeNames[D3DSIO_ENDREP]          = "endrep";
    opcodeNames[D3DSIO_IF]              = "if";
    opcodeNames[D3DSIO_IFC]             = "if";
    opcodeNames[D3DSIO_ELSE]            = "else";
    opcodeNames[D3DSIO_ENDIF]           = "endif";
    opcodeNames[D3DSIO_BREAK]           = "break";
    opcodeNames[D3DSIO_BREAKC]          = "breakc";
    opcodeNames[D3DSIO_MOVA]            = "mova";
    opcodeNames[D3DSIO_DEFB]            = "defb";
    opcodeNames[D3DSIO_DEFI]            = "defi";
    opcodeNames[D3DSIO_TEXCOORD]        = "texcoord";
    opcodeNames[D3DSIO_TEXKILL]         = "texkill";
    opcodeNames[D3DSIO_TEX]             = "texld";
    opcodeNames[D3DSIO_TEXBEM]          = "texbem";
    opcodeNames[D3DSIO_TEXBEML]         = "texbeml";
    opcodeNames[D3DSIO_TEXREG2AR]       = "texreg2ar";
    opcodeNames[D3DSIO_TEXREG2GB]       = "texreg2gb";
    opcodeNames[D3DSIO_TEXM3x2PAD]      = "TEXM3x2PAD";
    opcodeNames[D3DSIO_TEXM3x2TEX]      = "TEXM3x2TEX";
    opcodeNames[D3DSIO_TEXM3x3PAD]      = "TEXM3x3PAD";
    opcodeNames[D3DSIO_TEXM3x3TEX]      = "TEXM3x3TEX";
    opcodeNames[D3DSIO_RESERVED0]       = "RESERVED0";
    opcodeNames[D3DSIO_TEXM3x3SPEC]     = "TEXM3x3SPEC";
    opcodeNames[D3DSIO_TEXM3x3VSPEC]    = "TEXM3x3VSPEC";
    opcodeNames[D3DSIO_EXPP]            = "expp";
    opcodeNames[D3DSIO_LOGP]            = "logp";
    opcodeNames[D3DSIO_CND]             = "cnd";
    opcodeNames[D3DSIO_DEF]             = "def";
    opcodeNames[D3DSIO_TEXREG2RGB]      = "texreg2rgb";
    opcodeNames[D3DSIO_TEXDP3TEX]       = "texdp3tex";
    opcodeNames[D3DSIO_TEXM3x2DEPTH]    = "TEXM3x2DEPTH";
    opcodeNames[D3DSIO_TEXDP3]          = "texdp3";
    opcodeNames[D3DSIO_TEXM3x3]         = "TEXM3x3";
    opcodeNames[D3DSIO_TEXDEPTH]        = "texdepth";
    opcodeNames[D3DSIO_CMP]             = "cmp";
    opcodeNames[D3DSIO_BEM]             = "bem";
    opcodeNames[D3DSIO_DP2ADD]          = "dp2add";
    opcodeNames[D3DSIO_DSX]             = "dsx";
    opcodeNames[D3DSIO_DSY]             = "dsy";
    opcodeNames[D3DSIO_TEXLDD]          = "texldd";
    opcodeNames[D3DSIO_SETP]            = "setp";
    opcodeNames[D3DSIO_TEXLDL]          = "texldl";
    opcodeNames[D3DSIO_BREAKP]          = "breakp";
    opcodeNames[D3DSIO_PHASE]           = "phase";
    opcodeNames[D3DSIO_COMMENT]         = "comment";
    opcodeNames[D3DSIO_END]             = "end";

    //comparisonNames[ D3DSPC_RESERVED0 ] = "RESERVED0";
    comparisonNames[ D3DSPC_RESERVED0 ] = "";
    comparisonNames[ D3DSPC_GT ] = "_gt";
    comparisonNames[ D3DSPC_EQ ] = "_eq";
    comparisonNames[ D3DSPC_GE ] = "_ge";
    comparisonNames[ D3DSPC_LT ] = "_lt";
    comparisonNames[ D3DSPC_NE ] = "_ne";
    comparisonNames[ D3DSPC_LE ] = "_le";
    //comparisonNames[ D3DSPC_RESERVED1 ] = "RESERVED1";
    comparisonNames[ D3DSPC_RESERVED1 ] = "";

    textureTypeNames[ D3DSTT_UNKNOWN ] = "UNKNOWN";
    textureTypeNames[ D3DSTT_2D      ] = "2d";
    textureTypeNames[ D3DSTT_CUBE    ] = "cube";
    textureTypeNames[ D3DSTT_VOLUME  ] = "volume";

    usageNames[0]  = "position";
    usageNames[1]  = "blendweight";
    usageNames[2]  = "blendindices";
    usageNames[3]  = "normal";
    usageNames[4]  = "psize";
    usageNames[5]  = "texcoord";
    usageNames[6]  = "tangent";
    usageNames[7]  = "binormal";
    usageNames[8]  = "tessfactor";
    usageNames[9]  = "positiont";
    usageNames[10] = "color";
    usageNames[11] = "fog";
    usageNames[12] = "depth";
    usageNames[13] = "sample";
}

void IRDisassembler::visit(EndIRNode *n)
{
}
