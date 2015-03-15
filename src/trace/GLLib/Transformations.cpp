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

#include "Transformations.h"
#include <iostream>
#include "support.h"
#include <cmath>

using namespace std;
using namespace libgl;

ModelviewTransformation::ModelviewTransformation(string iPos, string eyePos)
{
    string text =
        "# Transform the vertex to eye-coordinates\n"
        "DP4 " + eyePos + ".x, mvm[0], " + iPos + ";\n"
        "DP4 " + eyePos + ".y, mvm[1], " + iPos + ";\n"
        "DP4 " + eyePos + ".z, mvm[2], " + iPos + ";\n"
        "DP4 " + eyePos + ".w, mvm[3], " + iPos + ";\n";

    setCode(text);
}


ProjectionTransformation::ProjectionTransformation(string eyePos, string oPos)
{
    string text =
        "# Transform the vertex to clip-coordinates\n"
        "DP4 " + oPos + ".x, proj[0], " + eyePos + ";\n"
        "DP4 " + oPos + ".y, proj[1], " + eyePos + ";\n"
        "DP4 " + oPos + ".z, proj[2], " + eyePos + ";\n"
        "DP4 " + oPos + ".w, proj[3], " + eyePos + ";\n";

    setCode(text);
}

MVPTransformation::MVPTransformation(string iPos, string oPos)
{
    string text =
        "# Multiply the vertex to mvp\n"
        "DP4 " + oPos + ".x, mvp[0], " + iPos + ";\n"
        "DP4 " + oPos + ".y, mvp[1], " + iPos + ";\n"
        "DP4 " + oPos + ".z, mvp[2], " + iPos + ";\n"
        "DP4 " + oPos + ".w, mvp[3], " + iPos + ";\n";

    setCode(text);
}

NHEyePositionComputing::NHEyePositionComputing(string eyePos, string eyePosNH)
{
    string text =  "# Non-homogeneous Eye position\n"
                   "RCP temp1.w, " + eyePos + ".w;\n"
                   "MUL " + eyePosNH + ", " + eyePos + ", temp1.w;\n";
    setCode(text);
}

NormalTransformation::NormalTransformation(bool normalize, bool isRescaling)
{
    string text =
        "# ------------ Normal Calculations -----------\n"
        "# Transform the normal to eye coordinates. \n"
        "DP3 normalEye.x, mvinv[0], iNormal;\n"
        "DP3 normalEye.y, mvinv[1], iNormal;\n"
        "DP3 normalEye.z, mvinv[2], iNormal;\n";

    if (normalize)
    {
        text +=
            "# Normalize Eye Normal\n"
            "DP3 normalEye.w, normalEye, normalEye;\n"
            "RSQ normalEye.w, normalEye.w;\n"
            "MUL normalEye, normalEye, normalEye.w;\n";
    }
    if (isRescaling)
    {
        text +=
            "#Rescaling of normalvector\n"
            "MUL normalEye, normalEye, rescaleScale;\n";
    }

    text += "# ------------ End Normal Calculations -----------\n";

    setCode(text);
}

TextureMatrixTransformation::TextureMatrixTransformation(unsigned int texUnit, string iCoord)
{
    stringstream ss;
    ss << texUnit;
    string unit = ss.str();

    string text = "# --------- Texture coordinate transformation by Texture Matrix " + unit + "  --------\n"
                  "DP4 result.texcoord[" + unit + "].x, state.matrix.texture[" + unit + "].row[0], " + iCoord + unit + ";\n"
                  "DP4 result.texcoord[" + unit + "].y, state.matrix.texture[" + unit + "].row[1], " + iCoord + unit + ";\n"
                  "DP4 result.texcoord[" + unit + "].z, state.matrix.texture[" + unit + "].row[2], " + iCoord + unit + ";\n"
                  "DP4 result.texcoord[" + unit + "].w, state.matrix.texture[" + unit + "].row[3], " + iCoord + unit + ";\n";
    setCode(text);
}

VertexEyeVectorComputing::VertexEyeVectorComputing(bool normalize)
{
    string text =
        "# ----------------- Vertex to Eye vector computing -----------------\n"
        "# Compute the vector pointing from the vertex position to the eye position\n"
        "# d = |eyePositionConst - eyePosNH|\n"
        "SUB temp1, eyePositionConst, eyePosNH;\n"
        "# temp1.w = d^2\n"
        "DP3 temp1.w, temp1, temp1;\n"
        "# reflEyeLightVector.w = 1/d \n"
        "RSQ temp1.w, temp1.w;\n"
        "# Normalized direction (VP_e unit vector) \n"
        "MUL vertexEyeVector, temp1, temp1.w;\n"
        "# ---------------- End Vertex to Eye vector computing ---------------\n";

    setCode(text);
}


AttenuationComputing::AttenuationComputing(int light)
{

    stringstream ss;
    ss << light;
    string i = ss.str();;

    string text =

        "# ----------------- Vertex to Light" + i + " vector computing -----------------\n"
        "# Compute the vector pointing from the vertex position to the light position\n"
        "# temp1 = lightposition - eyeposition (VP_pli)\n"
        "SUB temp1, lightPos" + i + ", eyePosNH;\n"

        "# Compute light direction and distance vector\n"
        "# d = |lightposition - eyeposition|\n"
        "DP3 temp1.w, temp1, temp1; # temp1.w = d^2\n"
        "RSQ distVector.w, temp1.w; # distVector.w = 1/d\n"
        "# Normalized direction (VP_pli unit vector)\n"
        "MUL reflEyeLightVector, temp1, distVector.w;\n"
        "DST distVector, temp1.w, distVector.w; # distVector = (1, d, d^2, 1/d)\n"

        "# Compute Distance Attenuation\n"
        "DP3 distVector.w, distVector, lightAttCoeff" + i + ";\n"
        "RCP distVector.w, distVector.w;\n"
        "# ---------------- End Vertex to Light" + i + " vector computing ---------------\n";

    setCode(text);

}

SpotLightComputation::SpotLightComputation(int light)
{
    stringstream ss;
    ss << light;
    string i = ss.str();

    string text =
        "# ----------------- Spotlight cone attenuation computing -----------------\n"
        "# Compute spotlight cone attenuation\n"
        "DP3 temp1.y, reflEyeLightVector, -lightSpotDir" +i+ ";\n"
        "ADD temp1.x, temp1.y, -lightSpotDir" +i+ ".w;\n"
        "MOV temp1.w, lightAttCoeff" +i+ ".w;\n"
        "# Now temp1 = (P_pliV * S_dli - cos(c_rli), P_pliV * s_dli, -, s_lri)\n"

        "# temp1.z = 0.0, if p_pliV * S_dli - cos(c_rli) < 0\n"
        "LIT temp1, temp1;\n"
        "# Light attenuation and spotlight attenuation is now multiplied together\n"
        "MUL distVector.w, distVector.w, temp1.z;\n"
        "# -------------- End Spotlight cone attenuation computing ----------------\n";

    setCode(text);
}


