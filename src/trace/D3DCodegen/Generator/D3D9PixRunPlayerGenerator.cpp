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

#include "Items/ClassDescription.hpp"
#include "Items/MethodDescription.hpp"
#include "Utilities/String.hpp"
#include "Utilities/FileSystem.hpp"

#include "Parser/DXHParser.hpp"
#include "Generator/IGenerator.hpp"
#include <algorithm>
#include "Generator/D3D9PixRunPlayerGenerator.hpp"


using namespace dxcodegen;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;
using namespace dxcodegen::Generator;
using namespace dxcodegen::Utilities;
using namespace std;

void PerParameterGenerator::setParameter(MethodDescriptionParam p) {
    param = p;

    // Remove "CONST " from the type because we won't use it and it
    // forces us to initialize some variables.
    param.type = removeConst(param.type);

    // A REFGUID or RIID variable is a reference to a GUID and must be initialized,
    // so we translate it to GUID.
    param.type = translateReferencesToGuid(param.type);
}



void PerParameterGenerator::generateCodeBeforeCall(ofstream *out) {

    bool isVoid;
    bool isHiddenPointer;
    bool isLockedRect;
    bool isLockedBox;

    switch(getType()) {
        case VALUE_PARAMETER:

            isVoid = (param.type.find("void") != param.type.npos);

            isHiddenPointer = (param.type.find("HDC") != param.type.npos) |
                              (param.type.find("HWND") != param.type.npos) |
                              (param.type.find("HMONITOR") != param.type.npos);

            if (isVoid || isHiddenPointer || isIrregularPointer(param.type))
            {
                /***********************
                EXAMPLE:

                    HANDLE *SizeToLock

                    PIXPointer ov_SizeToLock;
                    HANDLE *sv_SizeToLock;
                    reader.readParameter<PIXPointer>(&ov_SizeToLock);
                    sv_SizeToLock = (HANLDE *) ov_SizeToLock;
                ***********************/

                *out << "PIXPointer ov_" << param.name << ";" << endl;
                *out << param.type << " sv_" << param.name << ";" << endl;
                *out << "reader.readParameter<PIXPointer>(&"
                    << "ov_" << param.name << ");" << endl;
                *out << "sv_" << param.name << " = (" << param.type << ") pointer(ov_" << param.name << ");"
                    << endl << endl;
            }
            else
            {
                /***********************
                EXAMPLE:

                    UINT SizeToLock

                    UINT ov_SizeToLock;
                    UINT sv_SizeToLock;
                    reader.readParameter<UINT>(&ov_SizeToLock);
                    sv_SizeToLock = ov_SizeToLock;
                ***********************/

                *out << param.type << " ov_" << param.name << ";" << endl;
                *out << param.type << " sv_" << param.name << ";" << endl;
                *out << "reader.readParameter<" << param.type << ">(&"
                    << "ov_" << param.name << ");" << endl;
                *out << "sv_" << param.name << " = " << "ov_" << param.name << ";"
                    << endl << endl;
            }
            break;

        case VALUE_POINTER_PARAMETER:

            isLockedRect = (param.type.find("D3DLOCKED_RECT") != param.type.npos);
            isLockedBox = (param.type.find("D3DLOCKED_BOX") != param.type.npos);

            if (isLockedRect)
            {
                /***************************
                EXAMPLE:

                    D3DLOCKED_RECT * pLockedRect;

                    PIXPointer opv_pLockedRect;
                    D3DLOCKED_RECT_COMP_WIN32 ov_pLockedRect;
                    D3DLOCKED_RECT * spv_pLockedRect;
                    D3DLOCKED_RECT   sv_pLockedRect;
                    reader.readParameter<PIXPointer>(&opv_pLockedRect);
                    reader.readParameter<D3DLOCKED_RECT_COMP_WIN32>(&ov_pLockedRect);
                    spv_pLockedRect = ((opv_pLockedRect == 0 ) ? 0 : &sv_pLockedRect);
                    sv_pLockedRect.Pitch = ov_pLockedRect.Pitch;
                    sv_pLockedRect.pBits = (void *) pointer(ov_pLockedRect.pBits);
                ***************************/
                *out << "PIXPointer opv_" << param.name << ";" << endl;
                *out << "D3DLOCKED_RECT_COMP_WIN32 ov_" << param.name << ";" << endl;
                *out << param.type << " spv_" << param.name << ";" << endl;
                *out << removeLastAsterisk(param.type) << " sv_" << param.name << ";" << endl;
                *out << "reader.readParameter<PIXPointer>(&"
                    << "opv_" << param.name << ");" << endl;
                *out << "reader.readParameter<D3DLOCKED_RECT_COMP_WIN32>(&"
                    << "ov_" << param.name << ");" << endl;
                *out << "spv_" << param.name << " = " << "(opv_" << param.name << " == 0 ) ? 0: &sv_"
                    << param.name << ";" << endl;
                *out << "sv_" << param.name << ".Pitch = " << "ov_" << param.name << ".Pitch;" << endl;
                *out << "sv_" << param.name << ".pBits = " << "(void *) pointer(ov_" << param.name << ".pBits);" << endl;
                *out << endl;
            }
            else if (isLockedBox)
            {
                /***************************
                EXAMPLE:

                    D3DLOCKED_BOX * pLockedBox;

                    PIXPointer opv_pLockedBox;
                    D3DLOCKED_BOX_COMP_WIN32 ov_pLockedBox;
                    D3DLOCKED_BOX * spv_pLockedBox;
                    D3DLOCKED_BOX   sv_pLockedBox;
                    reader.readParameter<PIXPointer>(&opv_pLockedBox);
                    reader.readParameter<D3DLOCKED_BOX_COMP_WIN32>(&ov_pLockedBox);
                    spv_pLockedBox = ((opv_pLockedBox == 0 ) ? 0 : &sv_pLockedBox);
                    sv_pLockedBox.RowPitch = ov_pLockedBox.RowPitch;
                    sv_pLockedBox.SlicePitch = ov_pLockedBox.SlicePitch;
                    sv_pLockedBox.pBits = (void *) pointer(ov_pLockedBox.pBits);
                ***************************/
                *out << "PIXPointer opv_" << param.name << ";" << endl;
                *out << "D3DLOCKED_BOX_COMP_WIN32 ov_" << param.name << ";" << endl;
                *out << param.type << " spv_" << param.name << ";" << endl;
                *out << removeLastAsterisk(param.type) << " sv_" << param.name << ";" << endl;
                *out << "reader.readParameter<PIXPointer>(&"
                    << "opv_" << param.name << ");" << endl;
                *out << "reader.readParameter<D3DLOCKED_BOX_COMP_WIN32>(&"
                    << "ov_" << param.name << ");" << endl;
                *out << "spv_" << param.name << " = " << "(opv_" << param.name << " == 0 ) ? 0: &sv_"
                    << param.name << ";" << endl;
                *out << "sv_" << param.name << ".RowPitch = " << "ov_" << param.name << ".RowPitch;" << endl;
                *out << "sv_" << param.name << ".SlicePitch = " << "ov_" << param.name << ".SlicePitch;" << endl;
                *out << "sv_" << param.name << ".pBits = " << "(void *) pointer(ov_" << param.name << ".pBits);" << endl;
                *out << endl;
            }
            else
            {
                /***************************
                EXAMPLE:

                    D3DMATRIX * pMatrix;

                    PIXPointer opv_pMatrix;
                    D3DMATRIX   ov_pMatrix;
                    D3DMATRIX * spv_pMatrix;
                    D3DMATRIX   sv_pMatrix;
                    reader.readParameter<PIXPointer>(&opv_pMatrix);
                    reader.readParameter<D3DMATRIX>(&ov_pMatrix);
                    spv_pMatrix = ((opv_pMatrix == 0 ) ? 0 : &sv_pMatrix);
                    sv_pMatrix = ov_pMatrix;
                ***************************/
                *out << "PIXPointer opv_" << param.name << ";" << endl;
                *out << removeLastAsterisk(param.type) << " ov_" << param.name << ";" << endl;
                *out << param.type << " spv_" << param.name << ";" << endl;
                *out << removeLastAsterisk(param.type) << " sv_" << param.name << ";" << endl;
                *out << "reader.readParameter<PIXPointer>(&"
                    << "opv_" << param.name << ");" << endl;
                *out << "reader.readParameter<" << removeLastAsterisk(param.type) << ">(&"
                    << "ov_" << param.name << ");" << endl;
                *out << "spv_" << param.name << " = " << "(opv_" << param.name << " == 0 ) ? 0: &sv_"
                    << param.name << ";" << endl;
                *out << "sv_" << param.name << " = " << "ov_" << param.name << ";"
                    << endl << endl;
            }

            break;

        case INTERFACE_POINTER_PARAMETER:

            /***************************
            EXAMPLE:

                IDirect3DTexture9 * pTexture;

                PIXPointer oip_pTexture;
                IDirect3DTexture9 * sip_pTexture;
                reader.readParameter<PIXPointer>(&oip_pTexture);
                sip_pTexture = static_cast<IDirect3DTexture9 *>(status.getSubstitute(oip_pTexture));
            ***************************/

            *out << "PIXPointer oip_" << param.name << ";" << endl;
            *out << param.type << " sip_" << param.name << ";" << endl;
            *out << "reader.readParameter<PIXPointer>(&"
                << "oip_" << param.name << ");" << endl;
            *out << "sip_" << param.name << " = " << "static_cast<"
                << param.type << ">(status.getSubstitute(oip_"
                << param.name << "));"
                << endl << endl;
            break;
        case POINTER_INTERFACE_POINTER_PARAMETER:

            /***************************
            EXAMPLE:

                IDirect3DTexture9 * ppTexture;

                PIXPointer opip_ppTexture;
                PIXPointer oip_ppTexture;
                IDirect3DTexture9 ** spip_ppTexture;
                IDirect3DTexture9 *sip_ppTexture;
                reader.readParameter<PIXPointer>(&opip_ppTexture);
                reader.readParameter<PIXPointer>(&oip_ppTexture);
                spip_ppTexture = &sip_ppTexture;
            *****************************/

            *out << "PIXPointer opip_" << param.name  << ";" << endl;
            *out << "PIXPointer oip_" << param.name << ";" << endl;
            *out << param.type << " spip_" << param.name  << ";" << endl;
            *out << removeLastAsterisk(param.type) << " sip_" << param.name << ";" << endl;

            *out << "reader.readParameter<PIXPointer>(&"
                << "opip_" << param.name << ");" << endl;
            *out << "reader.readParameter<PIXPointer>(&"
                << "oip_" << param.name << ");" << endl;
            *out << "spip_" << param.name << " = " << "&sip_" << param.name << ";"
                << endl << endl;
            break;
    }

}

