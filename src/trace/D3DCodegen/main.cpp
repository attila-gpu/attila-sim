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

#include "stdafx.h"
//#include "Vld/vld.h"  // VLD (virtual leak detection) library not included in the repository
#include "DXCodeGenException.hpp"
#include "Config/ConfigManager.hpp"
#include "Config/ParserConfiguration.hpp"
#include "Config/GeneratorConfiguration.hpp"
#include "Items/CppMacro.hpp"
#include "Items/ClassDescription.hpp"
#include "Items/MethodDescription.hpp"
#include "Items/StructDescription.hpp"
#include "Items/EnumDescription.hpp"
#include "Parser/DXHParser.hpp"
#include "Generator/DXHGenerator.hpp"
#include "Generator/IGenerator.hpp"
#include "Generator/D3D9PixRunPlayerGenerator.hpp"
#include "Generator/D3D9XMLGenerator.hpp"

using namespace std;
using namespace dxcodegen;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;
using namespace dxcodegen::Generator;

bool isPointer(StructDescription s) {
    string n = s.GetName();
    return n.find("*") != n.npos;
}

int main()
{
  try
  {
    // Get configuration
    ConfigManager config("dxcodegen.config");

    // Parse d3d9 original headers

    ParserConfiguration configParser;
    config.GetParserConfiguration(configParser);
    DXHParser parser(configParser);
    parser.ParseFiles();

    GeneratorConfiguration configGenerator;
    config.GetGeneratorConfiguration(configGenerator);

    // Generate code for D3D9PixRunPlayer

    D3D9PixRunPlayerGenerator d3dpgenerator(configGenerator);
    d3dpgenerator.setClasses(parser.GetClasses());
    d3dpgenerator.GenerateCode();


    // Uncomment to generate a XML representation of D3D9API
    
    /** @fix DXHParser incorrectly identify some
     *  pointer typedefs as structs. This example
     *  generates a struct named D3DDEVINFO_VCACHE,
     *  but also another named *LPD3DDEVINFO_VCACHE,
     *  and this is an error.
     * 
     * typedef struct _D3DDEVINFO_VCACHE {
     *     ...
     *     ...
     * } D3DDEVINFO_VCACHE, *LPD3DDEVINFO_VCACHE;
     **/
    
    // Remove all structs with a * in the name.
    // "remove_if" is not straightforward, search STL
    // docs for its usage.
    
    //vector<StructDescription> &structs = parser.GetStructs();
    //vector<StructDescription>::iterator itNewEnd;
    //itNewEnd = remove_if(structs.begin(), structs.end(), isPointer);
    //structs.erase(itNewEnd, structs.end());

    //D3D9XMLGenerator d3dxmlgenerator;
    //d3dxmlgenerator.setClasses(parser.GetClasses());
    //d3dxmlgenerator.setEnums(parser.GetEnums());
    //d3dxmlgenerator.setStructs(parser.GetStructs());

    //d3dxmlgenerator.GenerateCode();

    }
    catch (DXCodeGenException& e)
    {
        cout << "EXCEPTION INTERNAL: " << e.what() << endl;
    }
    catch (exception& e)
    {
    cout << "EXCEPTION EXTERNAL: " << e.what() << endl;
    }

    return 0;
}