DifusseAndSpecularComputing::DifusseAndSpecularComputing(int light, bool localLight, bool computeFront, bool computeBack, bool localViewer)
{
    string text;

    stringstream ss;
    ss << light;
    string i = ss.str();

    if ( !localViewer ) // infinite viewer
    {
        if ( computeFront )
        {
            text +=
                "# Compute the light coefficients for diffuse and\n"
                "# specular light for Front material\n";

            // calculate the diffuse coefficient (front side)
            // calculate the specular coefficient (front side, local viewer)
            // multiply each coefficient (front side) with the specular/diffuse product
            if ( localLight )
            {
                text += "DP3 temp1.x, normalEye, reflEyeLightVector;\n"
                        "DP3 temp1.y, normalEye, halfVector; \n";
            }
            else // directional light
            {
                text += "DP3 temp1.x, normalEye, lightPosNorm" + i + ";\n"
                        "DP3 temp1.y, normalEye, halfVector; \n";
            }

            text += "MOV temp1.w, specExpFront.x; # now: temp1 = (n * VP_pli , n * h_i , - , s_rm)\n"
                    "LIT temp1, temp1;\n";

            if ( localLight )
            {
                text +=
                    "# Light attenuation and spotlight is now multiplied with\n"
                    "# the ambient, diffuse and specular coefficients \n"
                    "MUL temp1, temp1, distVector.w;\n";
            }
        }
        if (computeBack)
        {
            text +=
                "# Compute the light coefficients for diffuse and\n"
                "# specular light for Back material\n";

            if ( localLight ) // Both V and P are points
                text += "DP3 temp1.x, -normalEye, reflEyeLightVector;\n";
            else // V is a point and P is a direction
                text += "DP3 temp1.x, -normalEye, lightPosNorm" + i + ";\n";

            text +=
                "DP3 temp1.y, -normalEye, halfVector; \n"
                "MOV temp1.w, specExpBack.x; # now: temp1 = (-n * VP_pli , -n * h_i , - , s_rm)\n"
                "LIT temp1, temp1;\n";

            if ( localLight )
            {
                text +=
                    "# Light attenuation and spotlight is now multiplied with\n"
                    "# the ambient, diffuse and specular coefficients \n"
                    "MUL temp1, temp1, distVector.w;\n";
            }
        }
    }
    else
    { // local viewer
        if ( computeFront )
        {
            text +=
                "# Compute the light coefficients for diffuse and\n"
                "# specular light for Front material\n";

            if ( localLight ) // Both V and P are points
                text += "DP3 temp1.x, normalEye, reflEyeLightVector;\n";
            else // V is a point and P is a direction
                text += "DP3 temp1.x, normalEye, lightPosNorm" + i+ ";\n";

            text +=
                "DP3 temp1.y, normalEye, halfVector; \n"
                "MOV temp1.w, specExpFront.x; # now: temp1 = (n * VP_pli , n * h_i , - , s_rm)\n"
                "LIT temp1, temp1;\n";

            if ( localLight ) //multiply each coefficient (front side) with the specular/diffuse product
            {
                text +=
                    "# Light attenuation and spotlight is now multiplied with\n"
                    "# the ambient, diffuse and specular coefficients \n"
                    "MUL temp1, temp1, distVector.w;\n";
            }
        }
        if ( computeBack )
        {
            text +=
                "# Compute the light coefficients for diffuse and\n"
                "# specular light for Back material\n";

            if ( localLight ) // Both V and P are points
                text += "DP3 temp1.x, -normalEye, reflEyeLightVector;\n";
            else // V is a point and P is a direction
                text += "DP3 temp1.x, -normalEye, lightPosNorm" + i+ ";\n";

            text +=
                "DP3 temp1.y, -normalEye, halfVector; \n"
                "MOV temp1.w, specExpBack.x; # now: temp1 = (-n * VP_pli , -n * h_i , - , s_rm)\n"
                "LIT temp1, temp1; \n";

            if ( localLight ) // multiply each coefficient (back side) with the specular/diffuse product
            {
                text +=
                    "# Light attenuation and spotlight is now multiplied with\n"
                    "# the ambient, diffuse and specular coefficients\n"
                    "MUL temp1, temp1,distVector.w;\n";
            }
        }
    }

    setCode(text);
}

PrimaryColorComputing::PrimaryColorComputing(int light, bool computeFront, bool computeBack, bool separateSpecular)
{
    string text;

    stringstream ss;
    ss << light;
    string i = ss.str();

    if ( computeFront )
    {
        text +=
            "# Adding up all the color components for Front Material\n"
            "MAD primFront.xyz, temp1.x, state.lightprod[" + i + "].front.ambient, primFront;\n"
            "MAD primFront.xyz, temp1.y, state.lightprod[" + i + "].front.diffuse, primFront;\n";

        if ( !separateSpecular ) // The specular color is added up with the primary color
            text += "MAD primFront.xyz, temp1.z, state.lightprod[" + i + "].front.specular, primFront;\n";

    }
    if ( computeBack )
    {
        text +=
            "# Adding up all the color components for Back Material\n"
            "MAD primBack.xyz, temp1.x, state.lightprod[" + i + "].back.ambient, primBack;\n"
            "MAD primBack.xyz, temp1.y, state.lightprod[" + i + "].back.diffuse, primBack; \n";
        if ( !separateSpecular ) // The specular color is added up with the primary color
            text += "MAD primBack.xyz, temp1.z, state.lightprod[" + i + "].back.specular, primBack;\n";
    }

    setCode(text);
}


SecondaryColorComputing::SecondaryColorComputing(int light, bool computeFront, bool computeBack)
{
    stringstream ss;
    ss << light;
    string i = ss.str();
    string text;

    if ( computeFront )
        text += "MAD secFront.xyz, temp1.z, state.lightprod[" + i + "].front.specular, secFront; \n";
    if ( computeBack )
        text += "MAD secBack.xyz, temp1.z, state.lightprod[" + i + "].back.specular, secBack; \n";

    setCode(text);
}