string PerParameterGenerator::removeConst( string s) {
    size_t offset = s.find("CONST ");
    if( offset != s.npos )
        return s.substr(offset + strlen("CONST ") - 1 );
    else return s;
}

string PerParameterGenerator::translateReferencesToGuid(string s) {
    if ( ( s.find("REFGUID") != s.npos ) | (s.find("REFIID") != s.npos ) )
        return "GUID";
    else
        return s;
}

bool PerParameterGenerator::isIrregularPointer(string s) {
    return (
        (s.find("HANDLE") != string::npos) |
        (s.find("RGNDATA") != string::npos) |
        (s.find("D3DVERTEXELEMENT9") != string::npos) |
        (s.find("DWORD") != string::npos)
        );
}


string PerParameterGenerator::removeLastAsterisk( string s) {
    size_t offset = s.find_last_of('*');
    return s.substr(0, offset);
}

void PerParameterGenerator::generateCodeAfterCall(ofstream *out) {
    switch (getType()) {
        case POINTER_INTERFACE_POINTER_PARAMETER:
			*out << "if (sv_Return == D3D_OK)" << endl;
            *out << "    status.setSubstitute("
                << "oip_" << param.name << ", "
                << "sip_" << param.name << ");"
                << endl << endl;
			break;
    }
}

