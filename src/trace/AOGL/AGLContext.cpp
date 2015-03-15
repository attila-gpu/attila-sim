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

#include "AGLContext.h"
#include "AGLBufferObject.h"
#include "support.h"
#include "MatrixStackImp.h"
#include "ACDSampler.h"
#include "GL2ACD.h"
#include "GL2ACDX.h"
#include <typeinfo>

using namespace agl;
using namespace acdlib;

#define PI 3.14159265
//#define max(a,b) a < b ? b : a
#define TEXTURE_UNITS 16

GLContext::GLContext(ACDDevice* acddev) :
    _acddev(acddev), _vertexBuffer(0), _colorBuffer(0), _normalBuffer(0),
    _numTU(TEXTURE_UNITS),
    _currentStackGroup(0),
    _clearColorR(0),
    _clearColorG(0),
    _clearColorB(0),
    _clearColorA(0),
    _depthClearValue(1.0f),
    _stencilClearValue(0),
    _bufferVertexes(0),
    _vertexStreamMap(1000),
    _colorStreamMap(1000),
    _normalStreamMap(1000),
    _textureManager(TEXTURE_UNITS),
    _textureUnits(TEXTURE_UNITS),
    _texVArray(TEXTURE_UNITS),
    _currentClientTextureUnit(0),
    _fogStart(0.0),
    _fogEnd(1.0),
    _activeBuffer(0),
    _vertexVBO(false),
    _colorVBO(false),
    _normalVBO(false),
    _genericVArray(16),
    _genericDesc(16),
    _genericBuffer(16),
    _genericStreamMap(16),
    _genericAttribArrayFlags(16, false),
	_clientTextureUnits(8,false),
	_emulVertexProgram(0),
	_emulFragmentProgram(0)

{
    for ( acd_uint i = 0; i < TEXTURE_UNITS; ++i )
        _textureBuffer[i] = 0;

    for ( acd_uint i = 0; i < 16; ++i )
        _textureVBO[i] = false;

    for ( acd_uint i = 0; i < 31; ++i )
        _freeStream.insert(i);

    for ( acd_uint i = 0; i < 16; ++i )
        _freeSampler.insert(i);

    for ( acd_uint i = 0; i < TEXTURE_UNITS; ++i )
        _textureStreamMap[i] = 0;

    _fpState = acdlib::ACDXCreateFixedPipelineState(acddev);

    acdlib::ACDXLoadFPDefaultSettings(_fpSettings);

    _vertexDesc.offset = 0;
    _vertexDesc.components = 4;
    _vertexDesc.frequency = 0;
    _vertexDesc.type = ACD_SD_FLOAT32;
    _vertexDesc.stride = 16;

    _colorDesc.offset = 0;
    _colorDesc.components = 4;
    _colorDesc.frequency = 0;
    _colorDesc.type = ACD_SD_FLOAT32;
    _colorDesc.stride = 16;

    _normalDesc.offset = 0;
    _normalDesc.components = 3;
    _normalDesc.frequency = 0;
    _normalDesc.type = ACD_SD_FLOAT32;
    _normalDesc.stride = 12;

    for (int i = 0; i < 16; i++)
    {
        _textureDesc[i].offset = 0;
        _textureDesc[i].components = 4;
        _textureDesc[i].frequency = 0;
        _textureDesc[i].type = ACD_SD_FLOAT32;
        _textureDesc[i].stride = 16;
    }

    _emulVertexProgram = _acddev->createShaderProgram();
    _emulFragmentProgram = _acddev->createShaderProgram();

    _currentColor[0] = 1.0f;
    _currentColor[1] = 1.0f;
    _currentColor[2] = 1.0f;
    _currentColor[3] = 1.0f;

    _currentNormal[0] = 0.0f;
    _currentNormal[1] = 0.0f;
    _currentNormal[2] = 1.0f;

    _currentModelview = 0;
	_activeTextureUnit = 0;

    // Create the projection matrix stack
    _stackGroup[0].push_back(new MatrixStackProjection(&_fpState->tl()));

    // Create the group of stacks for the different modelview matrices
    for ( acd_uint i = 0; i < ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT; ++i )
        _stackGroup[1].push_back( new MatrixStackModelview(&_fpState->tl(), i) );

    // Create the group of stacks for the different texture stages
    for ( acd_uint i = 0; i < ACDX_FP_MAX_TEXTURE_STAGES_LIMIT; ++i )
        _stackGroup[2].push_back(new MatrixStackTextureCoord(&_fpState->txtcoord(), i));

    for ( acd_uint i = 0; i < 16; ++i )
        _textureSamplerMap[i] = -1;

    for(acd_uint i = 0; i < RS_MAXRENDERSTATE; i++)
        _renderStates[i] = false;
}

ACDDevice& GLContext::acd()
{
    return *_acddev;
}

void GLContext::addVertex(acd_float x, acd_float y, acd_float z, acd_float w)
{
    ++_bufferVertexes;

    _vertexBuffer->pushData(&x, sizeof(acd_float));
    _vertexBuffer->pushData(&y, sizeof(acd_float));
    _vertexBuffer->pushData(&z, sizeof(acd_float));
    _vertexBuffer->pushData(&w, sizeof(acd_float));

    _colorBuffer->pushData(_currentColor, 4*sizeof(acd_float));
    _normalBuffer->pushData(_currentNormal, 3*sizeof(acd_float));

    acd_float aux[4];
    aux[0] = _currentTexCoord[0][0];
    aux[1] = _currentTexCoord[0][1];
    aux[2] = _currentTexCoord[0][2];
    aux[3] = _currentTexCoord[0][3];

    _textureBuffer[0]->pushData(aux, 4*sizeof(acd_float));
    _textureBuffer[1]->pushData(aux, 4*sizeof(acd_float));
    _textureBuffer[2]->pushData(aux, 4*sizeof(acd_float));
	_textureBuffer[3]->pushData(aux, 4*sizeof(acd_float));
	_textureBuffer[4]->pushData(aux, 4*sizeof(acd_float));
	_textureBuffer[5]->pushData(aux, 4*sizeof(acd_float));
	_textureBuffer[6]->pushData(aux, 4*sizeof(acd_float));
	_textureBuffer[7]->pushData(aux, 4*sizeof(acd_float));
}

void GLContext::setColor(acd_float red, acd_float green, acd_float blue, acd_float alpha)
{
    _currentColor[0] = red;
    _currentColor[1] = green;
    _currentColor[2] = blue;
    _currentColor[3] = alpha;
}

void GLContext::setNormal(acd_float r, acd_float s, acd_float t)
{
    _currentNormal[0] = r;
    _currentNormal[1] = s;
    _currentNormal[2] = t;
}


acd_uint GLContext::countInternalVertexes() const
{
    return _bufferVertexes;
}

void GLContext::initInternalBuffers(acd_bool createBuffers)
{
    //This function is used only by glBegin

    //Destroy all the buffers that are not used as VBO

    _bufferVertexes = 0;

    if ( _vertexBuffer && !_vertexVBO )
    {
        _acddev->destroy(_vertexBuffer);
        _vertexBuffer = 0;
    }

    if ( _colorBuffer && !_colorVBO )
    {
        _acddev->destroy(_colorBuffer);
        _colorBuffer = 0;
    }

    if ( _normalBuffer && !_normalVBO )
    {
        _acddev->destroy(_normalBuffer);
        _normalBuffer = 0;
    }

    for (int i = 0; i < 16; i++)
        if ( _textureBuffer[i] && !_textureVBO[i])
        {
            _acddev->destroy(_textureBuffer[i]);
            _textureBuffer[i] = 0;
        }

    if (createBuffers) {  // buffers are only created when we are using glBegin/glEnd calls; other calls attach directly the buffer
        _vertexBuffer = _acddev->createBuffer();
        _colorBuffer = _acddev->createBuffer();
        _normalBuffer = _acddev->createBuffer();

        for ( acd_int i = 0; i < 16; i++)
            _textureBuffer[i] = _acddev->createBuffer();

        _vertexVBO = false;
        _colorVBO = false;
        _normalVBO = false;

        for ( acd_int i = 0; i < 16; i++)
            _textureVBO[i] = false;
    }
}

void GLContext::setClearColor(acd_ubyte red, acd_ubyte green, acd_ubyte blue, acd_ubyte alpha)
{
    _clearColorR = red;
    _clearColorG = green;
    _clearColorB = blue;
    _clearColorA = alpha;
}

void GLContext::setClearColor(acd_float red, acd_float green, acd_float blue, acd_float alpha)
{
    _clearColorR = _clamp(red);
    _clearColorG = _clamp(green);
    _clearColorB = _clamp(blue);
    _clearColorA = _clamp(alpha);
}

void GLContext::getClearColor(acd_ubyte& red, acd_ubyte& green, acd_ubyte& blue, acd_ubyte& alpha) const
{
    red = _clearColorR;
    green = _clearColorG;
    blue = _clearColorB;
    alpha = _clearColorA;
}

void GLContext::setDepthClearValue(acd_float depthValue)
{
    _depthClearValue = depthValue;
}

acd_float GLContext::getDepthClearValue() const
{
    return _depthClearValue;
}

void GLContext::setStencilClearValue(acd_int stencilValue)
{
    _stencilClearValue = stencilValue;
}

acd_int GLContext::getStencilClearValue() const
{
    return _stencilClearValue;
}

acd_ubyte GLContext::_clamp(acd_float v)
{
    if ( v > 1.0f ) v = 1.0f;
    else if ( v < 0.0f ) v = 0.0f;
    return static_cast<acd_ubyte>(v * 255);
}


void GLContext::setMatrixMode(GLenum mode)
{
    if ( mode  == GL_PROJECTION )
        _currentStackGroup = 0;
    else if ( mode == GL_MODELVIEW )
        _currentStackGroup = 1;
    else if ( mode == GL_TEXTURE )
        _currentStackGroup = 2;
    else
        panic("GLContext", "setMatrixMode", "Unexpected setMatrixMode token");
}

void GLContext::setActiveTextureUnit(acd_uint textureStage)
{
    if ( textureStage >= ACDX_FP_MAX_TEXTURE_STAGES_LIMIT )
        panic("GLContext", "setActiveTextureUnit", "Texture stage unit not available");

    _activeTextureUnit = textureStage;
}

void GLContext::setActiveModelview(acd_uint modelviewStack)
{
    if ( modelviewStack >= ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT )
        panic("GLContext", "setActiveModelview", "Modelview matrix stack not available");

    _currentModelview = modelviewStack;
}