SceneColorComputing::SceneColorComputing(bool computeFront, bool computeBack, bool separateSpecular, bool colorMaterialEmissionFront, bool colorMaterialEmissionBack)
{
    string text;

    if ( computeFront )
    {
        text =
            "# The final addition of scene color and copy to the\n"
            "# output for Front Material\n";

        if (colorMaterialEmissionFront)
            text += "# Color Material Front for GL_EMISSION enabled. Using vertex color as e_cm.\n"
                    "MAD sceneLightFront, state.lightmodel.ambient, state.material.ambient, vertex.color;\n";

        text += "ADD oColorPrimFront, primFront, sceneLightFront;\n";


        if ( separateSpecular ) // The secondary color is moved to the output register
            text += "MOV oColorSecFront, secFront;\n";
    }
    if ( computeBack )
    {
        text +=
            "# The final addition of scene color and copy to the\n"
            "# output for Back Material\n";

        if (colorMaterialEmissionBack)
            text += "# Color Material Back for GL_EMISSION enabled. Using vertex color as e_cm.\n"
                    "MAD sceneLightBack, state.lightmodel.ambient, state.material.back.ambient, vertex.color;\n";
        else
            text += "ADD oColorPrimBack, primBack, sceneLightBack;\n";

        if ( separateSpecular ) // The secondary color is moved to the output register
            text += "MOV oColorSecBack, secBack;\n";
    }

    setCode(text);
}

LightInit::LightInit(int light, bool infiniteViewer, bool localLightSource, bool isSpotLight)
{
    stringstream ss;
    ss << light;

    string i = ss.str();

    string text =
        "# Declare parameters for light " + i + "\n"
        "PARAM lightPos" + i + " = state.light[" + i + "].position;\n";

    if ( infiniteViewer )
        text += "PARAM halfDir" + i + " = state.light[" + i + "].half;\n";

    if ( !localLightSource ) // Infinite light source
        text += "PARAM lightPosNorm" + i + " = program.local[" + i + "];\n";

    else // Light source is local
    {
        text += "PARAM lightAttCoeff" + i + " = state.light[" + i + "].attenuation;\n";

        if ( isSpotLight ) // Light source is a spot light
        {
            ss << (light + 16);
            text += "PARAM lightSpotDir" + i + " = state.light[" + i + "].spot.direction;\n";

                    // "PARAM lightSpotCos" + i + " = state.light[" + i + "].spot.direction;\n"
                    // "PARAM lightSpotDir" + i + " = program.local[" + ss.str() + "];\n";

        }
    }

    setCode(text);
}

BasicInputInit::BasicInputInit(bool isLighting, bool anyLightEnabled, bool computeNormals, bool separateSpecular, bool computeFrontColor, bool computeBackColor, bool isRescaling, bool colorMaterialEmissionFront, bool colorMaterialEmissionBack)
{
    string text =   "# ------------ Basic vertex input binding ------------\n"
                    "ATTRIB iPos = vertex.position;\n";

    if (computeNormals)
    {
        text += "ATTRIB iNormal = vertex.normal;\n";
    }

    if (isLighting)
    {
        if (computeFrontColor)
        {
            if (colorMaterialEmissionFront)
                text += "TEMP sceneLightFront;\n";
            else
                text += "# front scene color = a_cs * a_cm + e_cm\n"
                        "PARAM sceneLightFront = state.lightmodel.scenecolor;\n";
        }

        if (computeBackColor)
        {
            if (colorMaterialEmissionBack)
                text += "TEMP sceneLightBack;\n";
            else
                text += "# back scene color = a_cs * a_cm + e_cm\n"
                        "PARAM sceneLightBack = state.lightmodel.back.scenecolor;\n";
        }

        if (anyLightEnabled)
        {
            if (isRescaling)
                text += "PARAM rescaleScale = program.local[40];\n";

            text += "# Position of eye\n"
                    "PARAM eyePositionConst = { 0.0, 0.0, 0.0, 1.0 };\n";
                    if (computeFrontColor)
                    {
                        text += "# Front Material specular exponent\n"
                                "PARAM specExpFront = state.material.front.shininess;\n";
                    }

                    if (computeBackColor)
                    {
                        text += "# Back Material specular exponent\n"
                                "PARAM specExpBack = state.material.back.shininess;\n";
                    }
        }
    }
    else // No lighting
    {
        text += "ATTRIB iColorPrim = vertex.color.primary; \n";
        if (separateSpecular)
            text += "ATTRIB iColorSec = vertex.color.secondary; \n";
    }

    text += "# ------------ End Basic vertex input binding ------------\n";

    setCode(text);
}

TextureBypass::TextureBypass(unsigned int tex, string oCord)
{
    stringstream ss;
    ss << tex;
    string t = ss.str();

    string text =   "# ------------- Bypass texture coordinates ------------\n"
                    "MOV " + oCord + t + ", vertex.texcoord["+t+"];\n";
    setCode(text);
}



MatricesInit::MatricesInit(bool separateModelviewProjection, bool computeNormals)
{
    string text = "# ------------ Matrices Initialization -----------\n";
    if (separateModelviewProjection)

        text +=
                "PARAM mvm[4] = { state.matrix.modelview };\n"
                "PARAM proj[4] = { state.matrix.projection };\n";

    else

        text += "PARAM mvp[4] = { state.matrix.mvp };\n";

    if (computeNormals)

        text += "PARAM mvinv[4] = { state.matrix.modelview.invtrans };\n";


    text += "# ------------ End Matrices Initialization -----------\n";

    setCode(text);
}


