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
 
#include "GLState.h"
#include "AuxFuncsLib.h"

using namespace std;
using namespace libgl::glsNS;


GLState::GLState()
{
    memset(dirtyMat,0,sizeof(dirtyMat));
    
    /* Init all matrices to identity */ 
    setMatrix(M_MODELVIEW, 0, Matrixf::identity());
    setMatrix(M_PROJECTION, 0, Matrixf::identity());

    int i;
    for ( i = 0;  i < MAX_TEXTURE_UNITS_ARB; i++ )
           setMatrix(M_TEXTURE, i, Matrixf::identity());
    
    /* Set material defaults */
    setVector(Quadf(0.2f, 0.2f, 0.2f, 1.0f), V_MATERIAL_FRONT_AMBIENT);
    setVector(Quadf(0.8f, 0.8f, 0.8f, 1.0f), V_MATERIAL_FRONT_DIFUSSE);
    setVector(Quadf(0.0f, 0.0f, 0.0f, 1.0f), V_MATERIAL_FRONT_SPECULAR);
    setVector(Quadf(0.0f, 0.0f, 0.0f, 1.0f), V_MATERIAL_FRONT_EMISSION);    
    setVector(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_MATERIAL_FRONT_SHININESS);    
    setVector(Quadf(0.2f, 0.2f, 0.2f, 1.0f), V_MATERIAL_BACK_AMBIENT);
    setVector(Quadf(0.8f, 0.8f, 0.8f, 1.0f), V_MATERIAL_BACK_DIFUSSE);
    setVector(Quadf(0.0f, 0.0f, 0.0f, 1.0f), V_MATERIAL_BACK_SPECULAR);
    setVector(Quadf(0.0f, 0.0f, 0.0f, 1.0f), V_MATERIAL_BACK_EMISSION);    
    setVector(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_MATERIAL_BACK_SHININESS);

    /* Set Light defaults */                    
    setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 1.0f), V_LIGHT_AMBIENT, 0);
    setVectorGroup(Quadf(1.0f, 1.0f, 1.0f, 1.0f), V_LIGHT_DIFFUSE, 0);
    setVectorGroup(Quadf(1.0f, 1.0f, 1.0f, 1.0f), V_LIGHT_SPECULAR, 0);
    setVectorGroup(Quadf(0.0f, 0.0f, 1.0f, 0.0f), V_LIGHT_POSITION,0);
    setVectorGroup(Quadf(1.0f, 0.0f, 0.0f, 0.0f), V_LIGHT_ATTENUATION,0);
    setVectorGroup(Quadf(0.0f, 0.0f, -1.0f, -1.0f), V_LIGHT_SPOT_DIRECTION, 0);        
    
    
    for ( i = 1; i < MAX_LIGHTS_ARB; i++ )
    {
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 1.0f), V_LIGHT_AMBIENT, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_LIGHT_DIFFUSE, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_LIGHT_SPECULAR, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 1.0f, 0.0f), V_LIGHT_POSITION, i);
        setVectorGroup(Quadf(1.0f, 0.0f, 0.0f, 0.0f), V_LIGHT_ATTENUATION, i);
        setVectorGroup(Quadf(0.0f, 0.0f, -1.0f, -1.0f), V_LIGHT_SPOT_DIRECTION, i);    
    }
    
    setVector(Quadf(0.2f, 0.2f, 0.2f, 1.0f), V_LIGHTMODEL_AMBIENT);
    
    for ( i = 0;  i < MAX_TEXTURE_UNITS_ARB; i++ )
    {
        setVectorGroup(Quadf(1.0f, 0.0f, 0.0f, 0.0f), V_TEXGEN_EYE_S, i);
        setVectorGroup(Quadf(0.0f, 1.0f, 0.0f, 0.0f), V_TEXGEN_EYE_T, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_TEXGEN_EYE_R, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_TEXGEN_EYE_Q, i);

        setVectorGroup(Quadf(1.0f, 0.0f, 0.0f, 0.0f), V_TEXGEN_OBJECT_S, i);
        setVectorGroup(Quadf(0.0f, 1.0f, 0.0f, 0.0f), V_TEXGEN_OBJECT_T, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_TEXGEN_OBJECT_R, i);
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_TEXGEN_OBJECT_Q, i);
    }
    
    setVector(Quadf(0.0, 0.0, 0.0, 0.0), V_FOG_COLOR);
    setVector(Quadf(1.0, 0.0, 1.0, 1.0), V_FOG_PARAMS);
    
    for ( i = 0;  i < MAX_TEXTURE_UNITS_ARB; i++ )
        setVectorGroup(Quadf(0.0f, 0.0f, 0.0f, 0.0f), V_TEXENV_COLOR, i);
    
}