MatrixStack& GLContext::matrixStack()
{
    switch ( _currentStackGroup ) {
        case 1:
            return *_stackGroup[1][_currentModelview];
        case 2:
            return *_stackGroup[2][_activeTextureUnit];
        default: // case 0: // Projection matrix
            return *_stackGroup[0][0];
    }
}

agl::MatrixStack& GLContext::matrixStack(GLenum mode)
{
    switch ( mode ) {
        case GL_PROJECTION:
            return *_stackGroup[0][0];
        case GL_MODELVIEW:
            return *_stackGroup[1][_currentModelview];
        case GL_TEXTURE:
            return *_stackGroup[2][_activeTextureUnit];
        default:
            panic("GLContext", "matrixStack(mode)", "Unknown/unsuported matrix mode selected");
    }

    // Dummy return to avoid stupid warnings (panic never returns)
    return *_stackGroup[0][0];
}

agl::MatrixStack& GLContext::matrixStack(GLenum mode, acd_uint unit)
{
    acd_uint selector;
    switch ( mode ) {
        case GL_PROJECTION:
            selector = 0;
        case GL_MODELVIEW:
            selector = 1;
        case GL_TEXTURE:
            selector = 2;
        default:
            selector = 0; // to avoid dummy warnings (panic never returns)
            panic("GLContext", "matrixStack(mode,unit)", "Unknown/unsuported matrix mode selected");
    }

    if ( _stackGroup[selector].size() >= unit )
        panic("GLContext", "matrixStack(mode,unit)", "Matrix unit not available");

    return *_stackGroup[selector][unit];
}


acdlib::ACDXFixedPipelineState& GLContext::fixedPipelineState()
{
    return *_fpState;
}

acdlib::ACDX_FIXED_PIPELINE_SETTINGS& GLContext::fixedPipelineSettings()
{
    return _fpSettings;
}

void GLContext::attachInternalBuffers()
{
    acd_uint err = 0;

    if (_vertexBuffer) {
        _vertexStreamMap = obtainStream();
        _acddev->stream(_vertexStreamMap).set(_vertexBuffer, _vertexDesc);
        _acddev->enableVertexAttribute(ACDX_VAM_VERTEX, _vertexStreamMap);
    }

    if (_colorBuffer) {
        _colorStreamMap = obtainStream();
        _acddev->stream(_colorStreamMap).set(_colorBuffer, _colorDesc);
        _acddev->enableVertexAttribute(ACDX_VAM_COLOR, _colorStreamMap);
    }

    if (_normalBuffer) {
        _normalStreamMap = obtainStream();
        _acddev->stream(_normalStreamMap).set(_normalBuffer, _normalDesc);
        _acddev->enableVertexAttribute(ACDX_VAM_NORMAL, _normalStreamMap);
    }

    for (int i=0; i<8; i++)
    {
        // If there if a _textureBuffer set and also a texture attach to that textureUnit sync the stream
        if (_textureBuffer[i] && _textureUnits[i].isTextureUnitActive() && !_genericAttribArrayFlags[i+8] ) {
            _textureStreamMap[i] = obtainStream();
            _acddev->stream(_textureStreamMap[i]).set(_textureBuffer[i], _textureDesc[i]);
            _acddev->enableVertexAttribute(ACDX_VAM_TEXTURE_0+i, _textureStreamMap[i]);
        }
    }

    //if ( _isGenericVArray() )
    //{
        for ( GLuint i = 0; i < _genericAttribArrayFlags.size(); i++ )
        {
            if ( _genericAttribArrayFlags[i] && _genericBuffer[i]!=0 )
            {
                _genericStreamMap[i] = obtainStream();
                _acddev->stream(_genericStreamMap[i]).set(_genericBuffer[i], _genericDesc[i]);
                _acddev->enableVertexAttribute( (ACDX_VERTEX_ATTRIBUTE_MAP)i, _genericStreamMap[i]);
            }
        }
    //}

}

void GLContext::attachInternalSamplers()
{
    for(int tu = 0; tu < TEXTURE_UNITS; tu++)
    {

        if (_currentFragmentProgram->getTextureUnitsUsage(tu))
        {
            GLTextureObject* to = _textureUnits[tu].getTextureObject(_currentFragmentProgram->getTextureUnitsUsage(tu));

            _textureSamplerMap[tu] = obtainSampler();

            if (_currentFragmentProgram->getTextureUnitsUsage(tu) == GL_TEXTURE_RECTANGLE)
                _acddev->sampler(_textureSamplerMap[tu]).setNonNormalizedCoordinates(true);
            else
                _acddev->sampler(_textureSamplerMap[tu]).setNonNormalizedCoordinates(false);

            _acddev->sampler(_textureSamplerMap[tu]).setEnabled(true);
            _acddev->sampler(_textureSamplerMap[tu]).setMinLOD(to->getMinLOD());
            _acddev->sampler(_textureSamplerMap[tu]).setMaxLOD(to->getMaxLOD());
            _acddev->sampler(_textureSamplerMap[tu]).setLODBias(to->getLodBias());
            _acddev->sampler(_textureSamplerMap[tu]).setUnitLODBias(_textureUnits[tu].getLodBias());
            _acddev->sampler(_textureSamplerMap[tu]).setMinFilter(agl::getACDTextureFilter(to->getMinFilter()));
            _acddev->sampler(_textureSamplerMap[tu]).setMagFilter(agl::getACDTextureFilter(to->getMagFilter()));
            _acddev->sampler(_textureSamplerMap[tu]).setMaxAnisotropy((acdlib::acd_uint)to->getMaxAnisotropy());
            _acddev->sampler(_textureSamplerMap[tu]).setTexture(to->getTexture());
            _acddev->sampler(_textureSamplerMap[tu]).setTextureAddressMode(ACD_TEXTURE_R_COORD, agl::getACDTextureAddrMode(to->getWrapR()));
            _acddev->sampler(_textureSamplerMap[tu]).setTextureAddressMode(ACD_TEXTURE_S_COORD, agl::getACDTextureAddrMode(to->getWrapS()));
            _acddev->sampler(_textureSamplerMap[tu]).setTextureAddressMode(ACD_TEXTURE_T_COORD, agl::getACDTextureAddrMode(to->getWrapT()));
        }
    }
}

void GLContext::_setSamplerConfiguration()
{
    for(int tu = 0; tu < TEXTURE_UNITS; tu++)
    {
        if (_textureUnits[tu].isTextureUnitActive())
        {
            GLTextureObject* to = _textureUnits[tu].getTextureObject(_textureUnits[tu].isTextureUnitActive());

            if (to == 0)
                panic("GLContext","attachInternalSamplers","TextureUnit being used inside the shader but no texture setted to that texture unit");

            fixedPipelineSettings().textureStages[tu].baseInternalFormat = agl::getACDXTextureInternalFormat(_textureUnits[tu].getTextureObject(_textureUnits[tu].isTextureUnitActive())->getBaseInternalFormat());
            fixedPipelineSettings().textureStages[tu].textureStageFunction = agl::getACDXTextureStageFunction(_textureUnits[tu].getTexMode());

            if ( fixedPipelineSettings().textureStages[tu].textureStageFunction == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE || 
                 fixedPipelineSettings().textureStages[tu].textureStageFunction == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE_NV )
            {
                fixedPipelineSettings().textureStages[tu].combineSettings.RGBFunction = agl::getACDXCombinerFunction(_textureUnits[tu].getRGBCombinerFunction());
                fixedPipelineSettings().textureStages[tu].combineSettings.ALPHAFunction = agl::getACDXCombinerFunction(_textureUnits[tu].getAlphaCombinerFunction());
                fixedPipelineSettings().textureStages[tu].combineSettings.RGBScale = _textureUnits[tu].getRGBScale();
                fixedPipelineSettings().textureStages[tu].combineSettings.ALPHAScale = _textureUnits[tu].getAlphaScale();

            }

            for ( acd_uint j = 0; j < 4; j++ )
            {
                // RGB
                GLenum src = _textureUnits[tu].getSrcRGB(j);

                if ( src == GL_TEXTURE )
                {
                     fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE;
                }
                else if ( GL_TEXTURE0 <= src && src < TEXTURE_UNITS + GL_TEXTURE0 )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn;
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB_texCrossbarReference[j] = src - GL_TEXTURE0;
                }
                else if ( src == GL_CONSTANT )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT;
                }
                else if ( src == GL_PRIMARY_COLOR )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR;
                }
                else if ( src == GL_PREVIOUS )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS;
                }
                else if ( src == GL_ZERO )
                {
                    // For "NV_texture_env_combine4" extension support
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO;
                }
                else if ( src == GL_ONE )
                {
                    // For "ATI_texture_env_combine3" extension support
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE;
                }
                else
                    panic("GLContext", "attachInternalSamplers", "Unexpected RGB Source or GL_TEXTUREXXX out of bounds");



                // Alpha
                src = _textureUnits[tu].getSrcAlpha(j);

                if ( src == GL_TEXTURE )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcALPHA[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE;
                }
                else if ( GL_TEXTURE0 <= src && src < TEXTURE_UNITS + GL_TEXTURE0 )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcALPHA[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn;
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcALPHA_texCrossbarReference[j] = src - GL_TEXTURE0;
                }
                else if ( src == GL_CONSTANT )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT;
                }
                else if ( src == GL_PRIMARY_COLOR )
                {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR;
                }
                 else if ( src == GL_PREVIOUS )
                 {
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS;
                }
                else if ( src == GL_ZERO )
                {
                    /* For "NV_texture_env_combine4" extension support */
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO;
                }
                else if ( src == GL_ONE )
                {
                    /* For "ATI_texture_env_combine3" extension support */
                    fixedPipelineSettings().textureStages[tu].combineSettings.srcRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE;
                }
                else
                    panic("GLContext", "attachInternalSamplers", "Unexpected Alpha Source or GL_TEXTUREXXX out of bounds");


                switch ( _textureUnits[tu].getOperandRGB(j) )
                {
                    case GL_SRC_COLOR:
                        fixedPipelineSettings().textureStages[tu].combineSettings.operandRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR;
                        break;
                    case GL_ONE_MINUS_SRC_COLOR:
                        fixedPipelineSettings().textureStages[tu].combineSettings.operandRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR;
                        break;
                    case GL_SRC_ALPHA:
                        fixedPipelineSettings().textureStages[tu].combineSettings.operandRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA;
                        break;
                    case GL_ONE_MINUS_SRC_ALPHA:
                        fixedPipelineSettings().textureStages[tu].combineSettings.operandRGB[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA;
                        break;
                    default:
                        panic("GLContext", "attachInternalSamplers", "Unexpected operand RGB");
                }
                switch ( _textureUnits[tu].getOperandAlpha(j) )
                {
                    case GL_SRC_ALPHA:
                        fixedPipelineSettings().textureStages[tu].combineSettings.operandALPHA[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA;
                        break;
                    case GL_ONE_MINUS_SRC_ALPHA:
                        fixedPipelineSettings().textureStages[tu].combineSettings.operandALPHA[j] = ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA;
                        break;
                    default:
                        panic("GLContext", "attachInternalSamplers", "Unexpected operand Alpha");
                }
            }

            if (_textureUnits[tu].isEnabledTexGen(GL_Q))
                fixedPipelineSettings().textureCoordinates[tu].coordQ = agl::getACDXTexGenMode(_textureUnits[tu].getTexGenMode(GL_Q));
            else
                fixedPipelineSettings().textureCoordinates[tu].coordQ = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;


            if (_textureUnits[tu].isEnabledTexGen(GL_R))
                fixedPipelineSettings().textureCoordinates[tu].coordR = agl::getACDXTexGenMode(_textureUnits[tu].getTexGenMode(GL_R));
            else
                fixedPipelineSettings().textureCoordinates[tu].coordR = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;


            if (_textureUnits[tu].isEnabledTexGen(GL_S))
                fixedPipelineSettings().textureCoordinates[tu].coordS = agl::getACDXTexGenMode(_textureUnits[tu].getTexGenMode(GL_S));
            else
                fixedPipelineSettings().textureCoordinates[tu].coordS = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;


            if (_textureUnits[tu].isEnabledTexGen(GL_T))
                fixedPipelineSettings().textureCoordinates[tu].coordT = agl::getACDXTexGenMode(_textureUnits[tu].getTexGenMode(GL_T));
            else
                fixedPipelineSettings().textureCoordinates[tu].coordT = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB;

        }
    }
}