TemporaryInit::TemporaryInit(bool isLighting, bool anyLightEnabled,
                             bool separateSpecular, bool computeFrontColor, bool computeBackColor,
                             bool anyLocalLight, bool infiniteViewer,
                             bool fogEnabled,
                             bool separateModelviewProjection, bool computeNormals,
                             vector<bool>& texUnitEnabled,
                             vector<bool>& texCoordMultiplicationRequired)
{
    string text = "";

    bool anyTexCoordMultiplicationRequired = false;
    bool anyTextureUnitEnabled = false;

    int i;

    for (i=0; i< texUnitEnabled.size(); i++)
    {
        if (texUnitEnabled[i])
            anyTextureUnitEnabled = true;

        if (texCoordMultiplicationRequired[i])
            anyTexCoordMultiplicationRequired = true;
    }

    if (isLighting || computeNormals || separateModelviewProjection || anyTextureUnitEnabled)
    /* Any temporary needed */
    {
        text += "# ------------ Temporary List --------------\n";

        if (isLighting)
        {
            if (computeFrontColor)
            {
                text += "TEMP primFront;\n";
                if (separateSpecular)
                    text += "TEMP secFront;\n";
            }

            if (computeBackColor)
            {
                text += "TEMP primBack;\n";
                if (separateSpecular)
                    text += "TEMP secBack;\n";
            }

            if (anyLightEnabled)
            {
                text += "TEMP temp1;\n"; /* used in several computations about lighting */
                text += "TEMP halfVector;\n";
            }
        }


        if (computeNormals)
            text += "TEMP normalEye;\n";

        if (separateModelviewProjection)
            text += "TEMP eyePos;\n";

        if (isLighting && (anyLocalLight || !infiniteViewer))
            text += "TEMP eyePosNH;\n";

        if (isLighting && !infiniteViewer)
            text += "TEMP vertexEyeVector;\n";

        if (isLighting && anyLocalLight)
            text += "TEMP distVector"            /* Used in attenuation computation */
                     ", reflEyeLightVector;\n"; /* Used in phong and lambert shading */

        if (anyTextureUnitEnabled)
        {
            for (i=0; i < texUnitEnabled.size(); i++)
            {
                if (texUnitEnabled[i])
                {
                    stringstream ss;
                    ss << i;

                    if (texCoordMultiplicationRequired[i])
                        text += "TEMP oCord" + ss.str() + ";\n";
                    else
                        text += "OUTPUT oCord" + ss.str() + " = result.texcoord[" + ss.str() + "];\n";
                }
            }
        }


        text += "# ------------ End Temporary List ------------\n";

        /* Initialize required variables to 0(they are accumulated in code) */

        if (isLighting)
        {
            if (computeFrontColor)
            {
                text += "MOV primFront, {0,0,0,0};\n";

                if (separateSpecular)
                    text += "MOV secFront, {0,0,0,0};\n";
            }

            //if (twoSidedLighting)
            if (computeBackColor)
            {
                text += "MOV primBack, {0,0,0,0};\n";
                if (separateSpecular)
                    text += "MOV secBack, {0,0,0,0};\n";
            }
        }
    }
    setCode(text);
}

OutputInit::OutputInit(bool separateSpecular, bool twoSidedLighting)
{
    string text = "# ------------ Output init --------------\n";

    text += "OUTPUT oPos = result.position;\n";

    text += "OUTPUT oColorPrimFront = result.color.front.primary;\n";

    if (separateSpecular)
           text += "OUTPUT oColorSecFront = result.color.front.secondary; \n";

    if (twoSidedLighting)
    {
            text += "OUTPUT oColorPrimBack = result.color.back.primary; \n";

            if (separateSpecular)
                text += "OUTPUT oColorSecBack = result.color.back.secondary; \n";
    }

    text += "# ----------- End Output init -----------\n";

    setCode(text);
}

HalfVectorComputing::HalfVectorComputing(int light, bool localLight, bool infiniteViewer)
{
    stringstream ss;
    ss << light;

    string i = ss.str();

    string text = "# Compute the normalized half-angle vector \n";
    bool doNormalization = false;

    if (localLight && !infiniteViewer)  // A positional light with local viewer
    {
        text += "ADD halfVector, reflEyeLightVector, vertexEyeVector;\n";
        doNormalization = true;
    }
    else if (localLight && infiniteViewer) // A positional light with infinite viewer
    {
        text += "ADD halfVector, reflEyeLightVector, {0,0,1,0};\n";
        doNormalization = true;
    }
    else if (!localLight && !infiniteViewer) // A directional light with local viewer
    {
        text += "ADD halfVector, lightPosNorm" + i + ", vertexEyeVector;\n";
        doNormalization = true;
    }
    else
    {   // A directional light with infinite viewer
        text += "MOV halfVector, halfDir" + i + "; \n";
        doNormalization = false;
    }

    // half vector normalization (necessary in all cases except the last one)

    if (doNormalization)
    {
        text += "DP3 halfVector.w, halfVector, halfVector; # halfVector.w = halfVector^2\n"
                "RSQ halfVector.w, halfVector.w; # halfVector.w = 1/|halfVector\n"
                "MUL halfVector, halfVector, halfVector.w; # normalized halfangle\n";
    }

    setCode(text);
}

TextureSampling::TextureSampling(unsigned int texUnit, std::string texTarget)
{
    stringstream ss;
    ss << texUnit;

    string t = ss.str();

    string text = "# -- Sampling Texture Unit " + t + " with Texture Target " + texTarget + " --\n"
                  "TEX texLookupColor" + t + ", iCoord" + t + ", texture[" + t + "], " + texTarget + ";\n";
    setCode(text);
}