bool GLState::isMatrix(GLuint id)
{
    return (id >= VECTORS_SIZE);
}


void GLState::setVector(const Quadf& data, VectorId id)
{
    if ( GLuint(id) < VECTORS_SIZE )
        vectorState[id] = data;
    else
        panic("GLSTate","setVectorState()", "State ID is not a Vector State");
}

void GLState::setVector(const Quadf& data, VectorId id, GLubyte mask)
{
    if ( GLuint(id) >= VECTORS_SIZE )
        panic("GLSTate","setVectorState()", "State ID is not a Vector State");
    
    if ( (mask & 1) != 0 )
        vectorState[id][0] = data[0];
    
    if ( (mask & 2) != 0 )
        vectorState[id][1] = data[1];
    
    if ( (mask & 4) != 0 )
        vectorState[id][2] = data[2];

    if ( (mask & 8) != 0 )
        vectorState[id][3] = data[3];    
    
}

void GLState::setVectorGroup(const Quadf& data, VectorId id, GLuint unit)
{
    GLuint i;
    switch ( id )
    {
        /* LIGHTS */
        case V_LIGHT_AMBIENT:
        case V_LIGHT_DIFFUSE:
        case V_LIGHT_SPECULAR:
        case V_LIGHT_POSITION:
        case V_LIGHT_ATTENUATION:
        case V_LIGHT_SPOT_DIRECTION:
            if ( unit >= MAX_LIGHTS_ARB )
                panic("GLState","setVectorGroup()", "Light unit does not exist (too high)");
            i = 7*unit + GLuint(id);
            setVector(data, VectorId(i));
            break; // OK
        case V_LIGHT_HALF:
            panic("GLState","setVectorGroup()", "HALF is derivative, cannot be set directly");
            break;
        /* TEXTURE GENERATION */
        case V_TEXGEN_EYE_S:
        case V_TEXGEN_EYE_T:
        case V_TEXGEN_EYE_R:
        case V_TEXGEN_EYE_Q:
        case V_TEXGEN_OBJECT_S:
        case V_TEXGEN_OBJECT_T:
        case V_TEXGEN_OBJECT_R:
        case V_TEXGEN_OBJECT_Q:
            if ( unit >= MAX_TEXTURE_UNITS_ARB )
                panic("GLState", "setVectorGroup", "Texture unit does not exist (too high)");
            i = 8*unit + GLuint(id);
            setVector(data, VectorId(i));
            break;
        case V_TEXENV_COLOR:
            if ( unit >= MAX_TEXTURE_UNITS_ARB )
                panic("GLState", "setVectorGroup", "Texture unit does not exist (too high)");
            i = unit + GLuint(id);
            setVector(data, VectorId(i));
            break;
        default:
            panic("GLState","setVectorGroup()", "This vector does not pertain to a unit or cannot be set directly (derivative)");            
    }       
}

void GLState::setVectorGroup(const Quadf& data, VectorId id, GLuint unit,GLubyte mask)
{    
    GLuint i;
    switch ( id )
    {
        case V_LIGHT_AMBIENT:
        case V_LIGHT_DIFFUSE:
        case V_LIGHT_SPECULAR:
        case V_LIGHT_POSITION:
        case V_LIGHT_ATTENUATION:
        case V_LIGHT_SPOT_DIRECTION:
            if ( unit >= MAX_LIGHTS_ARB )
                panic("GLState","setVectorGroup()", "Unit does not exist (too high)");
            i  = 7*unit + GLuint(id);
            setVector(data, VectorId(i), mask); // write with mask
            break; // OK
        case V_LIGHT_HALF:
            panic("GLState","setVectorGroup()", "HALF is derivative, cannot be set directly");
        default:
            panic("GLState","setVectorGroup()", "This vector does not pertain to a unit or cannot be set directly (derivative)");
    }       
}