void GLContext::deattachInternalSamplers()
{

    for(int i = 0; i < TEXTURE_UNITS; i++)
    {
        if (_textureSamplerMap[i] != -1)
        {
            _acddev->sampler(_textureSamplerMap[i]).setEnabled(false);
            releaseSampler(_textureSamplerMap[i]);
        }
    }

}

void GLContext::deattachInternalBuffers()
{
    if (_vertexBuffer)
    {
        _vertexBuffer = 0;
        _acddev->disableVertexAttribute(ACDX_VAM_VERTEX);
        releaseStream(_vertexStreamMap);
    }

    if (_colorBuffer)
    {
        _colorBuffer = 0;
        _acddev->disableVertexAttribute(ACDX_VAM_COLOR);
        releaseStream(_colorStreamMap);
    }

    if (_normalBuffer)
    {
        _normalBuffer = 0;
        _acddev->disableVertexAttribute(ACDX_VAM_NORMAL);
        releaseStream(_normalStreamMap);
    }

    for (int i=0; i<8; i++)
    {
        if (_textureBuffer[i]!=0 && !_genericAttribArrayFlags[i+8]) {
            _textureBuffer[i] = 0;
            _acddev->disableVertexAttribute(ACDX_VAM_TEXTURE_0+i);
            releaseStream(_textureStreamMap[i]);
        }
    }

    //if ( _isGenericVArray() )
    //{
        for ( GLuint i = 0; i < _genericAttribArrayFlags.size(); i++ )
        {
            if ( _genericAttribArrayFlags[i] )
            {
                _genericBuffer[i] = 0;
                _acddev->disableVertexAttribute((ACDX_VERTEX_ATTRIBUTE_MAP)i);
                releaseStream(_genericStreamMap[i]);
            }
        }
    //}
}


void GLContext::setShaders()
{
    acdlib::ACDShaderProgram* vsh;
    acdlib::ACDShaderProgram* fsh;

    // If vertex shader or fragment shader must be generated, call generate programs

    _setSamplerConfiguration();

    if ( !_renderStates[RS_VERTEX_PROGRAM] && !_renderStates[RS_FRAGMENT_PROGRAM] )
    {
		_emulVertexProgram = _acddev->createShaderProgram();
		_emulFragmentProgram = _acddev->createShaderProgram();

        ACDXGeneratePrograms(_fpState, _fpSettings, _emulVertexProgram, _emulFragmentProgram);

        vsh = _emulVertexProgram;
        fsh = _emulFragmentProgram;

    }
    else if (!_renderStates[RS_VERTEX_PROGRAM])
    {
        _emulVertexProgram = _acddev->createShaderProgram();

        ACDXGenerateVertexProgram(_fpState, _fpSettings, _emulVertexProgram);
        vsh = _emulVertexProgram;
    }
    else if (!_renderStates[RS_FRAGMENT_PROGRAM])
    {
		_emulFragmentProgram = _acddev->createShaderProgram();

        ACDXGenerateFragmentProgram(_fpState, _fpSettings, _emulFragmentProgram);
        fsh = _emulFragmentProgram;
    }

    if ( _renderStates[RS_VERTEX_PROGRAM] )
	{
        vsh = _arbProgramManager.target(GL_VERTEX_PROGRAM_ARB).getCurrent().compileAndResolve(_fpState);
		_emulVertexProgram = vsh;
	}

    if ( _renderStates[RS_FRAGMENT_PROGRAM] )
	{
        fsh = _arbProgramManager.target(GL_FRAGMENT_PROGRAM_ARB).getCurrent().compileAndResolve(_fpState);
		_emulFragmentProgram = fsh;
	}

    _acddev->setVertexShader(vsh);
    _acddev->setFragmentShader(fsh);
    _currentFragmentProgram = fsh;
    _acddev->setVertexDefaultValue(_currentColor);
}

void GLContext::setRenderState(AGL_RENDERSTATE state, acd_bool enabled)
{
    if ( state >= RS_MAXRENDERSTATE )
        panic("GLContext", "setRenderState", "Render state unknown");

    _renderStates[state] = enabled;
}

acd_bool GLContext::getRenderState(AGL_RENDERSTATE state) const
{
    if ( state >= RS_MAXRENDERSTATE )
        panic("GLContext", "getRenderState", "Render state unknown");
    return _renderStates[state];
}

ARBProgramManager& GLContext::arbProgramManager()
{
    return _arbProgramManager;
}

BufferManager& GLContext::bufferManager()
{
    return _bufferManager;
}

GLTextureManager& GLContext::textureManager()
{
    return _textureManager;
}

acd_uint GLContext::obtainStream()
{
    if ( _freeStream.empty() )
        panic("GLContext", "obtainStream", "No free streams available");

    acd_uint freeStream = *(_freeStream.begin()); // Get the free stream ID
    _freeStream.erase(_freeStream.begin()); // Remove it from the free list

    return freeStream;
}

void GLContext::releaseStream(acd_uint streamID)
{
    if ( streamID >= 31 )
        panic("GLContext", "releaseStream", "Max. stream ID is 30");

    _freeStream.insert(streamID);
}

acd_uint GLContext::obtainSampler()
{
    if ( _freeStream.empty() )
        panic("GLContext", "obtainSampler", "No free samplers available");

    acd_uint freeSampler = *(_freeSampler.begin()); // Get the free stream ID
    _freeSampler.erase(_freeSampler.begin()); // Remove it from the free list

    return freeSampler;
}

void GLContext::releaseSampler(acd_uint samplerID)
{
    if ( samplerID >= 31 )
        panic("GLContext", "releaseSampler", "Max. sampler ID is 30");

    _freeSampler.insert(samplerID);
}

void GLContext::setLightParam(GLenum light, GLenum pname, const GLfloat* params)
{
    acd_uint lightID = light - GL_LIGHT0;
    ACDXLight& lo = fixedPipelineState().tl().light(lightID);
    switch ( pname ) {
        case GL_AMBIENT:
            lo.setLightAmbientColor(params[0], params[1], params[2], params[3]);
            break;
        case GL_DIFFUSE:
            lo.setLightDiffuseColor(params[0], params[1], params[2], params[3]);
            break;
        case GL_SPECULAR:
            lo.setLightSpecularColor(params[0], params[1], params[2], params[3]);
            break;
        case GL_POSITION:
            if ( params[3] == 0 ) { // OpenGL directional light
                acdlib::ACDVector<acdlib::acd_float,4> vect;
                vect[0] = params[0];
                vect[1] = params[1];
                vect[2] = params[2];
                vect[3] = params[3];
                acdlib::_mult_mat_vect(vect,_stackGroup[1][_currentModelview]->get().transpose());
                lo.setLightDirection(vect[0], vect[1], vect[2]);
                fixedPipelineSettings().lights[lightID].lightType = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_DIRECTIONAL;
            }
            else
            { // OpenGL positional (point) light
                acdlib::ACDVector<acdlib::acd_float,4> vect;
                vect[0] = params[0];
                vect[1] = params[1];
                vect[2] = params[2];
                vect[3] = params[3];
                acdlib::_mult_mat_vect(vect,_stackGroup[1][_currentModelview]->get().transpose());
                lo.setLightPosition(vect[0], vect[1], vect[2], vect[3]);
                fixedPipelineSettings().lights[lightID].lightType = ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_POINT;
            }
            break;
        case GL_SPOT_DIRECTION:
            lo.setSpotLightDirection(params[0], params[1], params[2]);
            break;
        case GL_CONSTANT_ATTENUATION:
            lo.setAttenuationCoeffs(params[0], lo.getLinearAttenuation(), lo.getQuadraticAttenuation());
            break;
        case GL_LINEAR_ATTENUATION:
            lo.setAttenuationCoeffs(lo.getConstantAttenuation(), params[0], lo.getQuadraticAttenuation());
            break;
        case GL_QUADRATIC_ATTENUATION:
            lo.setAttenuationCoeffs(lo.getConstantAttenuation(), lo.getLinearAttenuation(), params[0]);
            break;
        case GL_SPOT_EXPONENT:
            lo.setSpotExponent(params[0]);
            break;
        case GL_SPOT_CUTOFF:
            lo.setSpotLightCutOffAngle(cos(params[0]*PI/180));
            fixedPipelineSettings().lights[lightID].lightType =  ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_SPOT;
            break;
        default:
            panic("GLContext", "setLightParam", "Unknown PNAME");
    }
}

