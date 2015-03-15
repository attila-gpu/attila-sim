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

#ifndef TLSHADERIMP_H
    #define TLSHADERIMP_H

#include "TLShader.h"
#include "Matrixf.h"
#include "AuxFuncsLib.h"
#include "GLState.h"
#include <cmath>

namespace libgl
{

/**
 * Parameter for normal rescaling process in vertex shader
 */
 
class ScaleFactor : public InitLocal
{
    enum { localParam = 40 };

public:

    void init(ProgramObject& po, const GLContext& ctx)
    {
        using namespace glsNS;
        const GLState& gls = ctx.getGLState();
        // compue scale factor
        Matrixf m = gls.getMatrix(M_MODELVIEW, MT_INVERSE);
        float sf = m[2][0]*m[2][0] + m[2][1]*m[2][1] + m[2][2]*m[2][2];
        sf = 1 / afl::sqrt(sf);
        po.getLocalParams()[localParam][0] = sf;
        po.getLocalParams()[localParam][1] = sf;
        po.getLocalParams()[localParam][2] = sf;
        po.getLocalParams()[localParam][3] = sf;
    }

    void dump() const
    {
        std::cout << "local[" << localParam << "] -> Scale Factor" 
            << std::endl;
    }

};

/**
 * Parameter for lighting process in vertex shader
 */
 
class LightPosNormalization : public InitLocal
{
private:

    int light;

public:

    LightPosNormalization(int light) : light(light) {}

    void init(ProgramObject& po, const GLContext& ctx)
    {
        using namespace glsNS;
        const GLState& gls = ctx.getGLState();
        QuadReg<float> pos = gls.getVector(V_LIGHT_POSITION + 7*light);
        afl::normalizeQuad(pos);
        po.getLocalParams()[light] = pos;
    }

    void dump() const
    {
        std::cout << "local[" << light << "] -> Normalized Light_" << light 
            << " position" << std::endl;
    }


};

/**
 * Parameter for linear fog computation
 *
 * Param needed PARAM p = {-1/(END-START), END/(END-START), NOT USED, NOT USED};
 * GLState contents V_FOG_PARAMS = (fog density, linear start, linear end, 1/(end-start)}
 *
 */
 
class LinearFogParams : public InitLocal
{
    enum { localParam = 41 };
    
public:

    LinearFogParams() {}
    
    void init(ProgramObject& po, const GLContext& ctx)
    {
        using namespace glsNS;
        const GLState& gls = ctx.getGLState();
        QuadReg<float> pos = gls.getVector(V_FOG_PARAMS);
        po.getLocalParams()[localParam][0] = - pos[3];
        po.getLocalParams()[localParam][1] = pos[2] * pos[3];
    }
        
    void dump() const
    {
        std::cout << "local[" << localParam << "] -> Linear Fog Params" << std::endl;
    }

};

/**
 * Parameter for exponential fog computation
 *
 * Param needed PARAM p = {DENSITY/LN(2), NOT USED, NOT USED, NOT USED};
 * GLState contents V_FOG_PARAMS = (fog density, linear start, linear end, 1/(end-start)}
 *
 */
 
class ExponentialFogParams : public InitLocal
{
    enum { localParam = 41 };
    
public:

    ExponentialFogParams() {}
    
    void init(ProgramObject& po, const GLContext& ctx)
    {
        using namespace glsNS;
        const GLState& gls = ctx.getGLState();
        QuadReg<float> pos = gls.getVector(V_FOG_PARAMS);
        po.getLocalParams()[localParam][0] = static_cast<f32bit>(pos[0]/std::log(2.0));
    }
        
    void dump() const
    {
        std::cout << "local[" << localParam << "] -> Exponential Fog Params" << std::endl;
    }

};

/**
 * Parameter for second order exponential fog computation
 *
 * Param needed PARAM p = {DENSITY/SQRT(LN(2)), NOT USED, NOT USED, NOT USED};
 * GLState contents V_FOG_PARAMS = (fog density, linear start, linear end, 1/(end-start)}
 *
 */
 
class SecondOrderExponentialFogParams : public InitLocal
{
    enum { localParam = 41 };
    
public:

    SecondOrderExponentialFogParams() {}
    
    void init(ProgramObject& po, const GLContext& ctx)
    {
        using namespace glsNS;
        const GLState& gls = ctx.getGLState();
        QuadReg<float> pos = gls.getVector(V_FOG_PARAMS);
        po.getLocalParams()[localParam][0] = static_cast<f32bit>(pos[0]/std::sqrt(std::log(2.0)));
    }
        
    void dump() const
    {
        std::cout << "local[" << localParam << "] -> Second Order Exponential Fog Params" << std::endl;
    }

};

/**
 * Parameter for alpha test computation. The parameter is the reference value.
 */

class AlphaTestReferenceValue: public InitLocal
{
    enum { localParam = 42 };
public:

    AlphaTestReferenceValue() {}

    void init(ProgramObject& po, const GLContext& ctx)
    {
        GLclampf refValue = ctx.getAlphaTestReferenceValue();
        po.getLocalParams()[localParam][0] = refValue;
        po.getLocalParams()[localParam][1] = refValue;
        po.getLocalParams()[localParam][2] = refValue;
        po.getLocalParams()[localParam][3] = refValue;
    }
        
    void dump() const
    {
        std::cout << "local[" << localParam << "] -> Alpha Test Reference Value Param" << std::endl;
    }
};    
    
 
        
} // namespace libgl

#endif