void PerParameterGenerator::generateCodeForCall(ofstream *out) {
    switch(getType()) {
        case VALUE_PARAMETER:
            *out << "sv_" << param.name ;
            break;
        case INTERFACE_POINTER_PARAMETER:
            *out << "sip_" << param.name;
            break;
        case POINTER_INTERFACE_POINTER_PARAMETER:
            *out << "spip_" << param.name;
            break;
        case VALUE_POINTER_PARAMETER:
            *out << "spv_" << param.name;
            break;
    }
}

PerParameterGenerator::ParameterType PerParameterGenerator::getType() {

    bool isInterface = (
        ( param.type.find("IDirect3D9") != param.type.npos ) |
        ( param.type.find("IDirect3DDevice9") != param.type.npos ) |
        ( param.type.find("IDirect3DSwapChain9") != param.type.npos ) |
        ( param.type.find("IDirect3DTexture9") != param.type.npos ) |
        ( param.type.find("IDirect3DVolumeTexture9") != param.type.npos ) |
        ( param.type.find("IDirect3DCubeTexture9") != param.type.npos ) |
        ( param.type.find("IDirect3DVertexBuffer9") != param.type.npos ) |
        ( param.type.find("IDirect3DIndexBuffer9") != param.type.npos ) |
        ( param.type.find("IDirect3DSurface9") != param.type.npos ) |
        ( param.type.find("IDirect3DVolume9") != param.type.npos ) |
        ( param.type.find("IDirect3DVertexDeclaration9") != param.type.npos ) |
        ( param.type.find("IDirect3DVertexShader9") != param.type.npos ) |
        ( param.type.find("IDirect3DPixelShader9") != param.type.npos ) |
        ( param.type.find("IDirect3DStateBlock9") != param.type.npos ) |
        ( param.type.find("IDirect3DQuery9") != param.type.npos ) |
        ( param.type.find("IDirect3DBaseTexture9") != param.type.npos ) |
        ( param.type.find("IDirect3DResource9") != param.type.npos )
        );

    bool isVoid = (param.type.find("void") != param.type.npos);

    if (param.type.find_last_of('*') != param.type.npos ) {
        if (isIrregularPointer(param.type))
            return VALUE_PARAMETER;
        else {
            string derreferenced = removeLastAsterisk(param.type);
            if (derreferenced.find_last_of('*') != param.type.npos ) {
                if( isInterface ) return POINTER_INTERFACE_POINTER_PARAMETER;
                else return VALUE_PARAMETER;
            }
            else {
                if( isInterface ) return INTERFACE_POINTER_PARAMETER;
                else if(isVoid) return VALUE_PARAMETER;
                else return VALUE_POINTER_PARAMETER;
            }
        }
    }
    else
        return VALUE_PARAMETER;
}

