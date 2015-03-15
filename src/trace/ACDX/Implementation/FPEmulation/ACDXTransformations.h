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

#ifndef ACDX_TRANSFORMATIONS_H
    #define ACDX_TRANSFORMATIONS_H

#include "ACDXCodeSnip.h"
#include <sstream>
#include <iomanip>
#include <vector>

namespace acdlib
{

/**
 * Generates required AMS code for multiply the current vertex by 
 * current OpenGL modelview matrix
 */
class ModelviewTransformation : public ACDXCodeSnip
{
public:
    /**
     * @param iPos vector containing vertex position in model coordinates
     * @param eyePos vector where to store eye-coordinates result
     */
    ModelviewTransformation(std::string iPos = "iPos", std::string eyePos = "eyePos");
};


/**
 * Generates required AMS code for multiply the current vertex in eyesCoords by 
 * current OpenGL projection matrix
 */
class ProjectionTransformation : public ACDXCodeSnip
{
public:
    /**
     * @param eyesPos vector containing vertex position in eyes-coordinates
     * @param oPos vector where to store result multiplication
     */
    ProjectionTransformation(std::string eyesPos = "eyePos", std::string oPos = "oPos");
};

/**
 * Generates required AMS code for multiply the current vertex by the
 * modelview projection matrix.
 */
class MVPTransformation : public ACDXCodeSnip
{
public:
    /**
     * @param iPos vector containing vertex model position
     * @param oPos vector where to store result multiplication
     */
    MVPTransformation(std::string iPos = "iPos", std::string oPos = "oPos");
};


/**
 * Generates required AMS code for multiply the current normal by the
 * current inversed transposed OpenGL modelview matrix
 */
class NormalTransformation : public ACDXCodeSnip
{
public:
    /**
     * @param normalize tells if normalization of normals is required.
     * @param isRescaling tells if rescaling to normals is applied.
     */     
    NormalTransformation(bool normalize, bool isRescaling);
};

/**
 * Generates required ASM code for multiply the specified texture
 * coordinate register by the related OpenGL texture matrix
 */
 
class TextureMatrixTransformation : public ACDXCodeSnip
{
public:
    /**
     * @param texUnit is the related texture unit
     * @param iCoord is the in coordinate register
     */
    TextureMatrixTransformation(unsigned int texUnit, std::string iCoord = "oCord");
};

/**
 * Generates required instructions to compute the non-homogeneous vertex
 * eye position. This is required to compute the Vertex to Eye vector (VP_e)
 * and the Vertex to Light vector (VP_pli).
 */
class NHEyePositionComputing : public ACDXCodeSnip
{
public:
    /**
     * @param eyePos temporary vector containing vertex eye position
     * @param eyePosNH output temporary where compute vertex eye NH position
     */
    NHEyePositionComputing(std::string eyePos = "eyePos", std::string eyePosNH = "eyePosNH");
};

/**
 * Generates required AMS code to calculate the normalized vector 
 * from vertex to eye.
 */
class VertexEyeVectorComputing : public ACDXCodeSnip
{
public:

    VertexEyeVectorComputing(bool normalize=true);
};

/**
 * Generates required AMS code to calculate the distance attenuation for
 * the light source
 */
class AttenuationComputing : public ACDXCodeSnip
{
public:

    AttenuationComputing(int light);
};

class SpotLightComputation : public ACDXCodeSnip
{
public:

    SpotLightComputation(int light);
};


class DifusseAndSpecularComputing : public ACDXCodeSnip
{
public:

    DifusseAndSpecularComputing(int light, bool localLight, bool computeFront, bool computeBack, bool localViewer);

};

class PrimaryColorComputing : public ACDXCodeSnip
{
public:

    PrimaryColorComputing(int light, bool computeFront, bool computeBack, bool separateSpecular);
};


class SecondaryColorComputing : public ACDXCodeSnip
{
public:

    SecondaryColorComputing(int light, bool computeFront, bool computeBack);
    
};

class SceneColorComputing : public ACDXCodeSnip
{
public:

    SceneColorComputing(bool computeFront, bool computeBack, bool separateSpecular, bool colorMaterialEmissionFront = false, bool colorMaterialEmissionBack = false);
};

/**
 * Generates required ASM code for light[i] initialization
 */
class LightInit : public ACDXCodeSnip
{
public:
                                          /* position[3] != 0 */  /* cutoff[0] != 180.0 */
    LightInit(int i, bool infiniteViewer, bool localLightSource, bool isSpotLight);
};

/**
 * Generates required ASM code to bind the basic inputs as normals, colors
 * and lightmodel parameters
 */
class BasicInputInit : public ACDXCodeSnip
{
public:

    BasicInputInit(bool isLighting, bool anyLightEnabled, bool computeNormals, bool separateSpecular, bool computeFrontColor, bool computeBackColor, bool isRescaling,  bool colorMaterialEmissionFront = false, bool colorMaterialEmissionBack = false);
};

/**
 * Generates required ASM code to bind the matrices used in model
 * transformations
 */
class MatricesInit : public ACDXCodeSnip
{
public:

    MatricesInit(bool separateModelviewProjection, bool computeNormals);
};

/**
 * Generates required ASM code for temporary variables used in code
 */
class TemporaryInit : public ACDXCodeSnip
{
public:

    TemporaryInit(bool isLighting, bool anyLightEnabled, bool separateSpecular, 
                  bool computeFrontColor, bool computeBackColor, bool anyLocalLight, bool infiniteViewer,
                  bool fogEnabled, bool separateModelviewProjection, bool computeNormals,
                  std::vector<bool>& texUnitEnabled, std::vector<bool>& texCoordMultiplicationRequired);
};

/**
 * Generates required ASM code to bind the basic outputs
 */
class OutputInit : public ACDXCodeSnip
{
public:

    OutputInit(bool separateSpecular, bool twoSidedLighting);
};

class TextureBypass : public ACDXCodeSnip
{
public:

    TextureBypass(unsigned int texUnit, std::string oCord = "oCord");
};

class NormalizeVector: public ACDXCodeSnip
{
public:
    NormalizeVector(std::string vector);
};

class HalfVectorComputing: public ACDXCodeSnip
{
public:
    
    HalfVectorComputing(int light, bool localLight, bool infiniteViewer);
};

class TextureSampling: public ACDXCodeSnip
{
public:

    TextureSampling(unsigned int texUnit, std::string texTarget);
};

class TextureFunctionComputing: public ACDXCodeSnip
{
public:
    
    enum CombineFunction 
    { 
        REPLACE, 
        MODULATE, 
        ADD, 
        ADD_SIGNED, 
        INTERPOLATE,
        SUBTRACT,
        DOT3_RGB, /* Only applicable to Combine alpha function */
        DOT3_RGBA,/* Only applicable to Combine alpha function */
        
        /* For "ATI_texture_env_combine3" extension support */
        MODULATE_ADD,
        MODULATE_SIGNED_ADD,
        MODULATE_SUBTRACT,
        
        /* For "NV_texture_env_combine4" extension support */
        ADD_4,
        ADD_4_SIGNED,
    };

    TextureFunctionComputing(unsigned int texUnit,
                             CombineFunction RGBOp, CombineFunction ALPHAOp, 
                             std::string RGBDestRegister, std::string ALPHADestRegister,
                             std::string RGBOperand0Register, std::string ALPHAOperand0Register,
                             std::string RGBOperand1Register, std::string ALPHAOperand1Register,
                             std::string RGBOperand2Register, std::string ALPHAOperand2Register,
                             std::string RGBOperand3Register, std::string ALPHAOperand3Register,
                             std::string RGBOperand0RegisterSWZ, std::string ALPHAOperand0RegisterSWZ,
                             std::string RGBOperand1RegisterSWZ, std::string ALPHAOperand1RegisterSWZ,
                             std::string RGBOperand2RegisterSWZ, std::string ALPHAOperand2RegisterSWZ,
                             std::string RGBOperand3RegisterSWZ, std::string ALPHAOperand3RegisterSWZ,
                             unsigned int RGBScaleFactor, unsigned int ALPHAScaleFactor,
                             bool clamp);
};

class ObjectLinearGeneration: public ACDXCodeSnip
{
public:
    
    ObjectLinearGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string iPos = "iPos");

};

class EyeLinearGeneration: public ACDXCodeSnip
{
public:

    EyeLinearGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string eyePos = "eyePos");

};

class SphereMapGeneration: public ACDXCodeSnip
{
public:
    SphereMapGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string normalEye = "normalEye");

};

class ReflectionMapGeneration: public ACDXCodeSnip
{
public:
    ReflectionMapGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string normalEye = "normalEye", std::string reflectedVector = "reflectedVector");

};

class NormalMapGeneration: public ACDXCodeSnip
{
public:
    NormalMapGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string normalEye = "normalEye");

};

class ReflectedVectorComputing: public ACDXCodeSnip
{
public:
    ReflectedVectorComputing(std::string normalEye = "normalEye", std::string eyePos = "eyePos");
};

class LinearFogFactorComputing: public ACDXCodeSnip
{
public:
    LinearFogFactorComputing(std::string fogParam = "fogParam", std::string fogFactor = "fogFactor");
};

class ExponentialFogFactorComputing: public ACDXCodeSnip
{
public:
    ExponentialFogFactorComputing(std::string fogParam = "fogParam", std::string fogFactor = "fogFactor");
};

class SecondOrderExponentialFogFactorComputing: public ACDXCodeSnip
{
public:
    SecondOrderExponentialFogFactorComputing(std::string fogParam = "fogParam", std::string fogFactor = "fogFactor");
};


} // namespace acdlib

#endif // ACDX_TRANSFORMATIONS_H
