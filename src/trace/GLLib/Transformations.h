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

#ifndef TRANSFORMATIONS_H
    #define TRANSFORMATIONS_H

#include "CodeSnip.h"
#include <sstream>
#include <iomanip>
#include <vector>

namespace libgl
{

/**
 * Generates required AMS code for multiply the current vertex by 
 * current OpenGL modelview matrix
 */
class ModelviewTransformation : public CodeSnip
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
class ProjectionTransformation : public CodeSnip
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
class MVPTransformation : public CodeSnip
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
class NormalTransformation : public CodeSnip
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
 
class TextureMatrixTransformation : public CodeSnip
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
class NHEyePositionComputing : public CodeSnip
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
class VertexEyeVectorComputing : public CodeSnip
{
public:

    VertexEyeVectorComputing(bool normalize=true);
};

/**
 * Generates required AMS code to calculate the distance attenuation for
 * the light source
 */
class AttenuationComputing : public CodeSnip
{
public:

    AttenuationComputing(int light);
};

class SpotLightComputation : public CodeSnip
{
public:

    SpotLightComputation(int light);
};


class DifusseAndSpecularComputing : public CodeSnip
{
public:

    DifusseAndSpecularComputing(int light, bool localLight, bool computeFront, bool computeBack, bool localViewer);

};

class PrimaryColorComputing : public CodeSnip
{
public:

    PrimaryColorComputing(int light, bool computeFront, bool computeBack, bool separateSpecular);
};


class SecondaryColorComputing : public CodeSnip
{
public:

    SecondaryColorComputing(int light, bool computeFront, bool computeBack);
    
};

class SceneColorComputing : public CodeSnip
{
public:

    SceneColorComputing(bool computeFront, bool computeBack, bool separateSpecular, bool colorMaterialEmissionFront = false, bool colorMaterialEmissionBack = false);
};

/**
 * Generates required ASM code for light[i] initialization
 */
class LightInit : public CodeSnip
{
public:
                                          /* position[3] != 0 */  /* cutoff[0] != 180.0 */
    LightInit(int i, bool infiniteViewer, bool localLightSource, bool isSpotLight);
};

/**
 * Generates required ASM code to bind the basic inputs as normals, colors
 * and lightmodel parameters
 */
class BasicInputInit : public CodeSnip
{
public:

    BasicInputInit(bool isLighting, bool anyLightEnabled, bool computeNormals, bool separateSpecular, bool computeFrontColor, bool computeBackColor, bool isRescaling,  bool colorMaterialEmissionFront = false, bool colorMaterialEmissionBack = false);
};

/**
 * Generates required ASM code to bind the matrices used in model
 * transformations
 */
class MatricesInit : public CodeSnip
{
public:

    MatricesInit(bool separateModelviewProjection, bool computeNormals);
};

/**
 * Generates required ASM code for temporary variables used in code
 */
class TemporaryInit : public CodeSnip
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
class OutputInit : public CodeSnip
{
public:

    OutputInit(bool separateSpecular, bool twoSidedLighting);
};

class TextureBypass : public CodeSnip
{
public:

    TextureBypass(unsigned int texUnit, std::string oCord = "oCord");
};

class NormalizeVector: public CodeSnip
{
public:
    NormalizeVector(std::string vector);
};

class HalfVectorComputing: public CodeSnip
{
public:
    
    HalfVectorComputing(int light, bool localLight, bool infiniteViewer);
};

class TextureSampling: public CodeSnip
{
public:

    TextureSampling(unsigned int texUnit, std::string texTarget);
};

class TextureFunctionComputing: public CodeSnip
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

class ObjectLinearGeneration: public CodeSnip
{
public:
    
    ObjectLinearGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string iPos = "iPos");

};

class EyeLinearGeneration: public CodeSnip
{
public:

    EyeLinearGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string eyePos = "eyePos");

};

class SphereMapGeneration: public CodeSnip
{
public:
    SphereMapGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string normalEye = "normalEye");

};

class ReflectionMapGeneration: public CodeSnip
{
public:
    ReflectionMapGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string normalEye = "normalEye", std::string reflectedVector = "reflectedVector");

};

class NormalMapGeneration: public CodeSnip
{
public:
    NormalMapGeneration(unsigned int maxTexUnit, std::vector<std::string>& coordinates, std::string oCord = "oCord", std::string normalEye = "normalEye");

};

class ReflectedVectorComputing: public CodeSnip
{
public:
    ReflectedVectorComputing(std::string normalEye = "normalEye", std::string eyePos = "eyePos");
};

class LinearFogFactorComputing: public CodeSnip
{
public:
    LinearFogFactorComputing(std::string fogParam = "fogParam", std::string fogFactor = "fogFactor");
};

class ExponentialFogFactorComputing: public CodeSnip
{
public:
    ExponentialFogFactorComputing(std::string fogParam = "fogParam", std::string fogFactor = "fogFactor");
};

class SecondOrderExponentialFogFactorComputing: public CodeSnip
{
public:
    SecondOrderExponentialFogFactorComputing(std::string fogParam = "fogParam", std::string fogFactor = "fogFactor");
};


} // namespace libgl

#endif
