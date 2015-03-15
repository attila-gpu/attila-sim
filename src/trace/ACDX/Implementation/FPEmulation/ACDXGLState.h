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

#ifndef ACDX_GLSTATE_H
    #define ACDX_GLSTATE_H

#include "gl.h"
#include "ACDXQuadReg.h"
#include "ACDXImplementationLimits.h"
#include "ACDXMatrixf.h"
#include "support.h"
#include <iostream>

namespace acdlib
{

typedef ACDXQuadReg<float> Quadf;

/* one token for each matrix category */
enum MatrixId
{
    M_MODELVIEW = 0,
    M_PROJECTION = STATE_MATRIX_PROJECTION_OFFSET,
    M_MVP = STATE_MATRIX_MVP_OFFSET,
    M_TEXTURE = STATE_MATRIX_TEXTURE_OFFSET,
    M_PALETTE = STATE_MATRIX_PALETTE_OFFSET,
    M_PROGRAM = STATE_MATRIX_PROGRAM_OFFSET
};

/* type of transformed matrix */
enum MatrixType
{
    MT_NONE = 0,
    MT_INVERSE = 1,
    MT_TRANSPOSE = 2,
    MT_INVTRANS = 3
};

/* one token for each vector */
enum VectorId
{
    V_MATERIAL_FRONT_AMBIENT = 0,
    V_MATERIAL_FRONT_DIFUSSE = 1,
    V_MATERIAL_FRONT_SPECULAR = 2,
    V_MATERIAL_FRONT_EMISSION = 3,
    V_MATERIAL_FRONT_SHININESS = 4,    
    V_MATERIAL_BACK_AMBIENT = 5,
    V_MATERIAL_BACK_DIFUSSE = 6,
    V_MATERIAL_BACK_SPECULAR = 7,
    V_MATERIAL_BACK_EMISSION = 8,
    V_MATERIAL_BACK_SHININESS = 9,
                
    V_LIGHT_AMBIENT = 10,
    V_LIGHT_DIFFUSE = 11,
    V_LIGHT_SPECULAR = 12,
    V_LIGHT_POSITION = 13,
    V_LIGHT_ATTENUATION = 14,
    V_LIGHT_SPOT_DIRECTION = 15,
    V_LIGHT_HALF = 16,    
    // ... Implicit enums for lights 1, 2, ..., MAX_LIGHTS_ARB-1
    
    V_LIGHTMODEL_AMBIENT = BASE_LIGHTMODEL,
    V_LIGHTMODEL_FRONT_SCENECOLOR = BASE_LIGHTMODEL+1,
    V_LIGHTMODEL_BACK_SCENECOLOR = BASE_LIGHTMODEL+2,
    
    V_LIGHTPROD_FRONT_AMBIENT = BASE_LIGHTPROD,
    V_LIGHTPROD_FRONT_DIFUSSE = BASE_LIGHTPROD + 1,
    V_LIGHTPROD_FRONT_SPECULAR = BASE_LIGHTPROD + 2,
    V_LIGHTPROD_BACK_AMBIENT = BASE_LIGHTPROD + 3,
    V_LIGHTPROD_BACK_DIFUSSE = BASE_LIGHTPROD + 4,
    V_LIGHTPROD_BACK_SPECULAR = BASE_LIGHTPROD + 5,
    // ... Implicit enums for lightprods 1, 2, ... MAX_LIGHTS_ARB-1
    
    V_TEXGEN_EYE_S = BASE_TEXGEN,
    V_TEXGEN_EYE_T = BASE_TEXGEN + 1,
    V_TEXGEN_EYE_R = BASE_TEXGEN + 2,
    V_TEXGEN_EYE_Q = BASE_TEXGEN + 3,
    V_TEXGEN_OBJECT_S = BASE_TEXGEN + 4,
    V_TEXGEN_OBJECT_T = BASE_TEXGEN + 5,
    V_TEXGEN_OBJECT_R = BASE_TEXGEN + 6,
    V_TEXGEN_OBJECT_Q = BASE_TEXGEN + 7,
    // ... implicit enums for texgen 1, 2, ... MAX_TEXTURE_UNITS_ARB - 1
    
    V_FOG_COLOR = BASE_FOG,
    V_FOG_PARAMS = BASE_FOG + 1,
    
    V_TEXENV_COLOR = BASE_TEXENV
    // ... Implicit enums for texenv 1, 2, ..., MAX_TEXTURE_UNITS_ARB - 1
    
};


/**
 * Container that keeps all the state visible for a ARB Vertex Program & Fragment programs
 *
 * Vector & Matrices States
 *
 * It also manages (and computes) all derivate state, for example: "the inverse of modelview[0]"
 * Accessible with: gls.getMatrix(M_MODELVIEW, MT_INVERSE, 0);
 */
class ACDXGLState
{

private:

    /**
     * declaring this object
     * avoid msg: "binary '<<' : no operator defined..."
     * (VS.6.0)
     */
    //QuadReg<float> dummy;
    Quadf dummy;

    enum 
    {

        /* Number of vectors */
        VECTORS_SIZE = BASE_STATE_MATRIX,

        /* Number of matrices */
        MATRICES_SIZE  = (2 + MAX_VERTEX_UNITS_ARB + MAX_TEXTURE_UNITS_ARB + 
                          MAX_PALETTE_MATRICES_ARB + MAX_PROGRAM_MATRICES_ARB)*4
    };
    
    /* Vector States */
    mutable Quadf vectorState[BASE_STATE_MATRIX];
    mutable bool dirtyVec[BASE_STATE_MATRIX];

    /* Matrix States */
    /* mutable to allow updates in dependencies */
    mutable ACDXMatrixf matrixState[MATRICES_SIZE];
    mutable bool dirtyMat[MATRICES_SIZE];

    /**
     * Recompute a given matrix if it must be updated
     */
    void checkMatrix(GLuint matId) const;

    /**
     * Print the four matrices availables (none,inv,trans,invtrans) given a matId
     */
    void printMatrix(std::ostream& os, MatrixId matId_) const;

public:

    ACDXGLState();

    /* Sets new contents in a matrix */
    void setMatrix(MatrixId mid, GLuint unit, const ACDXMatrixf& mat);
    void setMatrix(const ACDXMatrixf& data, GLuint id);

    /* Interface for getting a matrix easily */
    const ACDXMatrixf& getMatrix(MatrixId mid, MatrixType mtype, GLuint unit = 0) const;
    
    /* Compiler interface for getting a matrix */
    const ACDXMatrixf& getMatrix(GLuint id) const;    

    /* checks if and identifier is a matrix */
    static bool isMatrix(GLuint id);

    /**
     * Sets the state of a vector given in the Quadf object
     */
    void setVector(const Quadf&, VectorId id);
    
    /**
     * Same than before but using a write mask
     *
     * (Only uses the last 4 bits of mask)
     *
     * mask is read like that: 0001 means just q[0] is written, 0101 means q[0] & q[2] are written.
     * (note mask are interpreted backwards)
     *
     * This method is provided for avoiding read/writes in partial vector updates
     */
    void setVector(const Quadf&, VectorId id, GLubyte mask);
    
    
    /**
     * Versions for groups (LIGHTS, TEXGEN, etx)
     */
    void setVectorGroup(const Quadf&, VectorId id, GLuint unit);
    void setVectorGroup(const Quadf& data, VectorId id, GLuint unit,GLubyte mask);

    const Quadf& getVector(GLuint id) const;

    void dump(std::ostream& os= std::cout) const;
  
    
    static void normalizeQuad(Quadf& qr);
};

} // namespace acdlib

#endif // ACDX_GLSTATE_H