void PerMethodGenerator::generateSwitchBranch(ofstream *out) {
        *out    << "case " << composeFieldName()  << ":" << endl
                << composeMethodName() << "();" << endl
                << "break;" << endl;
}

void PerMethodGenerator::generateEnumField(ofstream *out) {
    *out << composeFieldName() << " = " << exMethod.CID << "," << endl;
}

void PerMethodGenerator::generateDeclaration(ofstream *out) {
    *out << "void " << composeMethodName() << "();" << endl;
}

void PerMethodGenerator::generateDefinition(ofstream *out) {

    *out << "void D3D9PixRunPlayer::" << composeMethodName() << "() {" << endl << endl;

    // Panic
    *out << "#ifdef " << composeMethodName() << "_PANIC" << endl;
    *out << "panic(\"D3D9PixRunPlayer\", \"" << composeMethodName() << "\""
            ", \"Not supported D3D9 call found in trace file.\");" << endl;
    *out << "#endif" << endl << endl;

    // Variables needed for autogeneration
    bool isVoidReturn = false;
    bool isHRESULTReturn = false;
    bool hasParams = false;
    MethodDescriptionParam returnDescription;
    MethodDescriptionParam thisDescription;

    // Update variables needed by autogeneration
    if(exMethod.autogenerate) {
        isVoidReturn = (method.GetType().find("void") != method.GetType().npos);
        isHRESULTReturn = (method.GetType().find("HRESULT") != method.GetType().npos);
        returnDescription.type = method.GetType();
        returnDescription.name = "Return";
        thisDescription.type = exMethod.ClassName;
        thisDescription.type.append(" *");
        thisDescription.name = "This";

        /** @fix
         *  DXHParser returns a param with name
         * "void" when there are no params.
         */
        hasParams = method.GetParam(0).name != "void";
    }


    // GENERATE CODE BEFORE CALL

    if(exMethod.autogenerate) {
        // Auto generate a parameter for hold the return value if needed
        if( !isVoidReturn ) {
            perParameterGenerator.setParameter(returnDescription);
            perParameterGenerator.generateCodeBeforeCall(out);
        }
        //  Auto generate a parameter for hold 'this' pointer
        perParameterGenerator.setParameter(thisDescription);
        perParameterGenerator.generateCodeBeforeCall(out);

        // Auto generate the others
        if( hasParams ) {
            for(unsigned int i = 0;i < method.GetParamsCount(); i++) {
                perParameterGenerator.setParameter(method.GetParam(i));
                perParameterGenerator.generateCodeBeforeCall(out);
            }
        }
    }

    *out << "#ifdef " << composeMethodName() << "_SPECIFIC_PRE" << endl;
    *out << composeMethodName() << "_SPECIFIC_PRE" << endl;
    *out << "#endif" << endl << endl;


    *out << "#ifdef " << composeMethodName() << "_USER_PRE" << endl;
    *out << composeMethodName() << "_USER_PRE" << endl;
    *out << "#endif" << endl << endl;

    // GENERATE CODE FOR THE CALL

    if(exMethod.autogenerate)
    {
        //  Generate check for the interface pointer.
        *out << "if (";
        perParameterGenerator.setParameter(thisDescription);
        perParameterGenerator.generateCodeForCall(out);
        *out << " == NULL)" << endl;
        *out << "{" << endl;
        *out << "    includelog::logfile().write(includelog::Panic, \"Ignoring call to NULL interface pointer for ";
        *out << composeMethodName() << " interface call.\\n\");" << endl;
        *out << "    if (!status.isEnabledContinueOnNULLHack())" << endl;
        *out << "        panic(\"D3D9PixRunPlayer\", \"" << composeMethodName() << "\""
            ", \"Calling to D3D9 interface with NULL pointer.\");" << endl;
        if (!isVoidReturn)
        {
            *out << "    sv_Return = -1;" << endl;
        }
        *out << "}" << endl;
        *out << "else" << endl;
        *out << "    ";

        if(!isVoidReturn)
        {
            perParameterGenerator.setParameter(returnDescription);
            perParameterGenerator.generateCodeForCall(out);
            *out << " = ";
        }


        perParameterGenerator.setParameter(thisDescription);
        perParameterGenerator.generateCodeForCall(out);

        *out << " -> ";

        *out << method.GetName() << "(" << endl;
        if( hasParams ) {
            for(unsigned int i = 0;i < method.GetParamsCount(); i++) {
                *out << ((i == 0)?"":" ,");
                perParameterGenerator.setParameter(method.GetParam(i));
                perParameterGenerator.generateCodeForCall(out);
            }
        }
        *out << ");" << endl << endl;
    }

    // GENERATE CODE AFTER THE CALL

    *out << "#ifdef " << composeMethodName() << "_USER_POST" << endl;
    *out << composeMethodName() << "_USER_POST" << endl;
    *out << "#endif" << endl << endl;

    *out << "#ifdef " << composeMethodName() << "_SPECIFIC_POST" << endl;
    *out << composeMethodName() << "_SPECIFIC_POST" << endl;
    *out << "#endif" << endl << endl;

    //  Generate code for release.
    if(exMethod.autogenerate)
    {
        if (method.GetName().compare("Release") == 0)
        {
            *out << "if (sv_Return == 0)" << endl;
            *out << "    status.removeSubstitute(oip_" << thisDescription.name << ");" << endl;
        }
    }
        
    if(exMethod.autogenerate)
    {
        if (isHRESULTReturn)
        {
            *out << "if (sv_Return != D3D_OK)" << endl;
            *out << "    includelog::logfile().write(includelog::Panic, \"Call to ";
            *out << composeMethodName() << " didn't return OK.\\n\");" << endl << endl;
        }
    }
        
    if(exMethod.autogenerate) {
        if( hasParams ) {
            for(unsigned int i = 0;i < method.GetParamsCount(); i++) {
                perParameterGenerator.setParameter(method.GetParam(i));
                perParameterGenerator.generateCodeAfterCall(out);
            }
        }
    }

    *out << "}" << endl << endl;
}