void GLState::setMatrix(const Matrixf& data, GLuint id)
{
    /* do not allow to modify derivative matrices directly */
    if ( id % 4 != 0 )
        panic("GLState","setMatrixState()","It is not possible to modify a derivative matrix");

    if ( id == STATE_MATRIX_MVP_OFFSET )
        /* MVP is fully derivative Modelview*projection */
        panic("GLState","setMatrixState()", "MVP matrix can not be updated, it derives from Modelview & Projection");   


    /* update derivative matrices flags (mark as dirtyMat) */
    dirtyMat[id+1] = true;
    dirtyMat[id+2] = true;
    dirtyMat[id+3] = true;

    if ( id == STATE_MATRIX_MODELVIEW_OFFSET || id == STATE_MATRIX_PROJECTION_OFFSET )
    {
        /* Only modelview0 affects MVP */
        dirtyMat[STATE_MATRIX_MVP_OFFSET] = true;
        dirtyMat[STATE_MATRIX_MVP_OFFSET+1] = true;
        dirtyMat[STATE_MATRIX_MVP_OFFSET+2] = true;
        dirtyMat[STATE_MATRIX_MVP_OFFSET+3] = true;
    }


    /* set new data */

    matrixState[id] = data;
}



void GLState::checkMatrix(GLuint id) const
{
    if ( dirtyMat[id] )
    {
        Matrixf& m = matrixState[id]; /* get a reference to the target matrix */
        
        dirtyMat[id] = false; /* mark as updated */
        
        GLuint generative = id - (id%4); /* compute its generative matrix */

        if ( generative == M_MVP )
        {
            /* We always recompute MVP, but if it's not dirtyMat it's not required */
            /* Compute MVP */
            Matrixf proj = getMatrix(M_PROJECTION, MT_NONE, 0);
            Matrixf mview = getMatrix(M_MODELVIEW, MT_NONE, 0);
            m = proj * mview;
        }
        else 
        {
            /* copy generative */
            m = matrixState[generative]; 
        }       

        switch ( id % 4 )
        {
            /* apply selected transformation */
            case MT_NONE:
                if ( generative != M_MVP )
                    panic("GLState","checkMatrix","Matrix that cannot be dirtyt!");
                break;
            case MT_INVERSE:
                m.inverseMe();
                break;
            case MT_TRANSPOSE:
                m.transposeMe();
                break;
            case MT_INVTRANS:
                m.invtransMe();
                break;
            default:
                panic("GLState","checkMatrix()","Implementation error: see 'switch(id % 4)' line");
        }

    }
}

const Matrixf& GLState::getMatrix(GLuint id) const

{
    if ( id >= VECTORS_SIZE + MATRICES_SIZE )
        panic("GLState","getMatrix(GLuint)","Access out of range");

    id = id - VECTORS_SIZE;
    checkMatrix(id); /* update matrix if it's required */
    return matrixState[id];
}


const Matrixf& GLState::getMatrix(MatrixId mid, MatrixType mtype, GLuint unit) const
{
    if ( M_MODELVIEW > mid || mid > M_PROGRAM )
    {
        panic("GLState", "getMatrix()", "Matrix identifier unknown");
    }

    GLuint id = GLuint(mid);

    switch ( mid )
    {
        case M_PROJECTION:
        case M_MVP:
            unit = 0; /* unit ignored */
            break;
        case M_MODELVIEW:
            if ( unit >= MAX_VERTEX_UNITS_ARB )
                panic("GLState","getMatrix()","VERTEX_UNIT not available (M_MODELVIEW)");
            break;
        case M_TEXTURE:
            if ( unit >= MAX_TEXTURE_UNITS_ARB )
                panic("GLState","getMatrix()","TEXTURE_UNIT not available (M_TEXTURE)");
            break;
        case M_PALETTE:
            if ( unit >= MAX_PALETTE_MATRICES_ARB )
                panic("GLState","getMatrix()","PALETTE_UNIT not available (M_PALETTE)");
            break;
        case M_PROGRAM:
            if ( unit >= MAX_PROGRAM_MATRICES_ARB )
                panic("GLState","getMatrix()","PROGRAM_UNIT not available (M_PROGRAM)");
            break;
        default:
            panic("GLState", "getMatrix()", "Unknown matrix identifier");
    }
    
    id += (unit * 4); // skip previous matrices (of other units)
    // checkMatrix(id);

    /* absolute access */
    return getMatrix(id + BASE_STATE_MATRIX + mtype);
}


