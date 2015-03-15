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

#ifndef FPFACTORY_H
    #define FPFACTORY_H

#include "FPState.h"
#include "TLShader.h"
#include "CodeSnip.h"
#include "Transformations.h"
#include <string>

namespace libgl
{

class FPFactory 
{
private:

    static TLShader* tlshader;
    
    static FPState* fps;
    static CodeSnip code;
    
    static void constructInitialization();
    
    static void computeColorSum();
    
    static void computeFogApplication();
    static void computeFogFactor();
    static void computeFogCombination();

    static void computeTextureApplication(FPState::TextureUnit textureUnit, 
                                          std::string inputFragmentColorTemp, 
                                          std::string outputFragmentColorTemp,
                                          std::string primaryColorRegister);
    
    static void computeTextureApplicationTemporaries();
    
    static void constructTexturing();
    
    static void constructFragmentProcessing();
    
    static void constructPostProcessingTests();
    static void constructAlphaTest();
    
    static void computeDepthBypass();

    FPFactory();
    FPFactory(const FPFactory&);
    FPFactory& operator=(const FPFactory&);
    
public:
    
    static TLShader* constructFragmentProgram(FPState& fps);
    
    /*
     * Constructs a small ARB fragment program containing isolated alpha test implementation.
     * The code is a well-formed fragment program that can be compiled by itself and uses
     * a first temporary that will contain the final color and second temporary test register 
     * for kill condition.
     */
    static TLShader* constructSeparatedAlphaTest(FPState& fps);

};

} // namespace libgl

#endif /* FPFACTORY_H */
