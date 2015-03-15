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

#ifndef ACDX_TLFACTORY_H
    #define ACDX_TLFACTORY_H

#include "ACDXTLState.h"
#include "ACDXTLShader.h"
#include "ACDXCodeSnip.h"
#include "ACDXTransformations.h"
#include <string>

namespace acdlib
{

class ACDXTLFactory 
{
private:

    static ACDXTLShader* tlshader;

    typedef ACDXTLState::Light Light;
    
    static ACDXTLState* tls;
    static ACDXCodeSnip code;
    static ModelviewTransformation modelview;
    static ProjectionTransformation projection;
    static NormalTransformation normal;
    static MVPTransformation mvp;

    /* Flags computed from ACDXTLState Info */
    static bool separateModelviewProjection;
    static bool computeNormals;
    static bool computeNHEyePosition;
    static bool anyNormalDependantCoordinateGenerationMode;
    static bool anyEyeDependantCoordinateGenerationMode;
    static bool anyReflectedVectorDependantCoordinateGenerationMode;
    static ACDXTLState::TextureUnit* texUnit;
    static bool frontFaceCulling;
    static bool backFaceCulling;
    static bool computeFrontColor;
    static bool computeBackColor;
    static bool bothFacesCulled;
    static bool twoSidedLighting;
    static bool separateSpecular;
    static bool colorMaterialForEmissionFront;
    static bool colorMaterialForEmissionBack;
    
    
    static void constructTLFactoryFlags();
    static void destroyTLFactoryFlags();



    static void constructInitialization();
    
    static void constructTransforms();

    static void constructModelviewTransform();
    static void constructMVPTransform();
    static void constructNormalTransform();
    static void constructNHEyePositionTransform();
    static void constructProjectionTransform();
    static void constructTextureCoordinateTransform();
    
    static void computeVertexEyeVector();
    static void computeAttenuation(Light i);
    static void computeSpotlight(Light i);
    static void computeHalfVector(Light i, bool locallight, bool localviewer);
    static void computeDiffuseAndSpecularComponents(Light i, bool locallight, bool computeFront, bool computeBack, bool localviewer);
    static void computePrimaryColor(Light i, bool computeFront, bool computeBack, bool separate_specular);
    static void computeSecondaryColor(Light i, bool computeFront, bool computeBack);
    static void computeSceneColor(bool computeFront, bool computeBack, bool separateSpecular, bool colorMaterialEmissionFront, bool colorMaterialEmissionBack);
    
      
    static void constructNoLighting();
    
    static void constructLighting();
    
    static void constructTextureCoordinate();
    static void constructTextureCoordinateOutput();

    static void constructFogCoordinateOutput();

    ACDXTLFactory();
    ACDXTLFactory(const ACDXTLFactory&);
    ACDXTLFactory& operator=(const ACDXTLFactory&);
    
public:
    
    static ACDXTLShader* constructVertexProgram(ACDXTLState& tls);
    
};

} // namespace acdlib

#endif // ACDX_TLFACTORY_H