void GLState::setMatrix(MatrixId mid, GLuint unit, const Matrixf& mat)
{
    if ( M_MODELVIEW > mid || mid > M_PROGRAM )
        panic("GLState", "setMatrix()", "Matrix identifier unknown");

    switch ( mid )
    {
        case M_PROJECTION:
        case M_MVP:
            unit = 0;
            break;
        case M_MODELVIEW:
            if ( unit >= MAX_VERTEX_UNITS_ARB )
                panic("GLState","setMatrix()","VERTEX_UNIT not available (M_MODELVIEW)");
            break;
        case M_TEXTURE:
            if ( unit >= MAX_TEXTURE_UNITS_ARB )
                panic("GLState","setMatrix()","TEXTURE_UNIT not available (M_TEXTURE)");
            break;
        case M_PALETTE:
            if ( unit >= MAX_PALETTE_MATRICES_ARB )
                panic("GLState","setMatrix()","PALETTE_MATRIX not available (M_PALETTE)");
            break;
        case M_PROGRAM:
            if ( unit >= MAX_PROGRAM_MATRICES_ARB )
                panic("GLState","setMatrix()","PROGRAM_MATRIX not available (M_PROGRAM)");
            break;
        default:
            panic("GLState","setMatrix()","Matrix identifier unknow (switch(mid) )");
    }

    /* only generative matrix can be set */
    GLuint offset = GLuint(mid) + GLuint(MT_NONE); 
    setMatrix(mat, 4*unit + offset);
}