void GLContext::setLightModel(GLenum pname, const GLfloat* params)
{
    switch ( pname ) {
        case GL_LIGHT_MODEL_AMBIENT:
            fixedPipelineState().tl().setLightModelAmbientColor(params[0], params[1], params[2], params[3]);
            break;
        case GL_LIGHT_MODEL_LOCAL_VIEWER:
            fixedPipelineSettings().localViewer = ( params[0] != 0 ? true : false );
            break;
        case GL_LIGHT_MODEL_TWO_SIDE:
            fixedPipelineSettings().twoSidedLighting = ( params[0] != 0 ? true : false );
            break;
        case GL_LIGHT_MODEL_COLOR_CONTROL:
            fixedPipelineSettings().separateSpecular = true;/*( params[0] == GL_SINGLE_COLOR ? true : false );*/
            break;
        default:
            panic("GLContext", "setLightModel", "Unknown PNAME");
    }
}

void GLContext::setMaterialParam(GLenum face, GLenum pname, const GLfloat* p)
{
    ACDX_FACE f = ACDX_FRONT;
    switch ( face ) {
        case GL_FRONT: f = ACDX_FRONT; break;
        case GL_BACK: f = ACDX_BACK; break;
        case GL_FRONT_AND_BACK: f = ACDX_FRONT_AND_BACK; break;
        default:
            panic("GLContext", "setMaterialParam", "Unknown FACE type");
    }

    ACDXMaterial& material = fixedPipelineState().tl().material(ACDX_FRONT);

    if (f != ACDX_FRONT_AND_BACK)
        material = fixedPipelineState().tl().material(f);
    else
        material = fixedPipelineState().tl().material(ACDX_FRONT);


    switch ( pname ) {
        case GL_AMBIENT:
            material.setMaterialAmbientColor(p[0], p[1], p[2], p[3]);
            break;
        case GL_DIFFUSE:
            material.setMaterialDiffuseColor(p[0], p[1], p[2], p[3]);
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            material.setMaterialAmbientColor(p[0], p[1], p[2], p[3]);
            material.setMaterialDiffuseColor(p[0], p[1], p[2], p[3]);
            break;
        case GL_SPECULAR:
            material.setMaterialSpecularColor(p[0], p[1], p[2], p[3]);
            break;
        case GL_SHININESS:
            material.setMaterialShininess(p[0]);
            break;
        case GL_EMISSION:
            material.setMaterialEmissionColor(p[0], p[1], p[2], p[3]);
            break;
        case GL_COLOR_INDEXES:
            panic("GLContext", "setMaterialParam", "GL_COLOR_INDEXES not supported");
            break;
    }

    if (f == ACDX_FRONT_AND_BACK)
    {
        material = fixedPipelineState().tl().material(ACDX_BACK);
        switch ( pname ) {
            case GL_AMBIENT:
                material.setMaterialAmbientColor(p[0], p[1], p[2], p[3]);
                break;
            case GL_DIFFUSE:
                material.setMaterialDiffuseColor(p[0], p[1], p[2], p[3]);
                break;
            case GL_AMBIENT_AND_DIFFUSE:
                material.setMaterialAmbientColor(p[0], p[1], p[2], p[3]);
                material.setMaterialDiffuseColor(p[0], p[1], p[2], p[3]);
                break;
            case GL_SPECULAR:
                material.setMaterialSpecularColor(p[0], p[1], p[2], p[3]);
                break;
            case GL_SHININESS:
                material.setMaterialShininess(p[0]);
                break;
            case GL_EMISSION:
                material.setMaterialEmissionColor(p[0], p[1], p[2], p[3]);
                break;
            case GL_COLOR_INDEXES:
                panic("GLContext", "setMaterialParam", "GL_COLOR_INDEXES not supported");
                break;
        }
    }
}

void GLContext::setDescriptor ( AGL_ARRAY descr, GLint offset, GLint components, ACD_STREAM_DATA type, GLint stride )
{
    switch (descr)
    {
        case VERTEX_ARRAY:
            _vertexDesc.offset = offset;
            _vertexDesc.components = components;
            _vertexDesc.frequency = 0;
            _vertexDesc.type = type;
            _vertexDesc.stride = stride;
            break;

        case COLOR_ARRAY:
            _colorDesc.offset = offset;
            _colorDesc.components = components;
            _colorDesc.frequency = 0;
            _colorDesc.type = type;
            _colorDesc.stride = stride;
            break;

        case NORMAL_ARRAY:
            _normalDesc.offset = offset;
            _normalDesc.components = components;
            _normalDesc.frequency = 0;
            _normalDesc.type = type;
            _normalDesc.stride = stride;
            break;

        case TEX_COORD_ARRAY_0:
        case TEX_COORD_ARRAY_1:
        case TEX_COORD_ARRAY_2:
        case TEX_COORD_ARRAY_3:
        case TEX_COORD_ARRAY_4:
        case TEX_COORD_ARRAY_5:
        case TEX_COORD_ARRAY_6:
        case TEX_COORD_ARRAY_7:
            _textureDesc[descr - TEX_COORD_ARRAY_0].offset = offset;
            _textureDesc[descr - TEX_COORD_ARRAY_0].components = components;
            _textureDesc[descr - TEX_COORD_ARRAY_0].frequency = 0;
            _textureDesc[descr - TEX_COORD_ARRAY_0].type = type;
            _textureDesc[descr - TEX_COORD_ARRAY_0].stride = stride;
            break;

        case GENERIC_ARRAY_0:
        case GENERIC_ARRAY_1:
        case GENERIC_ARRAY_2:
        case GENERIC_ARRAY_3:
        case GENERIC_ARRAY_4:
        case GENERIC_ARRAY_5:
        case GENERIC_ARRAY_6:
        case GENERIC_ARRAY_7:
        case GENERIC_ARRAY_8:
        case GENERIC_ARRAY_9:
        case GENERIC_ARRAY_10:
        case GENERIC_ARRAY_11:
        case GENERIC_ARRAY_12:
        case GENERIC_ARRAY_13:
        case GENERIC_ARRAY_14:
        case GENERIC_ARRAY_15:

            _genericDesc[descr - GENERIC_ARRAY_0].offset = offset;
            _genericDesc[descr - GENERIC_ARRAY_0].components = components;
            _genericDesc[descr - GENERIC_ARRAY_0].frequency = 0;
            _genericDesc[descr - GENERIC_ARRAY_0].type = type;
            _genericDesc[descr - GENERIC_ARRAY_0].stride = stride;
            break;

        default:
            panic("GLContext","setDescriptor","Array type not found");
    }
}

void GLContext::setVertexBuffer (ACDBuffer* ptr)
{
    _vertexBuffer = ptr;
    _vertexVBO = true;
}

void GLContext::setNormalBuffer (ACDBuffer *ptr)
{
    _normalBuffer = ptr;
    _normalVBO = true;
}

void GLContext::setColorBuffer (ACDBuffer *ptr)
{
    _colorBuffer = ptr;
    _colorVBO = true;
}

void GLContext::setTexBuffer (ACDBuffer *ptr)
{
    _textureBuffer[0] = ptr;
    _textureVBO[0] = true;
}

void GLContext::enableBufferArray (GLenum cap)
{

    /*GL_VERTEX_ARRAY: bit 0
    GL_COLOR_ARRAY: bit 1
    GL_NORMAL_ARRAY: bit 2
    GL_INDEX_ARRAY: bit 3
    GL_TEXTURE_COORD_ARRAY: bit 3
    GL_EDGE_FLAG_ARRAY: bit 4*/

    switch (cap) {
        case GL_VERTEX_ARRAY:
            _activeBuffer = _activeBuffer | 0x01;
            break;
        case GL_COLOR_ARRAY:
            _activeBuffer = _activeBuffer | 0x02;
            break;
        case GL_NORMAL_ARRAY:
            _activeBuffer = _activeBuffer | 0x04;
            break;
        case GL_INDEX_ARRAY:
            _activeBuffer = _activeBuffer | 0x08;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            _activeBuffer = _activeBuffer | 0x10;
			setEnabledCurrentClientTextureUnit(true);
            break;
        case GL_EDGE_FLAG_ARRAY:
            _activeBuffer = _activeBuffer | 0x20;
            break;

        default:
            panic("GLContext","enableBufferArray","Buffer array type not found");
            break;

    }
}

void GLContext::disableBufferArray (GLenum cap)
{

    /*GL_VERTEX_ARRAY: bit 0
    GL_COLOR_ARRAY: bit 1
    GL_NORMAL_ARRAY: bit 2
    GL_INDEX_ARRAY: bit 3
    GL_TEXTURE_COORD_ARRAY: bit 3
    GL_EDGE_FLAG_ARRAY: bit 4*/

    switch (cap) {
        case GL_VERTEX_ARRAY:
            _activeBuffer = _activeBuffer & ~(0x01);
            break;
        case GL_COLOR_ARRAY:
            _activeBuffer = _activeBuffer & ~(0x02);
            break;
        case GL_NORMAL_ARRAY:
            _activeBuffer = _activeBuffer & ~(0x04);
            break;
        case GL_INDEX_ARRAY:
            _activeBuffer = _activeBuffer & ~(0x08);
            break;
        case GL_TEXTURE_COORD_ARRAY:
            _activeBuffer = _activeBuffer & ~(0x10);
			setEnabledCurrentClientTextureUnit(false);
            break;
        case GL_EDGE_FLAG_ARRAY:
            _activeBuffer = _activeBuffer & ~(0x20);
            break;

        default:
            panic("GLContext","disableBufferArray","Buffer array type not found");
            break;

    }
}


void GLContext::getIndicesSet(std::set<GLuint>& iSet, GLenum type, const GLvoid* indices, GLsizei count)
{
    GLubyte* ub_ptr = (GLubyte*)indices;
    GLushort* us_ptr = (GLushort*)indices;
    GLuint* ui_ptr = (GLuint*)indices;

    int i = 0;

    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
            for ( ; i < count; i++ )
                iSet.insert(ub_ptr[i]);
            break;
        case GL_UNSIGNED_SHORT:
            for ( ; i < count; i++ )
                iSet.insert(us_ptr[i]);
            break;
        case GL_UNSIGNED_INT:
            for ( ; i < count; i++ )
                iSet.insert(ui_ptr[i]);
            break;
        default:
            panic("GLContext", "getIndicesSet", "Unsupported type");
    }
}