TextureFunctionComputing::TextureFunctionComputing(unsigned int texUnit,
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
                             bool clamp)
{
    string text = "";  /* Output transformation text */


    /* *************************** RGB Combination ************************** */

    string CombineOpcodeRGB;
    unsigned int numberOperandsRGB;

    /* Check number of operands needed to combine functions
     * and assign the main instruction opcode that implements
     * the combination
     */

    switch(RGBOp)
    {
        case REPLACE:               numberOperandsRGB = 1; CombineOpcodeRGB = "MOV"; break;
        case MODULATE:              numberOperandsRGB = 2; CombineOpcodeRGB = "MUL"; break;
        case ADD:                   numberOperandsRGB = 2; CombineOpcodeRGB = "ADD"; break;
        case ADD_SIGNED:            numberOperandsRGB = 2; CombineOpcodeRGB = "ADD"; break;
        case INTERPOLATE:           numberOperandsRGB = 3; CombineOpcodeRGB = "LRP"; break;
        case SUBTRACT:              numberOperandsRGB = 2; CombineOpcodeRGB = "SUB"; break;
        case DOT3_RGB:              numberOperandsRGB = 2; CombineOpcodeRGB = "DP3"; break;
        case DOT3_RGBA:             numberOperandsRGB = 2; CombineOpcodeRGB = "DP3"; break;

        /* For "ATI_texture_env_combine3" extension support */
        case MODULATE_ADD:          numberOperandsRGB = 3; CombineOpcodeRGB = "MAD"; break;
        case MODULATE_SIGNED_ADD:   numberOperandsRGB = 3; CombineOpcodeRGB = "MAD"; break;
        case MODULATE_SUBTRACT:     numberOperandsRGB = 3; CombineOpcodeRGB = "MAD"; break;

        /* For "NV_texture_env_combine4" extension support */
        case ADD_4:                 numberOperandsRGB = 4; CombineOpcodeRGB = "MAD"; break;
        case ADD_4_SIGNED:          numberOperandsRGB = 4; CombineOpcodeRGB = "MAD"; break;
        default:
            panic("TextureFunctionComputing","TextureFunctionComputing","Unexpected combine RGB texture function");
    }

#define IS_ONE_MINUS_OPERAND(s)    !s.find("!")
#define SKIP_ONE_MINUS_SYMBOL(s) s.erase(0,1)

    if (IS_ONE_MINUS_OPERAND(RGBOperand0RegisterSWZ))
    {
        text += "SUB operand0RGB, {1,1,1,1}, " + RGBOperand0Register + ";\n";
        SKIP_ONE_MINUS_SYMBOL(RGBOperand0RegisterSWZ);
        RGBOperand0Register = "operand0RGB";
    }

    if (numberOperandsRGB > 1 && IS_ONE_MINUS_OPERAND(RGBOperand1RegisterSWZ))
    {
        text += "SUB operand1RGB, {1,1,1,1}, " + RGBOperand1Register + ";\n";
        SKIP_ONE_MINUS_SYMBOL(RGBOperand1RegisterSWZ);
        RGBOperand1Register = "operand1RGB";
    }

    if (numberOperandsRGB > 2 && IS_ONE_MINUS_OPERAND(RGBOperand2RegisterSWZ))
    {
        text += "SUB operand2RGB, {1,1,1,1}, " + RGBOperand2Register + ";\n";
        SKIP_ONE_MINUS_SYMBOL(RGBOperand2RegisterSWZ);
        RGBOperand2Register = "operand2RGB";
    }

    if (numberOperandsRGB > 3 && IS_ONE_MINUS_OPERAND(RGBOperand3RegisterSWZ))
    {
        text += "SUB operand3RGB, {1,1,1,1}, " + RGBOperand3Register + ";\n";
        SKIP_ONE_MINUS_SYMBOL(RGBOperand3RegisterSWZ);
        RGBOperand3Register = "operand3RGB";
    }

    /* Check if RGB and ALPHA combination can be done in a single instruction */

    bool unifyRGBA = false;

    if (RGBOp == ALPHAOp &&
        RGBDestRegister == ALPHADestRegister &&
        RGBOperand0Register == ALPHAOperand0Register &&
        ((numberOperandsRGB <= 1) || RGBOperand1Register == ALPHAOperand1Register) &&
        ((numberOperandsRGB <= 2) || RGBOperand2Register == ALPHAOperand2Register) &&
        ((numberOperandsRGB <= 3) || RGBOperand3Register == ALPHAOperand3Register))

        unifyRGBA = true;

    /* Special case found in ARB_texture_env_dot3 specification:
     *
     *  "DOT3_RGBA_ARB           4*((Arg0_r - 0.5)*(Arg1_r - 0.5) +
     *                              (Arg0_g - 0.5)*(Arg1_g - 0.5) +
     *                              (Arg0_b - 0.5)*(Arg1_b - 0.5))
     *
     *                          This value is placed into all four
     *                          r,g,b,a components of the output. Note
     *                          that the result generated from
     *                          COMBINE_ALPHA_ARB function is ignored.
     */

    if (RGBOp == DOT3_RGBA)
        unifyRGBA = true;

    string RGBWriteMask;

    if (unifyRGBA)
        RGBWriteMask = "rgba";
    else
        RGBWriteMask = "rgb";

    string SatFlag;

    /* The last instruction of the seqüence generated in this transformation
     * has to output result clamped in [0,1], if clamped is requested.
     * If scale factor is applied (!= 1) then the result is not clamped here,
     * but in the last scaling instruction. See at the end of this
     * transformation code.
     */

    if (clamp && RGBScaleFactor == 1 && ALPHAScaleFactor == 1)
    {
        SatFlag = "_SAT";
    }

    /* In this cases we need a previous instruction that subtracts 0.5 to
     * the arguments and finaly multiplies by a 4 factor.
     */

    if (RGBOp == ADD_SIGNED)
    {
        text += "SUB operand1RGB, " + RGBOperand1Register + ", {0.5,0.5,0.5,0.5};\n";
        RGBOperand1Register = "operand1RGB";
    }
    else if (RGBOp == DOT3_RGB  ||  RGBOp == DOT3_RGBA)
    {
        text += "SUB operand0RGB, " + RGBOperand0Register + ", {0.5,0.5,0.5,0.5};\n";
        text += "SUB operand1RGB, " + RGBOperand1Register + ", {0.5,0.5,0.5,0.5};\n";
        RGBOperand0Register = "operand0RGB";
        RGBOperand1Register = "operand1RGB";
        RGBScaleFactor *= 4;
        if (RGBOp == DOT3_RGBA) ALPHAScaleFactor *= 4;
    }

    /* For "ATI_texture_env_combine3" extension support */
    else if (RGBOp == MODULATE_SIGNED_ADD)
    {
        text += "SUB operand1RGB, " + RGBOperand1Register + ", {0.5,0.5,0.5,0.5};\n";
        RGBOperand1Register = "operand1RGB";
    }

    /* Main instruction creation for combination RGB  */

    if (RGBOp == INTERPOLATE)     // Special case for INTERPOLATE Function.

         /* In ARBfp1.0 the LRP instruction has a different semantic than INTERPOLATE
          *  function operands. Operand register 0 in ARBfp1.0 is the factor of interpolation
          *  while in INTERPOLATE function this factor is the Operand register 1.
          *  Because of this difference is needed to interchange the operands at this time.
          */
    {
        text += CombineOpcodeRGB + SatFlag + " " + RGBDestRegister + "." + RGBWriteMask + ", "
                + RGBOperand2Register + "." + RGBOperand2RegisterSWZ + " , "
                + RGBOperand0Register + "." + RGBOperand0RegisterSWZ + " , "
                + RGBOperand1Register + "." + RGBOperand1RegisterSWZ + ";\n";
    }
    else if ( RGBOp == MODULATE_ADD || RGBOp == MODULATE_SIGNED_ADD || RGBOp == MODULATE_SUBTRACT )
    {
        text += CombineOpcodeRGB + SatFlag + " " + RGBDestRegister + "." + RGBWriteMask + ", ";
        text += RGBOperand0Register + "." + RGBOperand0RegisterSWZ;
        text += " , " + RGBOperand2Register + "." + RGBOperand2RegisterSWZ;

        if (RGBOp == MODULATE_SUBTRACT)
            text += " , -" + RGBOperand1Register + "." + RGBOperand1RegisterSWZ;
        else
            text += " , " + RGBOperand1Register + "." + RGBOperand1RegisterSWZ;

        text += ";\n";
    }
    else if (RGBOp == ADD_4)
    {
        /* ADD_4 :    Arg0 * Arg1 + Arg2 * Arg3 */

        text += "MUL operand2RGB, ";
        text += RGBOperand2Register + "." + RGBOperand2RegisterSWZ;
        text += " , " + RGBOperand3Register + "." + RGBOperand3RegisterSWZ + ";\n";

        text += CombineOpcodeRGB + SatFlag + " " + RGBDestRegister + "." + RGBWriteMask;
        text += ", " + RGBOperand0Register + "." + RGBOperand0RegisterSWZ;
        text += ", " + RGBOperand1Register + "." + RGBOperand1RegisterSWZ;
        text += ", operand2RGB;\n";
    }
    else if (RGBOp == ADD_4_SIGNED)
    {
        /* ADD_4 :    Arg0 * Arg1 + Arg2 * Arg3 - 0.5 */

        text += "MAD operand2RGB, ";
        text += RGBOperand2Register + "." + RGBOperand2RegisterSWZ;
        text += " , " + RGBOperand3Register + "." + RGBOperand3RegisterSWZ;
        text += " , -{0.5,0.5,0.5,0.5};\n";

        text += CombineOpcodeRGB + SatFlag + " " + RGBDestRegister + "." + RGBWriteMask;
        text += ", " + RGBOperand0Register + "." + RGBOperand0RegisterSWZ;
        text += ", " + RGBOperand1Register + "." + RGBOperand1RegisterSWZ;
        text += ", operand2RGB;\n";
    }
    else
    {
        text += CombineOpcodeRGB + SatFlag + " " + RGBDestRegister + "." + RGBWriteMask + ", ";

        text += RGBOperand0Register + "." + RGBOperand0RegisterSWZ;

        if (numberOperandsRGB > 1)
            text += " , " + RGBOperand1Register + "." + RGBOperand1RegisterSWZ;

        if (numberOperandsRGB > 2)
            text += " , " + RGBOperand2Register + "." + RGBOperand2RegisterSWZ;

        text += ";\n";
    }

    /* *************************** ALPHA Combination ************************** */

    if (!unifyRGBA)
    {
        string CombineOpcodeALPHA;
        unsigned int numberOperandsALPHA;

        switch(ALPHAOp)
        {
            case REPLACE:       numberOperandsALPHA = 1; CombineOpcodeALPHA = "MOV"; break;
            case MODULATE:      numberOperandsALPHA = 2; CombineOpcodeALPHA = "MUL"; break;
            case ADD:           numberOperandsALPHA = 2; CombineOpcodeALPHA = "ADD"; break;
            case ADD_SIGNED:    numberOperandsALPHA = 2; CombineOpcodeALPHA = "ADD"; break;
            case INTERPOLATE:   numberOperandsALPHA = 3; CombineOpcodeALPHA = "LRP"; break;
            case SUBTRACT:     numberOperandsALPHA = 2; CombineOpcodeALPHA = "SUB"; break;

            /* For "ATI_texture_env_combine3" extension support */
            case MODULATE_ADD:          numberOperandsALPHA = 3; CombineOpcodeALPHA = "MAD"; break;
            case MODULATE_SIGNED_ADD:   numberOperandsALPHA = 3; CombineOpcodeALPHA = "MAD"; break;
            case MODULATE_SUBTRACT:     numberOperandsALPHA = 3; CombineOpcodeALPHA = "MAD"; break;

            /* For "NV_texture_env_combine4" extension support */
            case ADD_4:                 numberOperandsALPHA = 4; CombineOpcodeALPHA = "MAD"; break;
            case ADD_4_SIGNED:          numberOperandsALPHA = 4; CombineOpcodeALPHA = "MAD"; break;

            default:
                panic("TextureFunctionComputing","TextureFunctionComputing","Unexpected combine ALPHA texture function");
        }

        if (IS_ONE_MINUS_OPERAND(ALPHAOperand0RegisterSWZ))
        {
            text += "SUB operand0ALPHA, {1,1,1,1}, " + ALPHAOperand0Register + ";\n";
            SKIP_ONE_MINUS_SYMBOL(ALPHAOperand0RegisterSWZ);
            ALPHAOperand0Register = "operand0ALPHA";
        }

        if (numberOperandsALPHA > 1 && IS_ONE_MINUS_OPERAND(ALPHAOperand1RegisterSWZ))
        {
            text += "SUB operand1ALPHA, {1,1,1,1}, " + ALPHAOperand1Register + ";\n";
            SKIP_ONE_MINUS_SYMBOL(ALPHAOperand1RegisterSWZ);
            ALPHAOperand1Register = "operand1ALPHA";
        }

        if (numberOperandsALPHA > 2 && IS_ONE_MINUS_OPERAND(ALPHAOperand2RegisterSWZ))
        {
            text += "SUB operand2ALPHA, {1,1,1,1}, " + ALPHAOperand2Register + ";\n";
            SKIP_ONE_MINUS_SYMBOL(ALPHAOperand2RegisterSWZ);
            ALPHAOperand2Register = "operand2ALPHA";
        }

        if (numberOperandsALPHA > 3 && IS_ONE_MINUS_OPERAND(ALPHAOperand3RegisterSWZ))
        {
            text += "SUB operand3ALPHA, {1,1,1,1}, " + ALPHAOperand3Register + ";\n";
            SKIP_ONE_MINUS_SYMBOL(ALPHAOperand3RegisterSWZ);
            ALPHAOperand3Register = "operand3ALPHA";
        }

#undef IS_ONE_MINUS_OPERAND
#undef SKIP_ONE_MINUS_SYMBOL


         /* In this cases we need a previous instruction that subtracts 0.5 to
           * the arguments
           */

        if (ALPHAOp == ADD_SIGNED)
        {
            text += "SUB operand1ALPHA, " + ALPHAOperand1Register + ", {0.5,0.5,0.5,0.5};\n";
        }
        /* For "ATI_texture_env_combine3" extension support */
        else if (ALPHAOp == MODULATE_SIGNED_ADD)
        {
            text += "SUB operand1ALPHA, " + ALPHAOperand1Register + ", {0.5,0.5,0.5,0.5};\n";
            ALPHAOperand1Register = "operand1ALPHA";
        }

        /* Main instruction creation for combination ALPHA  */

        if (ALPHAOp == INTERPOLATE) // Special case for INTERPOLATE Function.

              /* In ARBfp1.0 the LRP instruction has a different semantic than INTERPOLATE
               *  function operands. Operand register 0 in ARBfp1.0 is the factor of interpolation
               *  while in INTERPOLATE function this factor is the Operand register 1.
               *  Because of this difference is needed to interchange the operands at this time.
               */
         {
              text += CombineOpcodeALPHA + SatFlag + " " + ALPHADestRegister + ".a, "
                     + ALPHAOperand2Register + "." + ALPHAOperand2RegisterSWZ + " , "
                     + ALPHAOperand0Register + "." + ALPHAOperand0RegisterSWZ + " , "
                     + ALPHAOperand1Register + "." + ALPHAOperand1RegisterSWZ + ";\n";
         }
         else if ( ALPHAOp == MODULATE_ADD || ALPHAOp == MODULATE_SIGNED_ADD || ALPHAOp == MODULATE_SUBTRACT )
         {
             text += CombineOpcodeALPHA + SatFlag + " " + ALPHADestRegister + ".a, ";
             text += ALPHAOperand0Register + "." + ALPHAOperand0RegisterSWZ;
             text += " , " + ALPHAOperand2Register + "." + ALPHAOperand2RegisterSWZ;

             if (ALPHAOp == MODULATE_SUBTRACT)
                 text += " , -" + ALPHAOperand1Register + "." + ALPHAOperand1RegisterSWZ;
             else
                 text += " , " + ALPHAOperand1Register + "." + ALPHAOperand1RegisterSWZ;

             text += ";\n";
         }
         else if (ALPHAOp == ADD_4)
         {
             /* ADD_4 :    Arg0 * Arg1 + Arg2 * Arg3 */

             text += "MUL operand2ALPHA, ";
             text += ALPHAOperand2Register + "." + ALPHAOperand2RegisterSWZ;
             text += " , " + ALPHAOperand3Register + "." + ALPHAOperand3RegisterSWZ + ";\n";

             text += CombineOpcodeALPHA + SatFlag + " " + ALPHADestRegister + ".a";
             text += ", " + ALPHAOperand0Register + "." + ALPHAOperand0RegisterSWZ;
             text += ", " + ALPHAOperand1Register + "." + ALPHAOperand1RegisterSWZ;
             text += ", operand2ALPHA;\n";
         }
         else if (ALPHAOp == ADD_4_SIGNED)
         {
             /* ADD_4 :    Arg0 * Arg1 + Arg2 * Arg3 - 0.5 */

             text += "MAD operand2ALPHA, ";
             text += ALPHAOperand2Register + "." + ALPHAOperand2RegisterSWZ;
             text += " , " + ALPHAOperand3Register + "." + ALPHAOperand3RegisterSWZ;
             text += " , -{0.5,0.5,0.5,0.5};\n";

             text += CombineOpcodeALPHA + SatFlag + " " + ALPHADestRegister + ".a";
             text += ", " + ALPHAOperand0Register + "." + ALPHAOperand0RegisterSWZ;
             text += ", " + ALPHAOperand1Register + "." + ALPHAOperand1RegisterSWZ;
             text += ", operand2ALPHA;\n";
         }

          else
          {
            text += CombineOpcodeALPHA + SatFlag + " " + ALPHADestRegister + ".a, ";

            text += ALPHAOperand0Register + "." + ALPHAOperand0RegisterSWZ;

            if (numberOperandsALPHA > 1)
                text += " , " + ALPHAOperand1Register + "." + ALPHAOperand1RegisterSWZ;

            if (numberOperandsALPHA > 2)
                text += " , " + ALPHAOperand2Register + "." + ALPHAOperand2RegisterSWZ;

            text += ";\n";
         }

    }

    /* ********************** Final rescaling and clamping ************************ */

    stringstream ss;
    ss << RGBScaleFactor;
    string rgbscalefactor = ss.str();

    stringstream ss2;
    ss2 << ALPHAScaleFactor;
    string alphascalefactor = ss2.str();

    if (RGBScaleFactor != 1 || ALPHAScaleFactor != 1)
    {
        if (clamp)
            text += "MUL_SAT " + RGBDestRegister + " , " + RGBDestRegister + " , { " + rgbscalefactor + "," + rgbscalefactor + "," + rgbscalefactor + "," + alphascalefactor + " };\n";
        else
            text += "MUL " + RGBDestRegister + " , " + RGBDestRegister + " , { " + rgbscalefactor + "," + rgbscalefactor + "," + rgbscalefactor + "," + alphascalefactor + " };\n";
    }

    setCode(text);
}


