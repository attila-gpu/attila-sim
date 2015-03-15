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

#ifndef PUSHSTATE_H
    #define PUSHSTATE_H
    
#include "gl.h"
#include <stack>
#include <bitset>
#include <list>
#include <vector>
#include <map>
#include "QuadReg.h"
#include "ImplementationLimits.h"


/**
 * pushstate namespace includes operations and data structures to save and restore GLContext state.
 * 
 * GLContext state can be saved and restored with the implemented operations and structures of this namespace. 
 * This is used to implement the glPushAttrib() and glPopAttrib GL calls. 
 * The basis of the memento design pattern have been used in this first implementation.
 *
 * The operations and structures include:
 *
 *  - An abstract class AttribGroup defining the virtual methods saveState(), restoreState() and syncState() 
 *    that are implemented by the different subclasses as ViewportGroup or TextureGroup to save, restore and 
 *    commit the related state of GLContext. GLContext has the responsibility for creating these objects
 *    and passes itself as a parameter of the methods saveState() and restoreState(). To save the state of 
 *    GLContext accesing directly to its attributes, THE AttribGroup SUBCLASSES MUST BE DECLARED AS 
 *    FRIEND CLASSES OF GLContext. 
 *  
 *  - An STL stack which can pile up different AttribGroups of GLContext state joining them together in a 
 *    GroupContainerMemento class (the Memento class in the above mentioned design pattern). This 
 *    GroupContainerMemento class uses and STL list of AttribGroup superclass.
 *  
 * To create and pile up a new GroupContainerMemento object with the saved state of GLContext in the stack, 
 * (according to the bitfiled selected in the glPushAttrib()), this function is provided:
 * 
 *  void pushAttrib(TstateStack& stateStack, const GLContext& ctx, const TextureManager& tm, GLbitfield mask);
 * 
 * and, in a similar way, a function to pop the topmost GroupContainerMemento and to restore its state in GLContext
 * is also provided:
 *
 *  void popAttrib(TstateStack& stateStack, GLContext& ctx, TextureManager& tm);
 *
 *
 * @author Jordi Roca Monfort - jroca@ac.upc.es
 * @date 03/02/2006
 * @ver 1.0
 *       
 */

namespace libgl
{
    class GLContext;      // Forward declaration of GLContext to avoid unnecessary include of GLContext header.
    class TextureManager; // Forward declaration of TextureManager to avoid unnecessary include of TextureManager header.
    
    namespace pushstate
    {

        #ifndef Quadf
            typedef QuadReg<float> Quadf;
        #endif
        
        class AttribGroup
        {
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm)=0;
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const =0;
            
            virtual void syncState(GLContext& ctx) const = 0;
           
        };
        
        class EnableGroup: public AttribGroup
        {
        private:
        