acdlib::ACDBuffer* GLContext::createIndexBuffer (const GLvoid* indices, GLenum indicesType, GLsizei count, const std::set<GLuint>& usedIndices)
{
    GLint i = 0;
    map<GLuint,GLuint> conversion;

    set<GLuint>::const_iterator itSet = usedIndices.begin();
    for ( ; itSet != usedIndices.end(); itSet++, i++ )
        conversion[*itSet] = i; // create a conversion table

    GLsizei dataSize = getSize(indicesType) * count;

    GLubyte* ub_ptr = new GLubyte[dataSize];
    GLushort* us_ptr = (GLushort*)ub_ptr;
    GLuint* ui_ptr = (GLuint*)ub_ptr;

    i = 0;

    switch ( indicesType )
    {
        case GL_UNSIGNED_BYTE:
            for ( ; i < count; i++ )
                ub_ptr[i] = (GLubyte)conversion[((GLubyte*)indices)[i]];
            break;
        case GL_UNSIGNED_SHORT:
            for ( ; i < count; i++ )
                us_ptr[i] = (GLushort)conversion[((GLushort*)indices)[i]];
            break;
        case GL_UNSIGNED_INT:
            for ( ; i < count; i++ )
                ui_ptr[i] = (GLuint)conversion[((GLuint*)indices)[i]];
            break;
        default:
            panic("GLContext", "createIndexBuffer", "Unsupported indices type");
    }

    return _acddev->createBuffer(dataSize, (const acdlib::acd_ubyte*)ub_ptr);
}

GLuint GLContext::getSize(GLenum openGLType)
{
    switch ( openGLType )
    {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_INT_8_8_8_8:
            return sizeof(GLubyte);
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        case GL_2_BYTES:
            return 2*sizeof(GLubyte);
        case GL_3_BYTES:
            return 3*sizeof(GLubyte);
        case GL_4_BYTES:
            return 4*sizeof(GLubyte);
        default:
            panic("GLContext", "getSize", "Unknown openGL type size");
            return 0; /* avoid stupid warnings */
    }
}

void GLContext::setVertexArraysBuffers(GLuint start, GLuint count, std::set<GLuint>& iSet)
{
    ACDBuffer *aux;
    acd_uint auxStride;

    if ( _activeBuffer & 0x01) // Vertex Array Modified
    {
        if (!_posVArray.inicialized)
            panic ("GLContext","setVertexArraysBuffers", "Position VertexArray struct is not inicialized");

        aux = configureStream(&_posVArray, false, start, count, iSet);

        if (_posVArray.stride == 0) auxStride = _posVArray.size * getSize(_posVArray.type);
        else auxStride = _posVArray.stride;

        if(_posVArray.bufferID != 0)
            setDescriptor(VERTEX_ARRAY, u64bit(_posVArray.userBuffer), _posVArray.size, agl::getACDStreamType(_posVArray.type, _posVArray.normalized), auxStride);
        else
            setDescriptor(VERTEX_ARRAY, 0, _posVArray.size, agl::getACDStreamType(_posVArray.type, _posVArray.normalized), auxStride);

        setVertexBuffer(aux);
    }

    if ( _activeBuffer & 0x02) // Color Array Modified
    {

        if (!_colVArray.inicialized)
            panic ("GLContext","setVertexArraysBuffers", "Color VertexArray struct is not inicialized");

        aux = configureStream(&_colVArray, true, start, count, iSet);

        if (_posVArray.stride == 0) auxStride = _posVArray.size * getSize(_colVArray.type);
        else auxStride = _posVArray.stride;

        if(_colVArray.bufferID != 0)
            setDescriptor(COLOR_ARRAY,u64bit(_colVArray.userBuffer),_colVArray.size, agl::getACDStreamType(_colVArray.type, _colVArray.normalized),auxStride);
        else
            setDescriptor(COLOR_ARRAY,0,_colVArray.size, agl::getACDStreamType(_colVArray.type, _colVArray.normalized),auxStride);
        setColorBuffer(aux);
    }

    if ( _activeBuffer & 0x04) // Normal Array Modified
    {

        if (!_norVArray.inicialized)
            panic ("GLContext","setVertexArraysBuffers", "Normal VertexArray struct is not inicialized");

        aux = configureStream(&_norVArray, false, start, count, iSet);

		if (_norVArray.stride == 0) auxStride = _norVArray.size * getSize(_norVArray.type);
        else auxStride = _norVArray.stride;

        if(_norVArray.bufferID != 0)
            setDescriptor(NORMAL_ARRAY,u64bit(_norVArray.userBuffer),_norVArray.size, agl::getACDStreamType(_norVArray.type, _norVArray.normalized),auxStride);
        else
            setDescriptor(NORMAL_ARRAY,0,_norVArray.size, agl::getACDStreamType(_norVArray.type, _norVArray.normalized),auxStride);

        setNormalBuffer(aux);
    }

    if ( _activeBuffer & 0x10 ) // Texture Array Modified
    {

		for(int currentTU = 0; currentTU < 8; currentTU++)
		{
			if (isEnabledClientTextureUnit(currentTU))
			{
				if (!_texVArray[currentTU].inicialized)
					panic ("GLContext","setVertexArraysBuffers", "Texture VertexArray struct is not inicialized");

				aux = configureStream(&_texVArray[currentTU], false, start, count, iSet);

				if (_texVArray[currentTU].stride == 0) auxStride = _texVArray[currentTU].size * getSize(_texVArray[currentTU].type);
				else auxStride = _texVArray[currentTU].stride;

				if(_texVArray[currentTU].bufferID != 0)
					setDescriptor((AGL_ARRAY)(TEX_COORD_ARRAY_0+currentTU),u64bit(_texVArray[currentTU].userBuffer),_texVArray[currentTU].size, agl::getACDStreamType(_texVArray[currentTU].type, _texVArray[currentTU].normalized),auxStride);
				else
					setDescriptor((AGL_ARRAY)(TEX_COORD_ARRAY_0+currentTU),0 ,_texVArray[currentTU].size, agl::getACDStreamType(_texVArray[currentTU].type, _texVArray[currentTU].normalized),auxStride);

				setTexBuffer(aux);
			}
		}
    }

    for ( GLuint i = 0; i < _genericAttribArrayFlags.size(); i++ )
    {
        if ( _genericAttribArrayFlags[i] )
        {

            if (!_genericVArray[i].inicialized)
                panic ("GLContext","setVertexArraysBuffers", "Generic VertexArray struct is not inicialized");

            aux = configureStream(&_genericVArray[i], false, start, count, iSet);

            if (_genericVArray[i].stride == 0) auxStride = _genericVArray[i].size * getSize(_genericVArray[i].type); /*Tamany objecte*/
            else auxStride = _genericVArray[i].stride;

            if(_genericVArray[i].bufferID != 0)
                setDescriptor((AGL_ARRAY)(GENERIC_ARRAY_0+i), u64bit(_genericVArray[i].userBuffer), _genericVArray[i].size, agl::getACDStreamType(_genericVArray[i].type, _genericVArray[i].normalized), auxStride);
            else
                setDescriptor((AGL_ARRAY)(GENERIC_ARRAY_0+i), 0, _genericVArray[i].size, agl::getACDStreamType(_genericVArray[i].type, _genericVArray[i].normalized), auxStride);

            _genericBuffer[i] = aux;

        }
    }
}

ACDBuffer* GLContext::configureStream (VArray *va, bool colorAttrib, GLuint start, GLuint count, set<GLuint>& iSet)
{

    if ( va->bufferID != 0)
    {
        BufferObject bo = _bufferManager.getBuffer(GL_ARRAY_BUFFER, va->bufferID);
        return bo.getData();
    }
    else
    {

        bool indexedMode = ( iSet.empty() ? false : true ); // Check indexed mode

        if ( colorAttrib )
        {
            if ( indexedMode )
                return fillVAColor_ind(va->type, va->userBuffer, va->size, va->stride, iSet);
            else
            {
                ACDBuffer* aux = fillVAColor(va->type, va->userBuffer, va->size, va->stride, start, count);
                va->stride = 0;
                return aux;
            }
        }
        else
        {
            if ( indexedMode )
                return fillVA_ind(va->type, va->userBuffer, va->size, va->stride, iSet);
            else
            {
                ACDBuffer* aux = fillVA(va->type, va->userBuffer, va->size, va->stride, start, count);
                va->stride = 0;
                return aux;
            }
        }

    }
}