void PerMethodGenerator::setMethodInfo( Items::MethodDescription &md,
            ExtendedMethodDescription &exmd) {
    exMethod = exmd;
    method = md;
}

string PerMethodGenerator::composeMethodName() {

    string className = exMethod.ClassName;
    string methodName = method.GetName();
    String::ToUpper(className);
    String::ToUpper(methodName);
    string result = "D3D9OP_";
    if(className.length() > 0) {
        result.append(className);
        result.append("_");
    }
    result.append(methodName);

    return result;
}

string PerMethodGenerator::composeFieldName() {

    // Uppercase
    string className = exMethod.ClassName;
    String::ToUpper( className );
    string methodName = method.GetName();
    String::ToUpper( methodName );

    // Concat
    string result;
    result.append("D3D9CID_");
    if(className.length() > 0) {
        result.append(className);
        result.append("_");
    }
    result.append(methodName);

    return result;
}


void D3D9PixRunPlayerGenerator::GenerateCode() {

    /**************************************************************
    Open streams
    **************************************************************/

    string &path   = configuration.GetOutputPath();
    string pathDec = path;
    string pathDef = path;
    string pathSw  = path;
    string pathEn  = path;

    pathDec.append("D3D9PixRunDeclarations.gen");
    pathDef.append("D3D9PixRunDefinitions.gen");
    pathSw.append("D3D9PixRunSwitchBranches.gen");
    pathEn.append("D3D9PixRunEnum.gen");

    ofstream *outDec = CreateFilename(pathDec);
    ofstream *outDef = CreateFilename(pathDef);
    ofstream *outSw = CreateFilename(pathSw);
    ofstream *outEn = CreateFilename(pathEn);


    /**************************************************************
    Iterate through classes and methods generating code using method generator
    **************************************************************/
    vector< ClassDescription > :: iterator itC;

    for( itC = classes.begin(); itC != classes.end(); itC++ ) {

        for( unsigned int i = 0; i < (*itC).GetMethodsCount(); i++ ) {

            // Get CID of this method
            string methodClassName = (*itC).GetName();
            string methodName = (*itC).GetMethod(i).GetName();
            pair<string, string> name;
            name.first = methodClassName;
            name.second = methodName;
            int CID = nameToCID[name];

            // Get extended method description
            ExtendedMethodDescription exMethod = extendedMethodDescriptions[CID];


            // Configure perMethodGenerator
            perMethodGenerator.setMethodInfo((*itC).GetMethod(i), exMethod);

            //Generate code
            perMethodGenerator.generateDeclaration(outDec);
            perMethodGenerator.generateDefinition(outDef);
            perMethodGenerator.generateSwitchBranch(outSw);
            perMethodGenerator.generateEnumField(outEn);
        }
    }

    // Free resources
    CloseFilename(outDec);
    CloseFilename(outDef);
    CloseFilename(outSw);
    CloseFilename(outEn);
}



