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

#ifndef TLFACTORY_H
    #define TLFACTORY_H

#include "TLState.h"
#include "TLShader.h"
#include "CodeSnip.h"
#include "Transformations.h"
#include <string>

namespace libgl
{

class TLFactory 
{
private:

    static TLShader* tlshader;

    typedef TLState::Light Light;
    
    static TLState* tls;
    static CodeSnip code;
    static ModelviewTransformation modelview;
    static ProjectionTransformation projection;
    static NormalTransformation normal;
    static MVPTransformation mvp;

    /* Flags computed from TLState Info */
    static bool separateModelviewProjection;
    static bool computeNormals;
    static bool computeNHEyePosition;
    static bool anyNormalDependantCoordinateGenerationMode;
    static bool anyEyeDependantCoordinateGenerationMode;
    static bool anyReflectedVectorDependantCoordinateGenerationMode;
    static TLState::TextureUnit* texUnit;
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

    TLFactory();
    TLFactory(const TLFactory&);
    TLFactory& operator=(const TLFactory&);
    
public:
    
    static TLShader* constructVertexProgram(TLState& tls);
    
};

} // namespace libgl

#endif /* TLFACTORY_H */