ACDBuffer* GLContext::fillVA_ind(GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            return fillVA_ind((GLubyte*)buf, size, stride, iSet);
            break;
        case GL_BYTE:
            return fillVA_ind((GLbyte*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_SHORT:
            return fillVA_ind((GLushort*)buf, size, stride, iSet);
            break;
        case GL_SHORT:
            return fillVA_ind((GLshort*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_INT:
            return fillVA_ind((GLuint*)buf, size, stride, iSet);
            break;
        case GL_INT:
            return fillVA_ind((GLint*)buf, size, stride, iSet);
            break;
        case GL_FLOAT:
            return fillVA_ind((GLfloat*)buf, size, stride, iSet);
            break;
        case GL_DOUBLE:
            return fillVA_ind((GLdouble*)buf, size, stride, iSet);
            break;
        default:
            panic("GLContext", "fillVA_ind", "Unsupported type");
            return 0;
    }
}

ACDBuffer* GLContext::fillVAColor_ind(GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            return fillVAColor_ind((GLubyte*)buf, size, stride, iSet);
            break;
        case GL_BYTE:
            return fillVAColor_ind((GLbyte*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_SHORT:
            return fillVAColor_ind((GLushort*)buf, size, stride, iSet);
            break;
        case GL_SHORT:
            return fillVAColor_ind((GLshort*)buf, size, stride, iSet);
            break;
        case GL_UNSIGNED_INT:
            return fillVAColor_ind((GLuint*)buf, size, stride, iSet);
            break;
        case GL_INT:
            return fillVAColor_ind((GLint*)buf, size, stride, iSet);
            break;
        case GL_FLOAT:
            return fillVAColor_ind((GLfloat*)buf, size, stride, iSet);
            break;
        case GL_DOUBLE:
            return fillVAColor_ind((GLdouble*)buf, size, stride, iSet);
            break;
        default:
            panic("GLContext", "fillVAColor_ind", "Unsupported type");
            return 0;
    }
}

ACDBuffer* GLContext::fillVA (GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            return fillVA((GLubyte*)buf, size, stride, start, count);
            break;
        case GL_BYTE:
            return fillVA((GLbyte*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_SHORT:
            return fillVA((GLushort*)buf, size, stride, start, count);
            break;
        case GL_SHORT:
            return fillVA((GLshort*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_INT:
            return fillVA((GLuint*)buf, size, stride, start, count);
            break;
        case GL_INT:
            return fillVA((GLint*)buf, size, stride, start, count);
            break;
        case GL_FLOAT:
            return fillVA((GLfloat*)buf, size, stride, start, count);
            break;
        case GL_DOUBLE:
            return fillVA((GLdouble*)buf, size, stride, start, count);
            break;
        default:
            panic("GLContext", "fillVA", "Unsupported type");
            return 0;
    }
}

ACDBuffer* GLContext::fillVAColor (GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count)
{
    switch (type )
    {
        case GL_UNSIGNED_BYTE:
            return fillVAColor((GLubyte*)buf, size, stride, start, count);
            break;
        case GL_BYTE:
            return fillVAColor((GLbyte*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_SHORT:
            return fillVAColor((GLushort*)buf, size, stride, start, count);
            break;
        case GL_SHORT:
            return fillVAColor((GLshort*)buf, size, stride, start, count);
            break;
        case GL_UNSIGNED_INT:
            return fillVAColor((GLuint*)buf, size, stride, start, count);
            break;
        case GL_INT:
            return fillVAColor((GLint*)buf, size, stride, start, count);
            break;
        case GL_FLOAT:
            return fillVAColor((GLfloat*)buf, size, stride, start, count);
            break;
        case GL_DOUBLE:
            return fillVAColor((GLdouble*)buf, size, stride, start, count);
            break;
        default:
            panic("GLContext", "fillVAColor", "Unsupported type");
            return 0;
    }
}

template<typename T>
ACDBuffer* GLContext::fillVA_ind(const T* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet)
{
    using namespace std;

    int j;

    if ( stride % sizeof(T) != 0 )
        panic("GLContext", "fillVA_ind", "stride must be a multiple of T");

    if ( stride == 0 )
        stride = size;
    else
        stride /= sizeof(T);

    ACDBuffer *aux = _acddev->createBuffer();

    set<GLuint>::const_iterator it = iSet.begin();

    GLfloat auxFloat;
    for ( ; it != iSet.end(); it++ )
    {
        j = stride * (*it);

        auxFloat = (GLfloat)buf[j];
        aux->pushData(&auxFloat, sizeof(acd_float));
        auxFloat = (GLfloat)buf[j+1];
        aux->pushData(&auxFloat, sizeof(acd_float));

        if ( size == 3 )
        {
            auxFloat = (GLfloat)buf[j+2];
            aux->pushData(&auxFloat, sizeof(acd_float));
        }
        else if ( size == 4 )
        {
            auxFloat = (GLfloat)buf[j+2];
            aux->pushData(&auxFloat, sizeof(acd_float));
            auxFloat = (GLfloat)buf[j+3];
            aux->pushData(&auxFloat, sizeof(acd_float));
        }
    }

    return aux;
}

template<typename A>
ACDBuffer* GLContext::fillVAColor_ind(const A* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet)
{
    ACDBuffer *aux = _acddev->createBuffer();

    if ( size != 3 && size != 4 )
        panic("GLContext", "fillVAColor_ind", "size (nc), must be 3 or 4");

    if ( stride % sizeof(A) != 0 )
        panic("GLContext", "fillVAColor_ind", "stride must be a multiple of T");

#define __U_TO_FLOAT(divValue) {\
    r = GLfloat(buf[i])/divValue;\
    g = GLfloat(buf[i+1])/divValue;\
    b = GLfloat(buf[i+2])/divValue;\
    if ( size == 4 )\
        a = GLfloat(buf[i+3])/divValue;}

#define __TO_FLOAT(divValue) {\
    r = (2.0f * float(buf[i])+1)/divValue;\
    g = (2.0f * float(buf[i+1])+1)/divValue;\
    b = (2.0f * float(buf[i+2])+1)/divValue;\
    if ( size == 4 )\
        a = (2.0f * float(buf[i+3])) / divValue;}

    if ( stride == 0 )
        stride = size; // item size
    else
        stride /= sizeof(A);

    GLfloat r, g, b, a;

    int i;

    GLfloat rFloat;
    GLfloat gFloat;
    GLfloat bFloat;

    std::set<GLuint>::const_iterator it = iSet.begin();

    for (int j = 0 ; it != iSet.end(); it++, j++ )
    {
        i = stride * (*it); // position in the buffer for element *it

        if ( typeid(A) == typeid(GLubyte) )
            __U_TO_FLOAT(0xFF)

        else if ( typeid(A) == typeid(GLbyte) )
            __TO_FLOAT(0xFF)

        else if ( typeid(A) == typeid(GLushort) )
            __U_TO_FLOAT(0xFFFF)

        else if ( typeid(A) == typeid(GLshort) )
            __TO_FLOAT(0xFFFF)

        else if ( typeid(A) == typeid(GLuint) )
            __U_TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(A) == typeid(GLint) )
            __TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(A) == typeid(GLfloat) )
        {
            r = float(buf[i]);
            g = float(buf[i+1]);
            b = float(buf[i+2]);
            if ( size == 4 )
                a = float(buf[i+3]);
        }

        else if ( typeid(A) == typeid(GLdouble) )
        {
            r = GLfloat(buf[i]);
            g = GLfloat(buf[i+1]);
            b = GLfloat(buf[i+2]);
            if ( size == 4 )
                a = GLfloat(buf[i+3]);
        }

        else
            panic("GLContext","fillVAColor_ind","Unsupported 'templatized' type");

        rFloat = r;
        gFloat = g;
        bFloat = b;
        aux->pushData (&rFloat, sizeof(float));
        aux->pushData (&gFloat, sizeof(float));
        aux->pushData (&bFloat, sizeof(float));

        if ( size != 3 )
            aux->pushData (&a, sizeof(float));
    }


    #undef __U_TO_FLOAT
    #undef __TO_FLOAT
    return aux;
}

template<typename T>
ACDBuffer* GLContext::fillVA(const T* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count)
{
    using namespace std;

    int i, j;

    if ( stride % sizeof(T) != 0 )
        panic("GLContext", "fillVA", "stride must be a multiple of T");

    if ( stride == 0 )
        stride = size;
    else
        stride /= sizeof(T);

    j = start;
    i = start * stride;
    count += start;

    ACDBuffer *aux = _acddev->createBuffer();

    GLfloat auxFloat;
    for ( ; j < count; i += stride, j++ )
    {
        auxFloat = (GLfloat)buf[i];
        aux->pushData(&auxFloat, sizeof(acd_float));
        auxFloat = (GLfloat)buf[i+1];
        aux->pushData(&auxFloat, sizeof(acd_float));

        if ( size == 3 )
        {
            auxFloat = (GLfloat)buf[i+2];
            aux->pushData(&auxFloat, sizeof(acd_float));
        }
        else if( size == 4)
        {
            auxFloat = (GLfloat)buf[i+2];
            aux->pushData(&auxFloat, sizeof(acd_float));
            auxFloat = (GLfloat)buf[i+3];
            aux->pushData(&auxFloat, sizeof(acd_float));
        }
    }

    return aux;
}

template<typename A>
ACDBuffer* GLContext::fillVAColor(const A* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count)
{
    ACDBuffer *aux = _acddev->createBuffer();

    if ( size != 3 && size != 4 )
        panic("GLContext", "fillVAColor", "size (nc), must be 3 or 4");

    if ( stride % sizeof(A) != 0 )
        panic("GLContext", "fillVAColor", "stride must be a multiple of T");

#define __U_TO_FLOAT(divValue) {\
    r = GLfloat(buf[i])/divValue;\
    g = GLfloat(buf[i+1])/divValue;\
    b = GLfloat(buf[i+2])/divValue;\
    if ( size == 4 )\
        a = GLfloat(buf[i+3])/divValue;}

#define __TO_FLOAT(divValue) {\
    r = (2.0f * float(buf[i])+1)/divValue;\
    g = (2.0f * float(buf[i+1])+1)/divValue;\
    b = (2.0f * float(buf[i+2])+1)/divValue;\
    if ( size == 4 )\
        a = (2.0f * float(buf[i+3])) / divValue;}

    if ( stride == 0 )
        stride = size; // item size
    else
        stride /= sizeof(A);

    GLfloat r, g, b, a;

    int i, j;

    j = start;
    i = start * stride;
    count += start;

    GLfloat rFloat;
    GLfloat gFloat;
    GLfloat bFloat;

    for ( ; j < count; i+= stride, j++ )
    {
        if ( typeid(A) == typeid(GLubyte) )
            __U_TO_FLOAT(0xFF)

        else if ( typeid(A) == typeid(GLbyte) )
            __TO_FLOAT(0xFF)

        else if ( typeid(A) == typeid(GLushort) )
            __U_TO_FLOAT(0xFFFF)

        else if ( typeid(A) == typeid(GLshort) )
            __TO_FLOAT(0xFFFF)

        else if ( typeid(A) == typeid(GLuint) )
            __U_TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(A) == typeid(GLint) )
            __TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(A) == typeid(GLfloat) )
        {
            r = float(buf[i]);
            g = float(buf[i+1]);
            b = float(buf[i+2]);
            if ( size == 4 )
                a = float(buf[i+3]);
        }

        else if ( typeid(A) == typeid(GLdouble) )
        {
            r = GLfloat(buf[i]);
            g = GLfloat(buf[i+1]);
            b = GLfloat(buf[i+2]);
            if ( size == 4 )
                a = GLfloat(buf[i+3]);
        }

        else
            panic("GLContext","fillVAColor","Unsupported 'templatized' type");

        rFloat = r;
        gFloat = g;
        bFloat = b;
        aux->pushData (&rFloat, sizeof(float));
        aux->pushData (&gFloat, sizeof(float));


		if ( size == 3)
		{
			aux->pushData (&bFloat, sizeof(float));
		}
        else if ( size == 4 )
        {
			aux->pushData (&bFloat, sizeof(float));
            aux->pushData (&a, sizeof(float));
        }

    }

    #undef __U_TO_FLOAT
    #undef __TO_FLOAT
    return aux;
}

bool GLContext::allBuffersBound() const
{
    if ( (_activeBuffer & 0x01) == 0x01)
        if ( _posVArray.bufferID == 0 )
            return false;

    if ( (_activeBuffer & 0x02) == 0x02)
        if ( _colVArray.bufferID == 0 )
            return false;

    if ( (_activeBuffer & 0x04) == 0x04)
        if ( _norVArray.bufferID == 0 )
            return false;

    return true;
}

bool GLContext::allBuffersUnbound() const
{
    if ((_activeBuffer & 0x01) == 0x01)
        if ( _posVArray.bufferID != 0 )
            return false;

    if ((_activeBuffer & 0x02) == 0x02)
        if ( _colVArray.bufferID != 0 )
            return false;

    if ((_activeBuffer & 0x04) == 0x04)
        if ( _norVArray.bufferID != 0 )
            return false;

    return true;
}

ACDBuffer* GLContext::rawIndices(const GLvoid* indices, GLenum type, GLsizei count)
{
    GLsizei dataSize = getSize(type) * count;

    GLubyte* ub_ptr = new GLubyte[dataSize];
    GLushort* us_ptr = (GLushort*)ub_ptr;
    GLuint* ui_ptr = (GLuint*)ub_ptr;
    GLint i = 0;

    switch ( type )
    {
        case GL_UNSIGNED_BYTE:
            for ( ; i < count; i++ )
                ub_ptr[i] = ((GLubyte*)indices)[i];
            break;
        case GL_UNSIGNED_SHORT:
            for ( ; i < count; i++ )
                us_ptr[i] = ((GLushort*)indices)[i];
            break;
        case GL_UNSIGNED_INT:
            for ( ; i < count; i++ )
                ui_ptr[i] = ((GLuint*)indices)[i];
            break;
        default:
            panic("GLContext", "rawIndices", "Unsupported indices type");
    }
 
    ACDBuffer *indexBuffer = _acddev->createBuffer(dataSize, (const acdlib::acd_ubyte*)ub_ptr);

    delete[] ub_ptr;
            
    return indexBuffer;
}

void GLContext::resetDescriptors ()
{
    _vertexDesc.offset = 0;
    _vertexDesc.components = 4;
    _vertexDesc.frequency = 0;
    _vertexDesc.type = ACD_SD_FLOAT32;
    _vertexDesc.stride = 16;

    _colorDesc.offset = 0;
    _colorDesc.components = 4;
    _colorDesc.frequency = 0;
    _colorDesc.type = ACD_SD_FLOAT32;
    _colorDesc.stride = 16;

    _normalDesc.offset = 0;
    _normalDesc.components = 3;
    _normalDesc.frequency = 0;
    _normalDesc.type = ACD_SD_FLOAT32;
    _normalDesc.stride = 12;

    for(int i = 0; i < TEXTURE_UNITS; i++)
    {
        _textureDesc[i].offset = 0;
        _textureDesc[i].components = 4;
        _textureDesc[i].frequency = 0;
        _textureDesc[i].type = ACD_SD_FLOAT32;
        _textureDesc[i].stride = 16;
    }
}

void GLContext::fillVertexArrayBuffer (AGL_ARRAY stream, GLint count)
{
    switch (stream)
    {
        case COLOR_ARRAY:
            _colorBuffer = _acddev->createBuffer();
            for(int i=0; i<count; i++)
               _colorBuffer->pushData(_currentColor, 4*sizeof(acd_float));
            break;

        case NORMAL_ARRAY:
            _normalBuffer = _acddev->createBuffer();
            for(int i=0; i<count; i++)
               _normalBuffer->pushData(_currentNormal, 4*sizeof(acd_float));
            break;

        case TEX_COORD_ARRAY_0:
        case TEX_COORD_ARRAY_1:
        case TEX_COORD_ARRAY_2:
        case TEX_COORD_ARRAY_3:
        case TEX_COORD_ARRAY_4:
        case TEX_COORD_ARRAY_5:
        case TEX_COORD_ARRAY_6:
        case TEX_COORD_ARRAY_7:

            _textureBuffer[stream - TEX_COORD_ARRAY_0] = _acddev->createBuffer();
            for(int i=0; i<count; i++)
               _textureBuffer[stream - TEX_COORD_ARRAY_0]->pushData(_currentTexCoord[stream - TEX_COORD_ARRAY_0], 4*sizeof(acd_float));
            break;

        default:
            panic("GLContext","fillVertexArrayBuffer","Incorrect array selected");
    }
}

void GLContext::setTexCoord(GLuint unit, GLfloat r, GLfloat s, GLfloat t, GLfloat q)
{
    if ( unit >= _textureUnits.size() )
        panic("GLContext", "setTexCoord", "Unit not available");

    _currentTexCoord[unit][0] = r;
    _currentTexCoord[unit][1] = s;
    _currentTexCoord[unit][2] = t;
    _currentTexCoord[unit][3] = q; // by default 1
}

TextureUnit& GLContext::getTextureUnit(GLuint unit)
{
    if ( unit >= _textureUnits.size() )
        panic("GLContext", "getTextureUnit", "Texture unit %d does not exist");

    return _textureUnits[unit];
}

TextureUnit& GLContext::getActiveTextureUnit()
{
    return getTextureUnit(_textureManager.getCurrentGroup());
}

GLuint GLContext::getActiveTextureUnitNumber()
{
    return _textureManager.getCurrentGroup();
}

void GLContext::setFog(GLenum pname, const GLfloat* params)
{
    //using namespace glsNS;
    if ( pname == GL_FOG_COLOR )
        fixedPipelineState().postshade().setFOGBlendColor(params[0], params[1], params[2], params[3]);

    else if ( pname == GL_FOG_DENSITY )
        fixedPipelineState().postshade().setFOGDensity(params[0]);

    else if ( pname == GL_FOG_START )
    {
        _fogStart = params[0];
        fixedPipelineState().postshade().setFOGLinearDistances(_fogStart, _fogEnd);
    }
    else if ( pname == GL_FOG_END )
    {
        _fogEnd = params[0];
        fixedPipelineState().postshade().setFOGLinearDistances(_fogStart, _fogEnd);
    }
    else
    {
        GLint value = (GLint)params[0];
        setFog(pname, &value); // Call integer version
    }
}

void GLContext::setFog(GLenum pname, const GLint* params)
{

    GLint value = params[0];

    if ( pname == GL_FOG_MODE )
        fixedPipelineSettings().fogMode = agl::getACDXFogMode(value);

    else if ( pname == GL_FOG_COORDINATE_SOURCE )
        fixedPipelineSettings().fogCoordinateSource = agl::getACDXFogCoordinateSource(value);

    else if ( pname == GL_FOG_COLOR )
        fixedPipelineState().postshade().setFOGBlendColor((GLfloat)params[0], (GLfloat)params[1], (GLfloat)params[2], (GLfloat)params[3]);

    else if ( pname == GL_FOG_DENSITY )
        fixedPipelineState().postshade().setFOGDensity((GLfloat)params[0]);

    else if ( pname == GL_FOG_START )
    {
        _fogStart = (GLfloat)params[0];
        fixedPipelineState().postshade().setFOGLinearDistances(_fogStart, _fogEnd);
    }
    else if ( pname == GL_FOG_END )
    {
        _fogEnd = (GLfloat)params[0];
        fixedPipelineState().postshade().setFOGLinearDistances(_fogStart, _fogEnd);
    }
    else
        panic("GLContext", "setFog", "Unexpected FOG parameter");
}



void GLContext::setCurrentClientTextureUnit(GLenum tex)
{
    if ( tex >= TEXTURE_UNITS )
        panic("GLContext", "setCurrentClientTextureUnit", "texture unit does not exist");

    _currentClientTextureUnit = tex;
}

bool GLContext::isEnabledClientTextureUnit(GLuint tex)
{
    if ( tex >= _clientTextureUnits.size() )
        panic("GLContext", "isEnabledClientTextureUnit", "Texture unit does not exist");
    return _clientTextureUnits[tex];
}

GLuint GLContext::getCurrentClientTextureUnit() const
{
    return _currentClientTextureUnit;
}

bool GLContext::isEnabledCurrentClientTextureUnit() const
{
    return _clientTextureUnits[_currentClientTextureUnit];
}

void GLContext::setEnabledCurrentClientTextureUnit(bool enabled)
{
	_clientTextureUnits[_currentClientTextureUnit] = enabled;
}

GLContext::VArray& GLContext::getTexVArray(GLuint unit)
{
    if ( unit >= _textureUnits.size() )
        panic("GLContext", "getTexVArray", "Texture unit does not exist");

    return _texVArray[unit];
}

void GLContext::setTexGen(GLenum coord, GLenum pname, GLint param)
{
    switch ( pname )
    {
        case GL_TEXTURE_GEN_MODE:
            {
                GLuint tu = _textureManager.getCurrentGroup();
                getTextureUnit(tu).setTexGenMode(coord, param);
            }
            break;
        default:
            panic("GLContext", "setTexGen", "Unsuported parameter name");
    }
}

void GLContext::setTexGen(GLenum coord, GLenum pname, const GLfloat* params)
{

    GLuint tu = _textureManager.getCurrentGroup();

    switch ( pname )
    {
        case GL_OBJECT_PLANE:
            fixedPipelineState().txtcoord().setTextureCoordPlane(tu,ACDX_OBJECT_PLANE, agl::getACDXTexCoordPlane(coord),params[0],params[1],params[2],params[3]);
            break;

        case GL_EYE_PLANE:
            {
                // OpenGL requires multiply the plane by the current Modelview matrix.
                // The exact operation to do is (p1, p2, p3, p4) * inverse(Modelview).

                acdlib::ACDVector<acdlib::acd_float,4> aux;
                acdlib::_mult_mat_vect(aux,_stackGroup[1][_currentModelview]->get().transpose());

                fixedPipelineState().txtcoord().setTextureCoordPlane(tu,ACDX_EYE_PLANE, agl::getACDXTexCoordPlane(coord),aux[0],aux[1],aux[2],aux[3]);
            }
            break;
        default:
            panic("GLContext", "setTexGen", "Unsuported parameter name");
    }
}

void GLContext::setEnabledGenericAttribArray(GLuint index, bool enabled)
{
    if ( index >= _genericAttribArrayFlags.size() )
        panic("GLContext", "setEnabledGenericAttribArray", "Generic Vertex Array does not exist");

    _genericAttribArrayFlags[index] = enabled;
}

bool GLContext::isGenericAttribArrayEnabled(GLuint index) const
{
    if ( index >= _genericAttribArrayFlags.size() )
        panic("GLContext", "isGenericAttribArrayEnabled", "Generic Vertex Array does not exist");

    return _genericAttribArrayFlags[index];
}

GLuint GLContext::countGenericVArrays() const
{
    return _genericAttribArrayFlags.size();
}

GLContext::VArray& GLContext::getPosVArray()
{
    return _posVArray;
}

GLContext::VArray& GLContext::getColorVArray()
{
    return _colVArray;
}

GLContext::VArray& GLContext::getNormalVArray()
{
    return _norVArray;
}

GLContext::VArray& GLContext::getIndexVArray()
{
    return _indVArray;
}

GLContext::VArray& GLContext::getEdgeVArray()
{
    return _edgeVArray;
}

GLContext::VArray& GLContext::getTextureVArray(GLuint unit)
{
    if ( unit >= _textureUnits.size() )
        panic("GLContext", "getTextureVArray", "Texture unit does not exist");

    return _texVArray[unit];
}

GLContext::VArray& GLContext::getGenericVArray(GLuint attrib)
{
    if ( attrib >= _genericVArray.size() )
        panic("GLContext", "getGenericVArray", "Generic Vertex Array does not exist");

    return _genericVArray[attrib];
}

void GLContext::pushAttrib (GLbitfield mask)
{
    if ( ( mask & GL_ENABLE_BIT ) == GL_ENABLE_BIT )
    {

        /*    flagSeparateSpecular = ctx.testFlags(GLContext::flagSeparateSpecular);
    flagRescaleNormal = ctx.testFlags(GLContext::flagRescaleNormal);
    flagNormalize = ctx.testFlags(GLContext::flagNormalize);
    flagFog = ctx.testFlags(GLContext::flagFog);
    flagStencilTest = ctx.testFlags(GLContext::flagStencilTest);
    flagAlphaTest = ctx.testFlags(GLContext::flagAlphaTest);
    flagCullFace = ctx.testFlags(GLContext::flagCullFace);
    flagDepthTest = ctx.testFlags(GLContext::flagDepthTest);
    flagPolygonOffsetFill = ctx.testFlags(GLContext::flagPolygonOffsetFill);
    flagScissorTest = ctx.testFlags(GLContext::flagScissorTest);
    flagBlending = ctx.testFlags(GLContext::flagBlending);
    lights = ctx.lights;*/

        ACDStoredItemIDList listItems ;
        //listItems.insert(acdlib::ACD_ZST_BACK_COMPARE_FUNC);
        //listItems.insert(ALPHATEST);
        //listItems.insert(ACD_XXX_CULLFACE);
        //listItems.insert(ACD_XXX_DEPTH_TEST);
        //listItems.insert(
        _acddev->saveState(listItems);

    }
    if ( ( mask & GL_COLOR_BUFFER_BIT ) == GL_COLOR_BUFFER_BIT )
    {
/*
    flagAlphaTest = ctx.testFlags(GLContext::flagAlphaTest);
    alphaFunc = ctx.alphaFunc;
    alphaRef = ctx.alphaRef;
    flagBlending = ctx.testFlags(GLContext::flagBlending);
    blendSFactorRGB = ctx.blendSFactorRGB;
    blendDFactorRGB = ctx.blendDFactorRGB;
    blendSFactorAlpha = ctx.blendSFactorAlpha;
    blendDFactorAlpha = ctx.blendDFactorAlpha;
    blendEquation = ctx.blendEquation;
    blendColor = ctx.blendColor;
    red = ctx.red;
    green = ctx.green;
    blue = ctx.blue;
    alpha = ctx.alpha;
    clearColor = ctx.clearColor;
*/



    }
    if ( ( mask & GL_CURRENT_BIT ) == GL_CURRENT_BIT )
    {

/*
    GLuint nTUs = ctx.driver->getTextureUnits();

    for (unsigned int i=0; i < nTUs; i++)
        currentTexCoord.push_back(ctx.currentTexCoord[i]);

*/


    }
    if ( ( mask & GL_DEPTH_BUFFER_BIT ) == GL_DEPTH_BUFFER_BIT )
    {

        /*
             flagDepthTest = ctx.testFlags(GLContext::flagDepthTest);
     depthFunc = ctx.depthFunc;
     depthMask = ctx.depthMask;
     depthClear = ctx.depthClear;
     */


    }
    if ( ( mask & GL_FOG_BIT ) == GL_FOG_BIT )
    {
/*
    flagFog = ctx.testFlags(GLContext::flagFog);
    fogColor = ctx.gls.getVector(V_FOG_COLOR);
    fogParams = ctx.gls.getVector(V_FOG_PARAMS);
    fogMode = ctx.fogMode;
    fogCoordinateSource = ctx.fogCoordinateSource;

    */



    }
    if ( ( mask & GL_LIGHTING_BIT ) == GL_LIGHTING_BIT )
    {

  /*          shadeModel = ctx.shadeModel;
    flagLighting = ctx.testFlags(GLContext::flagLighting);
    flagColorMaterial = ctx.testFlags(GLContext::flagColorMaterial);
    colorMaterialMode = ctx.colorMaterialMode;
    colorMaterialFace = ctx.colorMaterialFace;
    ambientFront = ctx.gls.getVector(V_MATERIAL_FRONT_AMBIENT);
    diffuseFront = ctx.gls.getVector(V_MATERIAL_FRONT_DIFUSSE);
    specularFront = ctx.gls.getVector(V_MATERIAL_FRONT_SPECULAR);
    emissionFront = ctx.gls.getVector(V_MATERIAL_FRONT_EMISSION);
    shininessFront = ctx.gls.getVector(V_MATERIAL_FRONT_SHININESS);
    ambientBack = ctx.gls.getVector(V_MATERIAL_BACK_AMBIENT);
    diffuseBack = ctx.gls.getVector(V_MATERIAL_BACK_DIFUSSE);
    specularBack = ctx.gls.getVector(V_MATERIAL_BACK_SPECULAR);
    emissionBack = ctx.gls.getVector(V_MATERIAL_BACK_EMISSION);
    shininessBack = ctx.gls.getVector(V_MATERIAL_BACK_SHININESS);
    lightModelAmbient = ctx.gls.getVector(V_LIGHTMODEL_AMBIENT);
    flagLocalViewer = ctx.testFlags(GLContext::flagLocalViewer);
    flagTwoSidedLighting = ctx.testFlags(GLContext::flagTwoSidedLighting);
    flagSeparateSpecular = ctx.testFlags(GLContext::flagSeparateSpecular);

    for (int i=0; i < MAX_LIGHTS_ARB; i++)
    {
        ambient[i] = ctx.gls.getVector(V_LIGHT_AMBIENT + 7*i);
        diffuse[i] = ctx.gls.getVector(V_LIGHT_DIFFUSE + 7*i);
        specular[i] = ctx.gls.getVector(V_LIGHT_SPECULAR + 7*i);
        position[i] = ctx.gls.getVector(V_LIGHT_POSITION + 7*i);
        attenuation[i] = ctx.gls.getVector(V_LIGHT_ATTENUATION + 7*i);
        spotParams[i] = ctx.gls.getVector(V_LIGHT_SPOT_DIRECTION + 7*i);

    }

    lights = ctx.lights;
*/

    }
    if ( ( mask & GL_POLYGON_BIT ) == GL_POLYGON_BIT )
    {
/*
    flagCullFace = ctx.testFlags(GLContext::flagCullFace);
    cullFace = ctx.cullFace;
    faceMode = ctx.faceMode;
    slopeFactor = ctx.slopeFactor;
    unitsFactor = ctx.unitsFactor;
    flagPolygonOffsetFill = ctx.testFlags(GLContext::flagPolygonOffsetFill);

    */


    }
    if ( ( mask & GL_SCISSOR_BIT ) == GL_SCISSOR_BIT )
    {
/*
    flagScissorTest = ctx.testFlags(GLContext::flagScissorTest);
    scissorIniX = ctx.scissorIniX;
    scissorIniY = ctx.scissorIniY;
    scissorWidth = ctx.scissorWidth;
    scissorHeight = ctx.scissorHeight;
    */



    }
    if ( ( mask & GL_STENCIL_BUFFER_BIT ) == GL_STENCIL_BUFFER_BIT )
    {
/*    flagStencilTest = ctx.testFlags(GLContext::flagStencilTest);
    stFailUpdateFunc = ctx.stFailUpdateFunc;
    dpFailUpdateFunc = ctx.dpFailUpdateFunc;
    dpPassUpdateFunc =  ctx.dpPassUpdateFunc;
    stBufferClearValue = ctx.stBufferClearValue;
    stBufferUpdateMask = ctx.stBufferUpdateMask;
    stBufferFunc = ctx.stBufferFunc;
    stBufferRefValue = ctx.stBufferRefValue;
    stBufferCompMask = ctx.stBufferCompMask;

    */



    }
    if ( ( mask & GL_TEXTURE_BIT ) == GL_TEXTURE_BIT )
    {

    }
    if ( ( mask & GL_TRANSFORM_BIT ) == GL_TRANSFORM_BIT )
    {
/*
    currentMatrix = ctx.currentMatrix;
    flagNormalize = ctx.testFlags(GLContext::flagNormalize);
    flagRescaleNormal = ctx.testFlags(GLContext::flagRescaleNormal);
    */


    }
    if ( ( mask & GL_VIEWPORT_BIT ) == GL_VIEWPORT_BIT )
    {
/*
    viewportX = ctx.viewportX;
    viewportY = ctx.viewportY;
    viewportWidth = ctx.viewportWidth;
    viewportHeight = ctx.viewportHeight;
    near_ = ctx.near_;
    far_ = ctx.far_;
    */



    }
    if ( ( mask & GL_ACCUM_BUFFER_BIT ) == GL_ACCUM_BUFFER_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS )
    {
        panic("GLContext","pushAttribs","Accum Buffer Group not implemented yet");
    }
    if ( ( mask & GL_EVAL_BIT ) == GL_EVAL_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Eval Group not implemented yet");
    }
    if ( ( mask & GL_HINT_BIT ) == GL_HINT_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Hint Group not implemented yet");
    }
    if ( ( mask & GL_LINE_BIT ) == GL_LINE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Line Group not implemented yet");
    }
    if ( ( mask & GL_MULTISAMPLE_BIT ) == GL_MULTISAMPLE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Multisample Group not implemented yet");
    }
    if ( ( mask & GL_PIXEL_MODE_BIT ) == GL_PIXEL_MODE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Pixel Mode Group not implemented yet");
    }
    if ( ( mask & GL_POINT_BIT ) == GL_POINT_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Point Group not implemented yet");
    }
    if ( ( mask & GL_POLYGON_STIPPLE_BIT ) == GL_POLYGON_STIPPLE_BIT && ( mask & GL_ALL_ATTRIB_BITS ) != GL_ALL_ATTRIB_BITS)
    {
        panic("GLContext","pushAttribs","Polygon Stipple Group not implemented yet");
    }
}

void GLContext::popAttrib (void)
{

}