            bool flagSeparateSpecular;                               ///< Flag telling specular component is applied after texturing
            bool flagRescaleNormal;                                  ///< Flag telling auto-rescaling normals
            bool flagNormalize;                                      ///< Flag telling auto-normalize normals
            bool flagFog;                                            ///< Flag telling if fog is enabled/disabled
            bool flagStencilTest;                                    ///< Enable/disable stencil test
            bool flagAlphaTest;                                      ///< Enable/disable alpha test
            bool flagCullFace;                                       ///< Enable/disable face culling
            bool flagDepthTest;                                      ///< Enable/disable depth testing
            bool flagPolygonOffsetFill;                              ///< Enable/disable Polygon Offset for fill rasterization mode
            bool flagScissorTest;                                    ///< Enable/disable Scissor test
            bool flagBlending;                                       ///< Enable/disable blending
            std::bitset<MAX_LIGHTS_ARB> lights;                      ///< Flags for enable/disable lights.
            std::vector<std::map<GLenum, GLboolean> > texGenEnabled; ///< Texgen enabled for each texture coordinate
                                                                     ///  and each Texture Unit.
            
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class ColorGroup: public AttribGroup
        {
        private:
            
            bool      flagAlphaTest;        ///< Enable/disable alpha test
            GLenum    alphaFunc;            ///< The alpha test comparison function.
            GLclampf  alphaRef;             ///< The alpha test reference value.
            bool      flagBlending;         ///< Enable/disable blending
            GLenum    blendSFactorRGB;      ///< The blending equation Source Factor for RGB. (GL Spec. v1.3)
            GLenum    blendDFactorRGB;      ///< The blending equation Destination Factor for RGB. (GL Spec. v1.3)
            GLenum    blendSFactorAlpha;    ///< The blending equation Source Factor for Alpha. (GL Spec. v1.3)
            GLenum    blendDFactorAlpha;    ///< The blending equation Destination Factor for Alpha. (GL Spec. v1.3)
            GLenum    blendEquation;        ///< The blending equation (GL Spec. v1.5) 
            Quadf     blendColor;           ///< The blending equation constant color.
            GLboolean red;                  ///< Color mask for red component.
            GLboolean green;                ///< Color mask for green component.
            GLboolean blue;                 ///< Color mask for blue component.
            GLboolean alpha;                ///< Color mask for alpha component.
            Quadf     clearColor;           ///< Color when clearing color buffer.
            
            
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class CurrentGroup: public AttribGroup
        {
        private:
        
            Quadf  currentColor;                  ///< Current color
            Quadf  currentNormal;                 ///< Current normal
            std::vector<Quadf> currentTexCoord;   ///< Current Texture coordinate (one per texture unit)
        
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class DepthBufferGroup: public AttribGroup
        {
        private:
            
            GLboolean flagDepthTest; ///< Enable/disable depth testing
            GLclampd  depthClear;    ///< Stores the current depth clear value.
            GLboolean depthMask;     ///< if true, updates z buffer, false, doesn't update it
            GLenum    depthFunc;     ///< Stores the current depth function.
        
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class FogGroup: public AttribGroup
        {
        private:
        
            GLboolean flagFog;             ///< Enable/disable FOG
            Quadf     fogColor;            ///< Current fog color
            Quadf     fogParams;           ///< Current fog parameters
            GLenum    fogMode;             ///< Current fog mode.
            GLenum    fogCoordinateSource; ///< Current fog coordinate source.
        
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class LightingGroup: public AttribGroup
        {
        private:
        
            GLboolean shadeModel;                  ///< Current shade model. True if smooth mode, false if flat mode.
            GLboolean flagLighting;                ///< Flag telling if lighting is enabled or not
            GLboolean flagColorMaterial;           ///< Enabled or disabled colorMaterial
            GLenum    colorMaterialMode;           ///< Color material parameter used in lighting computation.
            GLenum    colorMaterialFace;           ///< Color material face mode.
            Quadf     ambientFront;                ///< Ambient color for the front face material
            Quadf     diffuseFront;                ///< Diffuse color for the front face material     
            Quadf     specularFront;               ///< Specular color for the front face material
            Quadf     emissionFront;               ///< Emission color for the front face material
            Quadf     shininessFront;              ///< Shininess factor for the front face material
            Quadf     ambientBack;                 ///< Ambient color for the back face material
            Quadf     diffuseBack;                 ///< Diffuse color for the back face material     
            Quadf     specularBack;                ///< Specular color for the back face material
            Quadf     emissionBack;                ///< Emission color for the back face material
            Quadf     shininessBack;               ///< Shininess factor for the back face material
            Quadf     lightModelAmbient;           ///< Light model ambient color
            GLboolean flagLocalViewer;             ///< Flag telling if local viewer is selected
            GLboolean flagTwoSidedLighting;        ///< Flag telling if lighting is applied to both front & back faces
            GLboolean flagSeparateSpecular;        ///< Flag telling specular component is applied after texturing
            Quadf     ambient[MAX_LIGHTS_ARB];     ///< Ambient color of each light source
            Quadf     diffuse[MAX_LIGHTS_ARB];     ///< Diffuse color of each light source
            Quadf     specular[MAX_LIGHTS_ARB];    ///< Specular color of each light source
            Quadf     position[MAX_LIGHTS_ARB];    ///< Position of each light source and
            Quadf     attenuation[MAX_LIGHTS_ARB]; ///< Attenuation coefs. of each light source
            Quadf     spotParams[MAX_LIGHTS_ARB];  ///< Spot parameters of each light source
            std::bitset<MAX_LIGHTS_ARB> lights;    ///< Flags for enable/disable lights.
        
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class PolygonGroup: public AttribGroup
        {
        private:
            
            GLboolean flagCullFace;             ///< Enable/disable face culling
            GLenum    cullFace;                 ///<  Stores the culling face
            GLenum    faceMode;                 ///<  Stores the polygon facing mode (CW or CCW)
            GLfloat   slopeFactor;              ///< Polygon Offset slope factor parameter.
            GLfloat   unitsFactor;              ///< Polygon Offset units parameter.
            GLboolean flagPolygonOffsetFill;    ///< Enable/disable Polygon Offset for fill rasterization mode
            
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class ScissorGroup: public AttribGroup
        {
        private:
        
            GLboolean flagScissorTest;  ///< Enable/disable Scissor test
            GLint     scissorIniX;      ///< Initial x corner of scissor box
            GLint     scissorIniY;      ///< Initial y corner of scissor box
            GLsizei   scissorWidth;     ///< Width dimension of scissor box
            GLsizei   scissorHeight;    ///< Height dimension of scissor box
            
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class StencilGroup: public AttribGroup
        {
        private:
        
            GLboolean flagStencilTest;     ///< Enable/disable stencil test
            GLenum    stFailUpdateFunc;    ///< Stencil test update function for stencil fail
            GLenum    dpFailUpdateFunc;    ///< Stencil test update function for depth test fail
            GLenum    dpPassUpdateFunc;    ///< Stencil test update function for depth test pass
            GLint     stBufferClearValue;  ///< Stencil buffer clear value
            GLuint    stBufferUpdateMask;  ///< Stencil buffer write mask
            GLenum    stBufferFunc;        ///< Stencil test compare function
            GLint     stBufferRefValue;    ///< Stencil test reference value to compare with
            GLuint    stBufferCompMask;    ///< Stencil test comparison mask
        
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
          
            virtual void syncState(GLContext& ctx) const;
              
        };
        
        class TransformGroup: public AttribGroup
        {
        private:
        
            GLuint    currentMatrix;      ///< Current work matrix
            GLboolean flagNormalize;      ///< Flag telling auto-normalize normals
            GLboolean flagRescaleNormal;  ///< Flag telling auto-rescaling normals
            
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class ViewportGroup: public AttribGroup
        {
        private:
            
            GLint    viewportX;        ///<  Viewport start x coordinate
            GLint    viewportY;        ///<  Viewport start y coordinate
            GLsizei  viewportWidth;    ///<  Viewport width
            GLsizei  viewportHeight;   ///<  Viewport height
            GLclampd near_;            ///<  Depth near value
            GLclampd far_;             ///<  Depth far value
    
        public:
        
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
          
            virtual void syncState(GLContext& ctx) const;
              
        };
        
        class TextureGroup: public AttribGroup
        {
        private:
        
            /** NEEDS TO BE FRIEND CLASS OF TEXTUREUNITLIB AND TEXTURE OBJECTS **/

            GLenum                                    activeTextureUnit;    ///< The Active texture Unit selector.
            
            /**
             * State per Texture Unit
             */
            std::vector<std::map<GLenum, GLboolean> > enabledTargets;      ///< Enabled Texture Target.
            std::vector<std::map<GLenum,GLuint> >     textureObjectBounds; ///< Texture Objects bound to each Texture Target.
            std::vector<std::map<GLenum, GLboolean> > texGenEnabled;       ///< Texgen enabled for each texture coord.
            std::vector<GLenum>                       textureEnvModes;     ///< Texture environment application functions.
            std::vector<Quadf>                        textureEnvColors;    ///< Texture environment colors.
            std::vector<GLfloat>                      textureUnitLODBias;  ///< Texture Units LOD biases.
            std::vector<std::map<GLenum,Quadf> >      texGenPlaneEqCoefs;  ///< Texgen plane equation coeffs. for each coord.
            std::vector<std::map<GLenum,Quadf> >      texGenObjEqCoefs;    ///< Texgen object linear coeffs. for each coord.
            std::vector<std::map<GLenum, GLenum> >    texGenModes;         ///< Function used for texgen for each coord.
            std::vector<GLenum>                       texEnvCombRGB;       ///< Texture env. combiner function for RGB.
            std::vector<GLenum>                       texEnvCombAlpha;     ///< Texture env. combiner function for ALPHA.
            std::vector<std::map<GLenum, GLenum> >    texEnvSourcesRGB;    ///< Texture env. sources RGB.
            std::vector<std::map<GLenum, GLenum> >    texEnvSourcesAlpha;  ///< Texture env. sources Alpha.
            std::vector<std::map<GLenum, GLenum> >    texEnvOperandsRGB;   ///< Texture env. operands RGB.
            std::vector<std::map<GLenum, GLenum> >    texEnvOperandsAlpha; ///< Texture env. operands Alpha.
            std::vector<GLint>                        texScaleRGB;         ///< Texture env. RGB post-combiner scaling.
            std::vector<GLint>                        texScaleAlpha;       ///< Texture env. Alpha post-combiner scaling.
            
            
            /**
             * State per Texture Object currently bound to some texture unit
             */
            std::map<GLenum, Quadf>                    textureBorderColors;  ///< Texture border colors.
            std::map<GLenum, GLenum>                   textureMinFilters;    ///< Texture minification filters.
            std::map<GLenum, GLenum>                   textureMagFilters;    ///< Texture magnification filters.
            std::map<GLenum, std::map<GLenum,GLenum> > textureWrapModes;     ///< Texture Wrap modes for each texture coord.
            std::map<GLenum, GLclampf>                 textureObjectPriority;///< Texture Object priority.
            std::map<GLenum, GLboolean>                textureResidency;     ///< Texture Residency.
            std::map<GLenum, GLfloat>                  textureMinLODs;       ///< Texture Min LODs.
            std::map<GLenum, GLfloat>                  textureMaxLODs;       ///< Texture Max LODs.
            std::map<GLenum, GLuint>                   textureBaseLevels;    ///< Texture Base Levels.
            std::map<GLenum, GLuint>                   textureMaxLevels;     ///< Texture Max Levels.
            std::map<GLenum, GLfloat>                  textureLODBias;       ///< Texture LOD Bias for each Texture Object.
            std::map<GLenum, GLenum>                   textureDepthModes;    ///< Texture Depth modes.
            std::map<GLenum, GLenum>                   textureCompareModes;  ///< Texture comparison modes.
            std::map<GLenum, GLenum>                   textureCompareFuncs;  ///< Texture comparison function.
            std::map<GLenum, GLboolean>                textureAutoGenMipMap; ///< Texture Automatic MipMap generation.
            
        
        public:
            
            virtual void saveState(const GLContext& ctx, const TextureManager& tm);
            
            virtual void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            virtual void syncState(GLContext& ctx) const;
            
        };
        
        class GroupContainerMemento
        {
        private:
            
             std::list<AttribGroup*> groupList;
        
        public:
            
            void addAttribGroup(AttribGroup* group);
            
            void restoreState(GLContext& ctx, TextureManager& tm) const;
            
            void syncState(GLContext& ctx) const;
            
            ~GroupContainerMemento();
        };
        
        typedef std::stack<GroupContainerMemento*> TstateStack;
        
        void pushAttrib(TstateStack& stateStack, const GLContext& ctx, const TextureManager& tm, GLbitfield mask);
        
        void popAttrib(TstateStack& stateStack, GLContext& ctx, TextureManager& tm);

    } // namespace pushstate

} // namespace libgl

#endif // PUSHSTATE_H