ObjectLinearGeneration::ObjectLinearGeneration(unsigned int maxTexUnits, vector<string>& coordinates, string oCord, string iPos)
{
    string text = "";

    for (unsigned int i=0; i<maxTexUnits; i++)
    {
        stringstream ss;
        ss << i;
        string unit = ss.str();

        if (!(coordinates[i].empty()))
            text += "# Compute the Object Linear texture coordinate generation for " + coordinates[i] + ""
                    " in Texture Unit " + unit + "\n";

        if (coordinates[i].find("s") != string::npos )
            text += "DP4 " + oCord + unit + ".x, state.texgen[" + unit + "].object.s, " + iPos + ";\n";

        if (coordinates[i].find("t") != string::npos )
            text += "DP4 " + oCord + unit + ".y, state.texgen[" + unit + "].object.t, " + iPos + ";\n";

        if (coordinates[i].find("r") != string::npos )
            text += "DP4 " + oCord + unit + ".z, state.texgen[" + unit + "].object.r, " + iPos + ";\n";

        if (coordinates[i].find("q") != string::npos )
            text += "DP4 " + oCord + unit + ".w, state.texgen[" + unit + "].object.q, " + iPos + ";\n";
    }
    setCode(text);
}

EyeLinearGeneration::EyeLinearGeneration(unsigned int maxTexUnits, vector<string>& coordinates, string oCord, string eyePos)
{
    string text = "";

    for (unsigned int i=0; i<maxTexUnits; i++)
    {
        stringstream ss;
        ss << i;
        string unit = ss.str();

        if (!(coordinates[i].empty()))
            text += "# Compute the Eye Linear texture coordinate generation for " + coordinates[i] + ""
                    " in Texture Unit " + unit + "\n";

        if (coordinates[i].find("s") != string::npos )
            text += "DP4 " + oCord + unit + ".x, state.texgen[" + unit + "].eye.s, " + eyePos + ";\n";

        if (coordinates[i].find("t") != string::npos )
            text += "DP4 " + oCord + unit + ".y, state.texgen[" + unit + "].eye.t, " + eyePos + ";\n";

        if (coordinates[i].find("r") != string::npos )
            text += "DP4 " + oCord + unit + ".z, state.texgen[" + unit + "].eye.r, " + eyePos + ";\n";

        if (coordinates[i].find("q") != string::npos )
            text += "DP4 " + oCord + unit + ".w, state.texgen[" + unit + "].eye.q, " + eyePos + ";\n";
    }
    setCode(text);
}