const Quadf& GLState::getVector(GLuint id) const
{
    if ( id >= VECTORS_SIZE )
        panic("GLState","getVector()", "Trying to get a vector out of range");
        // Accesing vector state
            
    if ( id == V_LIGHTMODEL_FRONT_SCENECOLOR )
    {
        QuadReg<float>& a_cs = vectorState[V_LIGHTMODEL_AMBIENT];
        QuadReg<float>& a_cm = vectorState[V_MATERIAL_FRONT_AMBIENT];
        QuadReg<float>& e_cm = vectorState[V_MATERIAL_FRONT_EMISSION];
        
        vectorState[id][0] = a_cs[0]*a_cm[0] + e_cm[0];
        vectorState[id][1] = a_cs[1]*a_cm[1] + e_cm[1];
        vectorState[id][2] = a_cs[2]*a_cm[2] + e_cm[2];
        vectorState[id][3] = a_cs[3]*a_cm[3] + e_cm[3];
    }
    
    if ( id == V_LIGHTMODEL_BACK_SCENECOLOR )
    {
        QuadReg<float>& a_cs = vectorState[V_LIGHTMODEL_AMBIENT];
        QuadReg<float>& a_cm = vectorState[V_MATERIAL_BACK_AMBIENT];
        QuadReg<float>& e_cm = vectorState[V_MATERIAL_BACK_EMISSION];
        
        vectorState[id][0] = a_cs[0]*a_cm[0] + e_cm[0];
        vectorState[id][1] = a_cs[1]*a_cm[1] + e_cm[1];
        vectorState[id][2] = a_cs[2]*a_cm[2] + e_cm[2];
        vectorState[id][3] = a_cs[3]*a_cm[3] + e_cm[3];
    }
    
    else if ( id == V_FOG_PARAMS )
    {
        QuadReg<float>& v = vectorState[id]; // use a reference
        v[3] = 1.0f / (v[2] - v[1]);
    }
    
    else if ( id < BASE_LIGHTMODEL && id >= BASE_LIGHTS && ((id - V_LIGHT_HALF) % 7 == 0) ) // lights[n].half
    {
        dirtyVec[id] = true; // Force recomputation each time... 
        if ( dirtyVec[id] )
        {
            Quadf a = getVector(id-3); 
            afl::normalizeQuad(a);
            Quadf& b = vectorState[id];
            b = Quadf(0,0,1,0);
            b[0] = a[0]+b[0]; b[1] = a[1]+b[1]; b[2] = a[2]+b[2]; b[3] = a[3]+b[3];
            afl::normalizeQuad(b);
            dirtyVec[id] = false;
        } 
    }   
    
    else if ( BASE_LIGHTPROD <= id && id < BASE_TEXGEN  ) // Lightprod
    {
        dirtyVec[id] = true; // Force recomputation each time... 
        if ( dirtyVec[id] ) /* recompute.... */
        {
            GLuint state = (id - BASE_LIGHTPROD) % 6;
            GLuint unit = (id - BASE_LIGHTPROD) / 6;
            
            GLuint material;
            GLuint light = BASE_LIGHTS + 7*unit + (state % 3); // select light
            switch ( state)
            {
                case 0: // LIGHTPROD.unit.front.ambient
                    material = V_MATERIAL_FRONT_AMBIENT;
                    break;
                case 1: // LIGHTPROD.unit.front.difusse
                    material = V_MATERIAL_FRONT_DIFUSSE;
                    break;
                case 2: // LIGHTPROD.unit.front.specular
                    material = V_MATERIAL_FRONT_SPECULAR;
                    break;
                case 3: // LIGHTPROD.unit.back.ambient
                    material = V_MATERIAL_BACK_AMBIENT; 
                    break;
                case 4: // LIGHTPROD.unit.back.difusse
                    material = V_MATERIAL_BACK_DIFUSSE;
                    break;
                case 5: // LIGHTPROD.unit.back.specular
                    material = V_MATERIAL_BACK_SPECULAR;
                    break;
                default:
                    panic("GLState","getVector()", "Should not happen ever (error programming?)");
            }
            
            Quadf& a = vectorState[material]; 
            Quadf& b = vectorState[light]; 
            vectorState[id] = Quadf(a[0]*b[0],  a[1]*b[1], a[2]*b[2], a[3]*b[3]);
            dirtyVec[id] = false; // reset dirty flag
        }
    }
    
    return vectorState[id];    
}


