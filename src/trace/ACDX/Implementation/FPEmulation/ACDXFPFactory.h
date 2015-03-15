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

#ifndef ACDX_FPFACTORY_H
    #define ACDX_FPFACTORY_H

#include "ACDXFPState.h"
#include "ACDXTLShader.h"
#include "ACDXCodeSnip.h"
#include "ACDXTransformations.h"
#include <string>

namespace acdlib
{

class ACDXFPFactory 
{
private:

    static ACDXTLShader* tlshader;
    
    static ACDXFPState* fps;
    static ACDXCodeSnip code;
    
    static void constructInitialization();
    
    static void computeColorSum();
    
    static void computeFogApplication();
    static void computeFogFactor();
    static void computeFogCombination();

    static void computeTextureApplication(ACDXFPState::TextureUnit textureUnit, 
                                          std::string inputFragmentColorTemp, 
                                          std::string outputFragmentColorTemp,
                                          std::string primaryColorRegister);
    
    static void computeTextureApplicationTemporaries();
    
    static void constructTexturing();
    
    static void constructFragmentProcessing();
    
    static void constructPostProcessingTests();
    static void constructAlphaTest();
    
    static void computeDepthBypass();

    ACDXFPFactory();
    ACDXFPFactory(const ACDXFPFactory&);
    ACDXFPFactory& operator=(const ACDXFPFactory&);
    
public:
    
    static ACDXTLShader* constructFragmentProgram(ACDXFPState& fps);
    
    /*
     * Constructs a small ARB fragment program containing isolated alpha test implementation.
     * The code is a well-formed fragment program that can be compiled by itself and uses
     * a first temporary that will contain the final color and second temporary test register 
     * for kill condition.
     */
    static ACDXTLShader* constructSeparatedAlphaTest(ACDXFPState& fps);

};

} // namespace acdlib

#endif // ACDX_FPFACTORY_H