ReflectedVectorComputing::ReflectedVectorComputing(string normalEye, string eyePos)
{
    string text = "# Compute reflected Vector\n"
                  "DP3 reflectedVector.w, " + normalEye + ", " + eyePos + ";\n"
                  "MUL reflectedVector.xyz, " + normalEye + ", reflectedVector.wwww;\n"
                  "MAD reflectedVector.xyz, reflectedVector, {-2,-2,-2,-2}, " + eyePos + ";\n";
    setCode(text);
}

SphereMapGeneration::SphereMapGeneration(unsigned int maxTexUnits, vector<string>& coordinates, string oCord, string normalEye)
{
    string text = "TEMP sphereMapVector;\n"
                  "MAD sphereMapVector.xyz, reflectedVector, {2,2,2,2}, {0,0,2,0};\n"
                  "DP3 sphereMapVector.w, sphereMapVector, sphereMapVector;\n"
                  "RSQ sphereMapVector.w, sphereMapVector.w;\n"
                  "MUL sphereMapVector.w, sphereMapVector.w, {0.5,0.5,0.5,0.5};\n";

    for (unsigned int i=0; i<maxTexUnits; i++)
    {
        stringstream ss;
        ss << i;
        string unit = ss.str();
        string mask;

        if (coordinates[i].find("s") != string::npos ) mask += "x";
        if (coordinates[i].find("t") != string::npos ) mask += "y";
        if (coordinates[i].find("r") != string::npos ) mask += "z";
        if (coordinates[i].find("q") != string::npos ) mask += "w";

        if (!mask.empty())
            text += "# Compute the Sphere Map texture coordinate generation for " + coordinates[i] + ""
                    " in Texture Unit " + unit + "\n"
                    "MAD " + oCord + unit + "." + mask + ", sphereMapVector, sphereMapVector.w, {0.5,0.5,0.5,0.5};\n";
    }

    setCode(text);
}