void GLState::dump(ostream& os) const
{   
    int i;
    
    os << "--- VECTOR STATES ---" << endl;
    os << "MATERIAL.FRONT.AMBIENT = " << vectorState[V_MATERIAL_FRONT_AMBIENT] << endl;
    os << "MATERIAL.FRONT.DIFUSSE = " << vectorState[V_MATERIAL_FRONT_DIFUSSE] << endl;
    os << "MATERIAL.FRONT.SPECULAR = " << vectorState[V_MATERIAL_FRONT_SPECULAR] << endl;
    os << "MATERIAL.FRONT.EMISSION = " << vectorState[V_MATERIAL_FRONT_EMISSION] << endl;
    os << "MATERIAL.FRONT.SHININESS = " << vectorState[V_MATERIAL_FRONT_SHININESS] << endl;
    
    os << "MATERIAL.BACK.AMBIENT = " << vectorState[V_MATERIAL_BACK_AMBIENT] << endl;
    os << "MATERIAL.BACK.DIFUSSE = " << vectorState[V_MATERIAL_BACK_DIFUSSE] << endl;
    os << "MATERIAL.BACK.SPECULAR = " << vectorState[V_MATERIAL_BACK_SPECULAR] << endl;
    os << "MATERIAL.BACK.EMISSION = " << vectorState[V_MATERIAL_BACK_EMISSION] << endl;
    os << "MATERIAL.BACK.SHININESS = " << vectorState[V_MATERIAL_BACK_SHININESS] << endl;
    
    for ( i = 0; i < MAX_LIGHTS_ARB; i++ )
    {
        os << "LIGHT[" << i << "].AMBIENT = " << vectorState[BASE_LIGHTS + 7*i] << endl;
        os << "LIGHT[" << i << "].DIFUSSE = " << vectorState[BASE_LIGHTS + 7*i+1] << endl;
        os << "LIGHT[" << i << "].SPECULAR = " << vectorState[BASE_LIGHTS + 7*i+2] << endl;
        os << "LIGHT[" << i << "].POSITION = " << vectorState[BASE_LIGHTS + 7*i+3] << endl;
        os << "LIGHT[" << i << "].ATTENUATION = " << vectorState[BASE_LIGHTS + 7*i+4] << endl;
        os << "LIGHT[" << i << "].SPOT DIRECTION = " << vectorState[BASE_LIGHTS + 7*i+5] << endl;
        os << "LIGHT[" << i << "].HALF = " << vectorState[BASE_LIGHTS + 7*i+6] << endl;
    }
    
    os << "LIGHTMODEL.AMBIENT = " << vectorState[V_LIGHTMODEL_AMBIENT] << endl;
    
    for ( i = 0; i < MAX_LIGHTS_ARB; i++ )
    {
        os << "LIGHTPROD[" << i << "].FRONT.AMBIENT = " << vectorState[BASE_LIGHTPROD + 7*i] << endl;
        os << "LIGHTPROD[" << i << "].FRONT.DIFUSSE = " << vectorState[BASE_LIGHTPROD + 7*i+1] << endl;
        os << "LIGHTPROD[" << i << "].FRONT.SPECULAR = " << vectorState[BASE_LIGHTPROD + 7*i+2] << endl;
        os << "LIGHTPROD[" << i << "].BACK.AMBIENT = " << vectorState[BASE_LIGHTPROD + 7*i+3] << endl;
        os << "LIGHTPROD[" << i << "].BACK.DIFUSSE = " << vectorState[BASE_LIGHTPROD + 7*i+1+4] << endl;
        os << "LIGHTPROD[" << i << "].BACK.SPECULAR = " << vectorState[BASE_LIGHTPROD + 7*i+2+5] << endl;
    }    
    
    os << "----" << endl;
    
    /*
    os << "--- MATRIX STATES ---" << endl;
    os << "MODELVIEW:" << endl;
    for ( i = 0; i < MAX_VERTEX_UNITS_ARB; i++ )
    {
        os << "VERTEX UNIT: " << i << endl;

        printMatrix(os, MatrixId(i*4));
    }

    os << "PROJECTION:" << endl;
    printMatrix(os, M_PROJECTION);

    os << "MVP:" << endl;
    printMatrix(os, M_MVP);

    os << "TEXTURE:" << endl;
    for ( i = 0; i < MAX_TEXTURE_UNITS_ARB; i++ )
    {
        os << "TEXTURE UNIT: " << i << endl;

        printMatrix(os, MatrixId(STATE_MATRIX_TEXTURE_OFFSET + i*4));
    }

    os << "PALETTE: " << endl;
    for ( i = 0; i < MAX_PALETTE_MATRICES_ARB; i++ )
    {
        os << "PALETTE MATRIX: " << i << endl;

        printMatrix(os, MatrixId(STATE_MATRIX_PALETTE_OFFSET + i*4));
    }

    os << "PROGRAM: " << endl;
    for ( i = 0; i < MAX_PROGRAM_MATRICES_ARB; i++ )
    {
        os << "PROGRAM MATRIX: " << i << endl;

        printMatrix(os, MatrixId(STATE_MATRIX_PROGRAM_OFFSET + i*4));
    }
    */
}


void GLState::printMatrix(ostream& os, MatrixId matId_) const
{

    GLuint matId = GLuint(matId_);
    os << "MT_NONE: " << (dirtyMat[matId] ? "NOT UPDATED" : "UPDATED") << "\n";
    matrixState[matId].dump(os);
    os << "MT_INVERSE: " << (dirtyMat[matId+1] ? "NOT UPDATED" : "UPDATED") << "\n";
    matrixState[matId+1].dump(os);
    os << "MT_TRANSPOSE: " << (dirtyMat[matId+2] ? "NOT UPDATED" : "UPDATED") << "\n";
    matrixState[matId+2].dump(os);
    os << "MT_INVTRANS: " << (dirtyMat[matId+3] ? "NOT UPDATED" : "UPDATED") << "\n";
    matrixState[matId+3].dump(os);
    
}

