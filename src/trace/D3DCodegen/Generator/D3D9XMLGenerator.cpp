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
#include "Items/StructDescription.hpp"
#include "Items/EnumDescription.hpp"
#include "Utilities/String.hpp"
#include "Utilities/FileSystem.hpp"
#include "Generator/IGenerator.hpp"
#include "Generator/D3D9XMLGenerator.hpp"

#include <iostream>


using namespace dxcodegen;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;
using namespace dxcodegen::Generator;
using namespace dxcodegen::Utilities;
using namespace std;

D3D9XMLGenerator::D3D9XMLGenerator() {
    /*
     * Hardcode inheritance info 
     */
    inheritance["IDirect3D9"] =  "IUnknown";
    inheritance["IDirect3DDevice9"] =  "IUnknown";
    inheritance["IDirect3DSwapChain9"] =  "IUnknown";
    inheritance["IDirect3DTexture9"] =  "IDirect3DBaseTexture9";
    inheritance["IDirect3DVolumeTexture9"] =  "IDirect3DBaseTexture9";
    inheritance["IDirect3DCubeTexture9"] =  "IDirect3DBaseTexture9";
    inheritance["IDirect3DVertexBuffer9"] =  "IDirect3DResource9";
    inheritance["IDirect3DIndexBuffer9"] =  "IDirect3DResource9";
    inheritance["IDirect3DSurface9"] =  "IDirect3DResource9";
    inheritance["IDirect3DVolume9"] =  "IDirect3DResource9";
    inheritance["IDirect3DVertexDeclaration9"] =  "IUnknown";
    inheritance["IDirect3DVertexShader9"] =  "IUnknown";
    inheritance["IDirect3DPixelShader9"] =  "IUnknown";
    inheritance["IDirect3DStateBlock9"] =  "IUnknown";
    inheritance["IDirect3DQuery9"] =  "IUnknown";
    inheritance["IDirect3DBaseTexture9"] =  "IDirect3DResource9";
    inheritance["IDirect3DResource9"] =  "IUnknown";
}

void D3D9XMLGenerator::setClasses(vector<ClassDescription> &c) {
    classes = c;
}

void D3D9XMLGenerator::setStructs(vector<StructDescription> &s) {
    structs = s;
}

void D3D9XMLGenerator::setEnums(vector<EnumDescription> &e) {
    enums = e;
}

void D3D9XMLGenerator::GenerateCode() {
    ofstream *out = CreateFilename("d3d9api.xml");
    
    *out << "<?xml version=\"1.0\"?>" << endl;
    *out << "<d3d9api>" << endl;

    *out << "<structs>" << endl;

    vector<StructDescription>::iterator itS;
    for(itS = structs.begin(); itS != structs.end(); itS ++) {
        generateStruct(out, *itS);
    }

    *out << "</structs>" << endl;

    *out << "<enums>" << endl;

    vector<EnumDescription>::iterator itE;
    for(itE = enums.begin(); itE != enums.end(); itE ++) {
        generateEnum(out, *itE);
    }

    *out << "</enums>" << endl;

    *out << "<interfaces>" << endl;

    vector<ClassDescription>::iterator itC;
    for(itC = classes.begin(); itC != classes.end(); itC ++) {
        generateClass(out, *itC);
    }

    *out << "</interfaces>" << endl;

    *out << "<functions>" << endl;

    generateFunctions(out);

    *out << "</functions>" << endl;

    *out << "</d3d9api>" << endl;

    CloseFilename(out);
}


void D3D9XMLGenerator::generateMethod(ostream *o, Items::MethodDescription &m) {
    *o << "<method name=\"" << m.GetName() << "\">" << endl;
    *o << "<return type=\"" << m.GetType() << "\"/>" << endl;

    /** @fix
    *  DXHParser returns a param with name
    * "void" when there are no params.
    */
    if(m.GetParam(0).name != "void") {
        *o << "<parameters>" << endl;
        for(unsigned int i = 0; i < m.GetParamsCount(); i ++) {
            MethodDescriptionParam p = m.GetParam(i);
            *o << "<parameter name=\"" << p.name <<
                "\" type=\"" << p.type << "\"/>" << endl;
        }
        *o << "</parameters>" << endl;
    }
    *o << "</method>" << endl;
}

void D3D9XMLGenerator::generateClass(ostream *o, Items::ClassDescription &c) {
    *o << "<interface name=\"" << c.GetName() <<
        "\" base=\"" << inheritance[c.GetName()] << "\">" << endl;

    for(unsigned int i = 0; i < c.GetMethodsCount(); i ++) {
        generateMethod(o, c.GetMethod(i));
    }

    *o << "</interface>" << endl;

}

void D3D9XMLGenerator::generateEnum(ostream *o, Items::EnumDescription &e) {
    *o << "<enum name=\"" << e.GetName() << "\">" << endl;
    for(unsigned int i= 0 ; i < e.GetMembersCount(); i ++) {
        *o << "<member name=\"" << e.GetMember(i).name << "\"/>" << endl;
    }
    *o << "</enum>" << endl;

}

void D3D9XMLGenerator::generateStruct(ostream *o, Items::StructDescription &s) {
    *o << "<struct name=\"" << s.GetName() << "\">" << endl;
    for(unsigned int i= 0 ; i < s.GetMemberCount(); i ++) {
        StructDescriptionMember sdm = s.GetMember(i);
            *o << "<member name=\"" << sdm.name
            << "\" type=\"" << sdm.type <<"\"/>" << endl;
    }
    *o << "</struct>" << endl;
}

void D3D9XMLGenerator::generateFunctions(ostream *o) {
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

    method.SetName("Direct3DShaderValidatorCreate9");
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

    for(unsigned int i = 0; i < global.GetMethodsCount(); i ++) {
        MethodDescription m = global.GetMethod(i);
        *o << "<function name=\"" << m.GetName() << "\">" << endl;
        *o << "<return type=\"" << m.GetType() << "\"/>" << endl;
        *o << "<parameters>" << endl;
        for(unsigned int i = 0; i < m.GetParamsCount(); i ++) {
            MethodDescriptionParam p = m.GetParam(i);
            *o << "<parameter name=\"" << p.name <<
                "\" type=\"" << p.type << "\"/>" << endl;
        }
        *o << "</parameters>" << endl;
        *o << "</function>" << endl;
    }



}