ReflectionMapGeneration::ReflectionMapGeneration(unsigned int maxTexUnits, vector<string>& coordinates, string oCord, string normalEye, string reflectedVector)
{
    string text = "";

    for (unsigned int i=0; i<maxTexUnits; i++)
    {
        stringstream ss;
        ss << i;
        string unit = ss.str();
        string mask;

        if (coordinates[i].find("s") != string::npos ) mask += "x";
        if (coordinates[i].find("t") != string::npos ) mask += "y";
        if (coordinates[i].find("r") != string::npos ) mask += "z";
        if (coordinates[i].find("q") != string::npos ) mask += "w";

        if (!mask.empty())
            text += "# Compute the Reflection Map texture coordinate generation for " + coordinates[i] + ""
                    " in Texture Unit " + unit + "\n"
                    "MOV " + oCord + unit + "." + mask + ", " + reflectedVector + ";\n";
    }

    setCode(text);
}

NormalMapGeneration::NormalMapGeneration(unsigned int maxTexUnits, vector<string>& coordinates, string oCord, string normalEye)
{
    string text = "";

    for (unsigned int i=0; i<maxTexUnits; i++)
    {
        stringstream ss;
        ss << i;
        string unit = ss.str();
        string mask;

        if (coordinates[i].find("s") != string::npos ) mask += "x";
        if (coordinates[i].find("t") != string::npos ) mask += "y";
        if (coordinates[i].find("r") != string::npos ) mask += "z";
        if (coordinates[i].find("q") != string::npos ) mask += "w";

        if (!mask.empty())
            text += "# Compute the Normal Map texture coordinate generation for " + coordinates[i] + " \n"
                     "MOV " + oCord + unit + "." + mask + ", " + normalEye + ";\n";
    }

    setCode(text);
}

/*
 *     LINEAR:
 *       #
 *       # Linear fog
 *       # f = (end-z)/(end-start)
 *       #
 *       PARAM p = {-1/(END-START), END/(END-START), NOT USED, NOT USED};
 *       PARAM fogColor = state.fog.color;
 *       TEMP fogFactor;
 *       ATTRIB fogCoord = fragment.fogcoord.x;
 *       MAD_SAT fogFactor.x, p.x, fogCoord.x, p.y;
 *       LRP result.color.rgb, fogFactor.x, finalColor, fogColor;
 *
 */

LinearFogFactorComputing::LinearFogFactorComputing(string fogParam, string fogFactor)
{
    string text = "# -------- Linear Fog factor computing ---------\n"
                  "MAD_SAT " + fogFactor + ".x, " + fogParam + ".x, fragment.fogcoord.x, " + fogParam + ".y;\n";
    setCode(text);
}


/*
 *     EXP:
 *       # Exponential fog
 *       # f = exp(-d*z)
 *       #
 *       PARAM p = {DENSITY/LN(2), NOT USED, NOT USED, NOT USED};
 *       PARAM fogColor = state.fog.color;
 *       TEMP fogFactor;
 *       ATTRIB fogCoord = fragment.fogcoord.x;
 *       MUL fogFactor.x, p.x, fogCoord.x;
 *       EX2_SAT fogFactor.x, -fogFactor.x;
 *       LRP result.color.rgb, fogFactor.x, finalColor, fogColor;
 */

ExponentialFogFactorComputing::ExponentialFogFactorComputing(string fogParam, string fogFactor)
{
    string text = "# ------ Exponential fog factor computing -------\n"
                  "MUL " + fogFactor + ".x, " + fogParam + ".x, fragment.fogcoord.x;\n"
                  "EX2_SAT " + fogFactor + ".x, -" + fogFactor + ".x;\n";
    setCode(text);
}

/*
 *     EXP2:
 *       #
 *       # 2nd-order Exponential fog
 *       # f = exp(-(d*z)^2)
 *       #
 *       PARAM p = {DENSITY/SQRT(LN(2)), NOT USED, NOT USED, NOT USED};
 *       PARAM fogColor = state.fog.color;
 *       TEMP fogFactor;
 *       ATTRIB fogCoord = fragment.fogcoord.x;
 *       MUL fogFactor.x, p.x, fogCoord.x;
 *       MUL fogFactor.x, fogFactor.x, fogFactor.x;
 *       EX2_SAT fogFactor.x, -fogFactor.x;
 *       LRP result.color.rgb, fogFactor.x, finalColor, fogColor;
 */

SecondOrderExponentialFogFactorComputing::SecondOrderExponentialFogFactorComputing(string fogParam, string fogFactor)
{
    string text = "# ----- 2nd order exponential fog factor computing ------\n"
                  "MUL " + fogFactor + ".x, " + fogParam + ".x, fragment.fogcoord.x;\n"
                  "MUL " + fogFactor + ".x, " + fogFactor + ".x, " + fogFactor + ".x;\n"
                  "EX2_SAT " + fogFactor + ".x, -" + fogFactor + ".x;\n";
    setCode(text);
}

NormalizeVector::NormalizeVector(std::string vector)
{
    string text = "# Normalize " +  vector + " vector\n"
                  "DP3 " + vector + ".w, " + vector + ", " + vector + ";\n"
                  "RSQ " + vector + ".w, " + vector + ".w;\n"
                  "MUL " + vector + ".xyz, " + vector + ", " + vector + ".w;\n";
    setCode(text);
}