void D3D9PixRunPlayerGenerator::setClasses(vector<ClassDescription> &c) {
    classes = c;
    onSetClasses();
}

void D3D9PixRunPlayerGenerator::onSetClasses() {

    /***************************************************************
    Global methods (p.e. Direct3DCreate9) not listed by DXHParser,
    we will hardcode its info in a additional class with empty name.
    ***************************************************************/
    ClassDescription global;
    global.SetName("");

    MethodDescription method;
    MethodDescriptionParam param;

    method.SetName("Direct3DCreate9");
    method.SetType("IDirect3D9 *");
    param.name = "SDKVersion";
    param.type = "UINT";
    method.AddParam(param);
    global.AddMethod(method);

    method.SetName("DebugSetMute");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("UNUSED_208");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("Direct3DShaderValidatorCreate9");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("UNUSED_210");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("UNUSED_211");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_BeginEvent");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_EndEvent");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_SetMarker");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_SetRegion");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_QueryRepeatFrame");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_SetOptions");
    method.SetType("");
    global.AddMethod(method);

    method.SetName("D3DPERF_GetStatus");
    method.SetType("");
    global.AddMethod(method);

    classes.push_back(global);

    /************************************************************
    Class methods as have correlative CID's, we must only hardcode
    the first CID of each class.
    ************************************************************/
    std::map< std::string , int > classToInitialCID;

    classToInitialCID[ "" ] = 206;
    classToInitialCID[ "IDirect3D9" ] = 219;
    classToInitialCID[ "IDirect3DDevice9" ] = 236;
    classToInitialCID[ "IDirect3DSwapChain9" ] = 355;
    classToInitialCID[ "IDirect3DTexture9" ] = 365;
    classToInitialCID[ "IDirect3DVolumeTexture9" ] = 387;
    classToInitialCID[ "IDirect3DCubeTexture9" ] = 409;
    classToInitialCID[ "IDirect3DVertexBuffer9" ] = 431;
    classToInitialCID[ "IDirect3DIndexBuffer9" ] = 445;
    classToInitialCID[ "IDirect3DSurface9" ] = 459;
    classToInitialCID[ "IDirect3DVolume9" ] = 476;
    classToInitialCID[ "IDirect3DVertexDeclaration9" ] = 487;
    classToInitialCID[ "IDirect3DVertexShader9" ] = 492;
    classToInitialCID[ "IDirect3DPixelShader9" ] = 497;
    classToInitialCID[ "IDirect3DStateBlock9" ] = 502;
    classToInitialCID[ "IDirect3DQuery9" ] = 508;

    /**************************************************************
    Iterate through classes and methods assigning CID's and names
    **************************************************************/
    vector< ClassDescription > :: iterator itC;

    for( itC = classes.begin(); itC != classes.end(); itC++ ) {
        string className = (*itC).GetName();
        int iniCID = classToInitialCID[ className ];
        for( unsigned int i = 0; i < (*itC).GetMethodsCount(); i++ ) {
                string methodName = (*itC).GetMethod(i).GetName();
                nameToCID[pair<string, string>(className,methodName)] = classToInitialCID[ className ] + i;
        }
    }

    /**************************************************************
    Iterate through classes and methods assigning default extended method description
    **************************************************************/

    for( itC = classes.begin(); itC != classes.end(); itC++ ) {
        string className = (*itC).GetName();
        int iniCID = classToInitialCID[ className ];
        for( unsigned int i = 0; i < (*itC).GetMethodsCount(); i++ ) {
                string methodName = (*itC).GetMethod(i).GetName();
                ExtendedMethodDescription extended;
                extended.CID = nameToCID[pair<string, string>(className,methodName)];
                extended.ClassName = className;
                extendedMethodDescriptions[extended.CID] = extended;
        }
    }


    /******************************************************************
    Hardcode extended method description for some operations
    *******************************************************************/

    // Autogenerated
    extendedMethodDescriptions[220].autogenerate = true;
    extendedMethodDescriptions[221].autogenerate = true;
    extendedMethodDescriptions[235].autogenerate = true;
    extendedMethodDescriptions[237].autogenerate = true;
    extendedMethodDescriptions[238].autogenerate = true;
    extendedMethodDescriptions[241].autogenerate = true;
    extendedMethodDescriptions[242].autogenerate = true;
    extendedMethodDescriptions[246].autogenerate = true;
    extendedMethodDescriptions[247].autogenerate = true;
    extendedMethodDescriptions[248].autogenerate = true;
    extendedMethodDescriptions[249].autogenerate = true;
    extendedMethodDescriptions[250].autogenerate = true;
    extendedMethodDescriptions[252].autogenerate = true;
    extendedMethodDescriptions[253].autogenerate = true;
    extendedMethodDescriptions[254].autogenerate = true;
    extendedMethodDescriptions[256].autogenerate = true;
    extendedMethodDescriptions[257].autogenerate = true;
    extendedMethodDescriptions[259].autogenerate = true;
    extendedMethodDescriptions[260].autogenerate = true;
    extendedMethodDescriptions[261].autogenerate = true;
    extendedMethodDescriptions[262].autogenerate = true;
    extendedMethodDescriptions[263].autogenerate = true;
    extendedMethodDescriptions[264].autogenerate = true;
    extendedMethodDescriptions[265].autogenerate = true;
    extendedMethodDescriptions[266].autogenerate = true;
    extendedMethodDescriptions[267].autogenerate = true;
    extendedMethodDescriptions[270].autogenerate = true;
    extendedMethodDescriptions[271].autogenerate = true;
    extendedMethodDescriptions[272].autogenerate = true;
    extendedMethodDescriptions[273].autogenerate = true;
    extendedMethodDescriptions[274].autogenerate = true;
    extendedMethodDescriptions[275].autogenerate = true;
    extendedMethodDescriptions[276].autogenerate = true;
    extendedMethodDescriptions[277].autogenerate = true;
    extendedMethodDescriptions[278].autogenerate = true;
    extendedMethodDescriptions[279].autogenerate = true;
    extendedMethodDescriptions[280].autogenerate = true;
    extendedMethodDescriptions[282].autogenerate = true;
    extendedMethodDescriptions[283].autogenerate = true;
    extendedMethodDescriptions[285].autogenerate = true;
    extendedMethodDescriptions[287].autogenerate = true;
    extendedMethodDescriptions[289].autogenerate = true;
    extendedMethodDescriptions[291].autogenerate = true;
    extendedMethodDescriptions[293].autogenerate = true;
    extendedMethodDescriptions[295].autogenerate = true;
    extendedMethodDescriptions[296].autogenerate = true;
    extendedMethodDescriptions[297].autogenerate = true;
    extendedMethodDescriptions[298].autogenerate = true;
    extendedMethodDescriptions[300].autogenerate = true;
    extendedMethodDescriptions[301].autogenerate = true;
    extendedMethodDescriptions[303].autogenerate = true;
    extendedMethodDescriptions[305].autogenerate = true;
    extendedMethodDescriptions[307].autogenerate = true;
    extendedMethodDescriptions[309].autogenerate = true;
    extendedMethodDescriptions[311].autogenerate = true;
    extendedMethodDescriptions[313].autogenerate = true;
    extendedMethodDescriptions[315].autogenerate = true;
    extendedMethodDescriptions[317].autogenerate = true;
    extendedMethodDescriptions[318].autogenerate = true;
    extendedMethodDescriptions[319].autogenerate = true;
    extendedMethodDescriptions[320].autogenerate = true;
    extendedMethodDescriptions[321].autogenerate = true;
    extendedMethodDescriptions[322].autogenerate = true;
    extendedMethodDescriptions[323].autogenerate = true;
    extendedMethodDescriptions[324].autogenerate = true;
    extendedMethodDescriptions[325].autogenerate = true;
    extendedMethodDescriptions[327].autogenerate = true;
    extendedMethodDescriptions[328].autogenerate = true;
    extendedMethodDescriptions[329].autogenerate = true;
    extendedMethodDescriptions[330].autogenerate = true;
    extendedMethodDescriptions[332].autogenerate = true;
    extendedMethodDescriptions[334].autogenerate = true;
    extendedMethodDescriptions[336].autogenerate = true;
    extendedMethodDescriptions[337].autogenerate = true;
    extendedMethodDescriptions[338].autogenerate = true;
    extendedMethodDescriptions[340].autogenerate = true;
    extendedMethodDescriptions[341].autogenerate = true;
    extendedMethodDescriptions[342].autogenerate = true;
    extendedMethodDescriptions[343].autogenerate = true;
    extendedMethodDescriptions[344].autogenerate = true;
    extendedMethodDescriptions[345].autogenerate = true;
    extendedMethodDescriptions[347].autogenerate = true;
    extendedMethodDescriptions[349].autogenerate = true;
    extendedMethodDescriptions[353].autogenerate = true;
    extendedMethodDescriptions[356].autogenerate = true;
    extendedMethodDescriptions[357].autogenerate = true;
    extendedMethodDescriptions[358].autogenerate = true;
    extendedMethodDescriptions[360].autogenerate = true;
    extendedMethodDescriptions[363].autogenerate = true;
    extendedMethodDescriptions[366].autogenerate = true;
    extendedMethodDescriptions[367].autogenerate = true;
    extendedMethodDescriptions[368].autogenerate = true;
    extendedMethodDescriptions[372].autogenerate = true;
    extendedMethodDescriptions[374].autogenerate = true;
    extendedMethodDescriptions[376].autogenerate = true;
    extendedMethodDescriptions[379].autogenerate = true;
    extendedMethodDescriptions[381].autogenerate = true;
    extendedMethodDescriptions[383].autogenerate = true;
    extendedMethodDescriptions[384].autogenerate = true;
    extendedMethodDescriptions[385].autogenerate = true;
    extendedMethodDescriptions[386].autogenerate = true;
    extendedMethodDescriptions[388].autogenerate = true;
    extendedMethodDescriptions[389].autogenerate = true;
    extendedMethodDescriptions[390].autogenerate = true;
    extendedMethodDescriptions[394].autogenerate = true;
    extendedMethodDescriptions[396].autogenerate = true;
    extendedMethodDescriptions[398].autogenerate = true;
    extendedMethodDescriptions[401].autogenerate = true;
    extendedMethodDescriptions[403].autogenerate = true;
    extendedMethodDescriptions[405].autogenerate = true;
    extendedMethodDescriptions[406].autogenerate = true;
    extendedMethodDescriptions[407].autogenerate = true;
    extendedMethodDescriptions[408].autogenerate = true;
    extendedMethodDescriptions[410].autogenerate = true;
    extendedMethodDescriptions[411].autogenerate = true;
    extendedMethodDescriptions[412].autogenerate = true;
    extendedMethodDescriptions[416].autogenerate = true;
    extendedMethodDescriptions[418].autogenerate = true;
    extendedMethodDescriptions[420].autogenerate = true;
    extendedMethodDescriptions[423].autogenerate = true;
    extendedMethodDescriptions[425].autogenerate = true;
    extendedMethodDescriptions[427].autogenerate = true;
    extendedMethodDescriptions[428].autogenerate = true;
    extendedMethodDescriptions[429].autogenerate = true;
    extendedMethodDescriptions[430].autogenerate = true;
    extendedMethodDescriptions[432].autogenerate = true;
    extendedMethodDescriptions[433].autogenerate = true;
    extendedMethodDescriptions[434].autogenerate = true;
    extendedMethodDescriptions[438].autogenerate = true;
    extendedMethodDescriptions[440].autogenerate = true;
    extendedMethodDescriptions[442].autogenerate = true;
    extendedMethodDescriptions[443].autogenerate = true;
    extendedMethodDescriptions[446].autogenerate = true;
    extendedMethodDescriptions[447].autogenerate = true;
    extendedMethodDescriptions[448].autogenerate = true;
    extendedMethodDescriptions[452].autogenerate = true;
    extendedMethodDescriptions[454].autogenerate = true;
    extendedMethodDescriptions[456].autogenerate = true;
    extendedMethodDescriptions[457].autogenerate = true;
    extendedMethodDescriptions[460].autogenerate = true;
    extendedMethodDescriptions[461].autogenerate = true;
    extendedMethodDescriptions[462].autogenerate = true;
    extendedMethodDescriptions[466].autogenerate = true;
    extendedMethodDescriptions[468].autogenerate = true;
    extendedMethodDescriptions[470].autogenerate = true;
    extendedMethodDescriptions[472].autogenerate = true;
    extendedMethodDescriptions[473].autogenerate = true;
    extendedMethodDescriptions[477].autogenerate = true;
    extendedMethodDescriptions[478].autogenerate = true;
    extendedMethodDescriptions[479].autogenerate = true;
    extendedMethodDescriptions[483].autogenerate = true;
    extendedMethodDescriptions[485].autogenerate = true;
    extendedMethodDescriptions[486].autogenerate = true;
    extendedMethodDescriptions[488].autogenerate = true;
    extendedMethodDescriptions[489].autogenerate = true;
    extendedMethodDescriptions[490].autogenerate = true;
    extendedMethodDescriptions[493].autogenerate = true;
    extendedMethodDescriptions[494].autogenerate = true;
    extendedMethodDescriptions[495].autogenerate = true;
    extendedMethodDescriptions[498].autogenerate = true;
    extendedMethodDescriptions[499].autogenerate = true;
    extendedMethodDescriptions[500].autogenerate = true;
    extendedMethodDescriptions[503].autogenerate = true;
    extendedMethodDescriptions[504].autogenerate = true;
    extendedMethodDescriptions[505].autogenerate = true;
    extendedMethodDescriptions[506].autogenerate = true;
    extendedMethodDescriptions[507].autogenerate = true;

}
