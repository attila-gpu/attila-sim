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

#ifndef ACD_DEVICE
    #define ACD_DEVICE

#include "ACDTypes.h"
#include "ACDBuffer.h"
#include "ACDStream.h"
#include "ACDStoredItemID.h"
#include "ACDTextureCubeMap.h"
#include "ACDTexture3D.h"
#include "ACDSampler.h"

#include <list>

namespace acdlib
{

// Forward declaration of interfaces (will be includes)
class ACDSampler;
class ACDResource;
class ACDTexture;
class ACDTexture2D;
class ACDTextureCubeMap;
class ACDRenderTarget;
class ACDZStencilTarget;

class ACDShaderProgram;

class ACDRasterizationStage;
class ACDZStencilStage;
class ACDBlendingStage;

class ACDStoredState;

/**
 * Stored item list definition
 */
typedef std::list<ACD_STORED_ITEM_ID> ACDStoredItemIDList;


/**
 * Atila geometric primitive identifiers
 */
enum ACD_PRIMITIVE
{
    ACD_PRIMITIVE_UNDEFINED,
    ACD_POINTS,
    ACD_LINES,
    ACD_LINE_LOOP,
    ACD_LINE_STRIP,
    ACD_TRIANGLES,
    ACD_TRIANGLE_STRIP,
    ACD_TRIANGLE_FAN,
    ACD_QUADS,
    ACD_QUAD_STRIP
};

enum ACD_SHADER_TYPE
{
    ACD_VERTEX_SHADER,
    ACD_FRAGMENT_SHADER,
    ACD_GEOMETRY_SHADER,
};

struct ACD_SHADER_LIMITS
{
    // Registers available per type
    acd_uint inRegs;
    acd_uint outRegs;
    acd_uint tempRegs;
    acd_uint constRegs;
    acd_uint addrRegs;

    // Read ports available
    acd_uint inReadPorts;
    acd_uint outReadPorts;
    acd_uint tempReadPorts;
    acd_uint constReadPorts;
    acd_uint addrReadPorts;

    // Instructions limits
    acd_uint staticInstrs;
    acd_uint dynInstrs;
};

struct ACD_CONFIG_OPTIONS
{
    acd_bool earlyZ; ///< Enables or disables early Z
    acd_bool hierarchicalZ; ///< Enables or disables hierarchical Z
};


/**
 * This interface represents the high-level abstraction of Atila GPU
 *
 * It includes access to two interfaces ACDResourceManager (for managing resources such as buffers, 
 * textures, etc) and ACDStateManager (to manage GPU state that can be saved and restored)
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @version 1.0
 * @date 01/19/2007
 */
class ACDDevice
{
public:

    /**
     * Set configuration options to the ACDDevice
     */
    virtual void setOptions(const ACD_CONFIG_OPTIONS& configOptions) = 0;

    /**
     * Sets the primitive to assemble vertex data
     */
    virtual void setPrimitive(ACD_PRIMITIVE primitive) = 0;

    /**
     * Gets an interface to the blending stage
     */
    virtual ACDBlendingStage& blending() = 0;

    /**
     * Gets an interface to the rasterization stage
     */
    virtual ACDRasterizationStage& rast() = 0;

    /**
     * Gets a configuration interface for the Depth & Stencil stage
     */
    virtual ACDZStencilStage& zStencil() = 0;

    /**
     * Creates a Texture2D object
     */
    virtual ACDTexture2D* createTexture2D() = 0;

    /**
     * Creates a Texture2D object
     */
    virtual ACDTexture3D* createTexture3D() = 0;

    /**
     * Creates a TextureCubeMap object
     */
    virtual ACDTextureCubeMap* createTextureCubeMap() = 0;

    /**
     * Creates a ACD Buffer object 
     *
     * @note data can be null to create an uninitilized buffer of size 'size'
     * @note size can be 0 (if data is null) -> resize or pushData should be called before updating the buffer
     */
    virtual ACDBuffer* createBuffer(acd_uint size = 0, const acd_ubyte* data = 0) = 0;

    /**
     * Release a ACDResource interface
     *
     * @param resourcePtr a pointer to the resource interface
     *
     * @returns true if the method succeds, false otherwise
     */
    virtual acd_bool destroy(ACDResource* resourcePtr) = 0;

    /**
     * Releases a ACDShaderProgram interface
     *
     * @param shProgram a pointer to the shader program interface
     * @return true if the mehod succeds, false otherwise
     */
    virtual acd_bool destroy(ACDShaderProgram* shProgram) = 0;

    /**
     * Sets the current display resolution
     *
     * @param width Resolution width in pixels
     * @param height Resolution height in pixels
     */
    virtual void setResolution(acd_uint width, acd_uint height) = 0;

    /**
     * Gets the current display resolution
     *
     * @retval width Resolution width in pixels
     * @retval height Resolution height in pixels
     * @return true if the resolution is defined, false otherwise
     */
    virtual acd_bool getResolution(acd_uint& width, acd_uint& height) = 0;

    /**
     * Selects an Atila stream to be configured
     *
     * @param streamID stream identifier
     *
     * @returns the Atila stream identified by 'stream'
     *
     * @code
     *    // Example: Setting a data buffer for the stream 0 (without changing anything else)
     *    ACDDevice* a3ddev = getACDDevice();
     *    ACDBuffer* vtxPosition = a3ddev->getResourceManager()->createBuffer(bName, bSize, 0);
     *    // ...
     *    a3ddev->stream(0).setBuffer(vtxPosition);
     * @endcode
     */
    virtual ACDStream& stream(acd_uint streamID) = 0;

    /**
     * Enables a vertex attribute and associates it to a stream
     *
     * @param vaIndex Vertex attribute index accessible by the vertex shader
     * @param streamID Stream from where to stream vertex data
     */
    virtual void enableVertexAttribute(acd_uint vaIndex, acd_uint streamID) = 0;

    /**
     * Disables a vertex atributes ( remove the association with the attached stream )
     *
     * @param vaIndex Vertex attribute index to be disabled
     */
    virtual void disableVertexAttribute(acd_uint vaIndex) = 0;

    /**
     * Disables all vertex attributes (remove the association among all the attached streams to attributes)
     */
    virtual void disableVertexAttributes() = 0;

    /**
     * Gets the number of available streams
     *
     * @returns the available number of streams
     */
    virtual acd_uint availableStreams() const = 0;

    /**
     * Gets the number of available vertex inputs
     */
    virtual acd_uint availableVertexAttributes() const = 0;
                                                 
    /**
     * Sets the buffer from where to fetch indices
     *
     * @param ib The index buffer representing the index data to fecth
     * @param offset Offset in bytes from the start of the index buffer
     * @param indicesType type of the indices in the index buffer
     *
     */                       
    virtual void setIndexBuffer( ACDBuffer* ib,
                                 acd_uint offset,
                                 ACD_STREAM_DATA indicesType) = 0;

    /**
     * Select an Atila texture sampler to be configured
     *
     * @param samplerID the ACD sampler identifier
     *
     * @returns a reference to the Atila sampler identified by 'samplerID'
     *
     * @code
     *  
     *    // Example: Configuring the minification filter of the sampler 0
     *    a3ddev->sampler(0).setMinFilter(minFilter);   
     *
     * @endcode
     */
    virtual ACDSampler& sampler(acd_uint samplerID) = 0;

    /**
     * Draws a sequence of nonindexed primitives from the current set of Atila input streams
     *
     * @param start First vertex of the current set of Atila streams to fetch
     * @param count Number of vertices to be rendered
     * @param instances Number of instances of the passed primitives to draw.
     *
     */
    virtual void draw(acd_uint start, acd_uint count, acd_uint instances = 1) = 0;
        
    /**
     * Draws the specified primitives based on indexing
     * 
     * @param startIndex The number of the first index used to draw primitives, numbered from the start
     *                   of the index buffer.
     * @param indexCount Number of index used to drawn
     * @param minIndex The lowest-numbered vertex used by the call. baseVertexIndex is added before this
     *                 is used. So the first actual vertex used will be (minIndex + baseVertexIndex)
     * @param maxIndex The highest-numbered vertex used by the call. baseVertexIndex is added before this
     *                 is used. So the last actual vertex used will be (maxIndex + baseVertexIndex)
     * @param baseVertexIndex This is added to all indices before being used to look up a vertex buffer
     * @param instances Number of instances of the passed primitives to draw.
     *
     * @returns True if the method succeds, false otherwise
     *
     * @note set minIndex = 0 and maxIndex = 0 to ignore this parameters
     *
     *
     * @code
     *    // This call
     *
     *    atilaDevice->drawIndexed(
     *                    ACD_TRIANGLELIST, // ACDPrimitive
     *                     6, // startIndex
     *                     9, // indexCount,
     *                     3, // minIndex
     *                     7, // maxIndex
     *                    20 // baseVertexIndex
     *    );
     * 
     *    // with this buffer: {1,2,3, 3,2,4, 3,4,5, 4,5,6, 6,5,7, 5,7,8}
     *
     *    // It will draw 3 triangles (indexCount = 9), starting with index number 6 (startIndex). So
     *    // it ignores the first two and the last one triangles in the index buffer and only uses these:
     *    // {3,4,5, 4,5,6, 6,5,7} (you have told that you are using only indeces in the range [3,7] OK)
     *
     *    // Then the baseVertexIndex is added to the indices before the vertices are fetched from the
     *    // vertex buffer. So the vertex actually used will be: {23,24,25, 24,25,26, 26,25,27}
     * 
     * @endcode
     */
    virtual void drawIndexed( acd_uint startIndex, 
                                  acd_uint indexCount,
                                  acd_uint minIndex,
                                  acd_uint maxIndex, 
                                  acd_int baseVertexIndex = 0,
                                  acd_uint instances = 1) = 0;

    /**
     * Creates a RenderTarget of a resource to access resource data
     * 
     * @param resource Pointer to the resource that contains the data
     * @param rtDescription Pointer to a render target description. Set this parameter to NULL
     *                      to create a render target that subresource 0 of the entire
     *                      resource (using the format the resource was created with)
     *
     * @code
     *
     * // Create a buffer
      * ACDBuffer* buffer = a3ddev->getResourceManager()->createBuffer(bName, bSize, bUsage );
     * // Define a render target description (48 = skip two elements, start from element at pos 2)
     * ACD_RT_DESC rtDesc = { ACD_FORMAT_UNKNOWN, ACD_RT_DIMENSION_BUFFER, { 48, 24 } };
     * // Create a render target using buffer and the render target description
     * ACDRenderTarget* rt = a3ddev->createRenderTarget(buffer, rtDesc);
     *
     * @endcode
     */
    //virtual ACDRenderTarget* createRenderTarget( ACDResource* resource, const ACD_RT_DESC * rtDescription ) = 0;
    virtual ACDRenderTarget* createRenderTarget( ACDTexture* resource, const ACD_RT_DIMENSION rtdimension, ACD_CUBEMAP_FACE face, acd_uint mipmap ) = 0;

    /**
     * Bind one or more render targets and the depth-stencil buffer to the ROP stage
     *
     * @param numRenderTargets Number of render targets to bind
     * @param renderTargets Pointer to an array of render targets to bind to the device
     * @param zStencilTarget Pointer to a depth-stencil target to bind to the device.
     *                           If NULL, the depth-stencil state is not bound
     *
     * @see createRenderTarget() 
     * @see createZStencilTarget()
     *
     * ACDTexture2D* tex2D = ...
     *
     * // Define a render target description & create the render target object
     * ACD_RT_DESC rtDesc = { ACD_FORMAT_R8G8B8A8_UINT, ACD_RT_DIMENSION_TEXTURE_2D, {0} };
     * ACDRenderTarget* rtTex2D = a3ddev->createRenderTarget(text2D, rtDesc);
     *
     * // Set the render target
     * a3ddev->setRenderTargets( 1, rtTex2D, 0); // do not use a stencil buffer
     */
    virtual acd_bool setRenderTargets( acd_uint numRenderTargets,
                                   const ACDRenderTarget* const * renderTargets,
                                   const ACDZStencilTarget* zStencilTarget ) = 0;


    /**
     *
     *  Binds the specified render target as the current render target (back/front buffer).
     *
     *  @param renderTarget Pointer to a render target.
     *
     *  @return If the render target was attached correctly.
     *
     */
     
    virtual acd_bool setRenderTarget(acd_uint indexRenderTarget, ACDRenderTarget *renderTarget) = 0;
    
    
    /**
     *
     *  Binds the specified render target as the current depth stencil buffer.
     *
     *  @param renderTarget Pointer to a render target.
     *
     *  @return If the render target was correctly attached.
     *
     */
     
    virtual acd_bool setZStencilBuffer(ACDRenderTarget *renderTarget) = 0;
    
    /**
     *
     *  Get a pointer to the current render target.
     *
     *  @return A pointer to the current render target.
     *
     */
     
    virtual ACDRenderTarget *getRenderTarget(acd_uint indexRenderTarget) = 0;
    
    /**
     *
     *  Get a pointer to the current z stencil buffer.
     *
     *  @return A pointer to the current z stencil buffer.
     *
     */
     
     virtual ACDRenderTarget *getZStencilBuffer() = 0;
    
    /**
     * Copies data from user space memory to a resource
     *
     * This method copies data from user space to a subresource of a resource
     *
     * @param destResource A pointer to the destination resource
     * @param destSubResource A zero-based index identifying the subresource to update
     * @param destRegion The portion of the destination subresource to copy the data into.
     *                   Coordinates are always in bytes. If NULL, the data is written 
     *                   to the destination resource
     *
     * @param srcData pointer to the user space buffer containing the data
     * @param srcRowPitch size in bytes of a row (not used in 1D textures)
     * @param srcDepthPitch size in bytes of a slice (only for 3D textures)
     *
     * @code
     *
     *    ACDBuffer* b = a3ddev->getResourceManager()->createBuffer(...);
     *    ...
     *
     * @endcode
     */
    virtual void updateResource( const ACDResource* destResource,
                                 acd_uint destSubResource, 
                                 const ACD_BOX* destRegion,
                                 const acd_ubyte* srcData,
                                 acd_uint srcRowPitch,
                                 acd_uint srcDepthPitch ) = 0;

    /**
     * Swaps back buffer into front buffer (displays the frame just rendered)
     */
    virtual acd_bool swapBuffers() = 0;

    /**
     * Clears the color buffer using the RGBA color passed as parameter
     */
    virtual void clearColorBuffer(acd_ubyte red, acd_ubyte green, acd_ubyte blue, acd_ubyte alpha) = 0;

    /**
     * Clears the Z-buffer and the Stencil buffer using
     */
    virtual void clearZStencilBuffer( acd_bool clearZ,
                                      acd_bool clearStencil,
                                      acd_float zValue,
                                      acd_int stencilValue ) = 0;

    /**
     * Sets all the elements in a render target to one value
     *
     * @param rTarget The render target to clear
     * @param red Red intensity component, ranging from 0 to 255
     * @param green Green intensity component, ranging from 0 to 255
     * @param blue Blue intensity component, ranging from 0 to 255
     * @param alpha Alpha components, ranging from 0 to 255
     */
    virtual void clearRenderTarget(ACDRenderTarget* rTarget, 
                                   acd_ubyte red, acd_ubyte green, acd_ubyte blue, acd_ubyte alpha) = 0;

    /**
     * Clears the Z and Stencil target
     *
     * @param zsTarget The Z & Stencil target to clear
     * @param clearZ Specify to clear the Z part of the Z&Stencil buffer
     * @param clearStencil Specify to clear the Stencil part of the Z&Stencil buffer
     * @param zValue The value used to clear the Z part of the Z&Stencil buffer
     * @param stencilValue The value used to clear the stencil part od the Z&Stencil buffer
     */
    virtual void clearZStencilTarget(ACDZStencilTarget* zsTarget,
                                     acd_bool clearZ,
                                     acd_bool clearStencil,
                                     acd_float zValue,
                                     acd_ubyte stencilValue ) = 0;



    /**
     *
     *  Copies data from a 2D texture mipmap to a render buffer.  The dimensions and format of the source
     *  mipmap and the destination render buffer must be the same.
     *
     *  @param sourceTexture Pointer to the source 2D texture.
     *  @param mipLevel Mip level in the source 2D texture to copy.
     *  @param destRenterTarget Pointer to the destination Render Buffer.
     *  @param preload Boolean flag that defines if the data is to be preloaded into GPU memory without
     *  consuming simulation cycles.
     *
     */
    virtual void copySurfaceDataToRenderBuffer(ACDTexture2D *sourceTexture, acd_uint mipLevel, ACDRenderTarget *destRenderTarget, bool preload) = 0;

    /**
     *
     *  Sets if color is converted from linear to sRGB space on color write.
     *
     */
     
    virtual void setColorSRGBWrite(acd_bool enable) = 0;
    
    /**
     * Gets the shader registers limitations
     */
    virtual void getShaderLimits(ACD_SHADER_TYPE type, ACD_SHADER_LIMITS* limits) = 0;

    /**
     * Creates a generic shader program that can be used as vertex/fragment/geometry shader program
     */
    virtual ACDShaderProgram* createShaderProgram() const = 0;

    /**
     * Set the current geometry shader
     */
    virtual void setGeometryShader(ACDShaderProgram* program) = 0;

    /**
     * Set the current vertex shader
     */
    virtual void setVertexShader(ACDShaderProgram* program) = 0;

    /**
     * Set the current fragment shader
     */
    virtual void setFragmentShader(ACDShaderProgram* program) = 0;

    /**
     * Set the vertex shader default value
     */
    virtual void setVertexDefaultValue(acd_float currentColor[4]) = 0;

    /**
     * Save a group of Atila states
     *
     * @param siIds List of state identifiers to save
     * @returns The group of states saved
     */
    virtual ACDStoredState* saveState(ACDStoredItemIDList siIds) const = 0;

    /**
     * Save the whole set of Atila states
     *
     * @returns The whole states group saved
     */
    virtual ACDStoredState* saveAllState() const = 0;

    /**
     * Restores the value of a group of states
     *
     * @param state a previously saved state group to be restored
     */
    virtual void restoreState(const ACDStoredState* state) = 0;

    /**
     * Releases a ACDStoredState object
     *
     * @param state a previously saved state group to release
     */
    virtual void destroyState(ACDStoredState* state) = 0;

    /**
     * Sets the starting frame from which to track current frame and batch.
     *
     * @param startFrame The starting frame.
     */
    virtual void setStartFrame(acd_uint startFrame) = 0;

    /**
     * Dumps the current device state (Attila Library state)
     */
    virtual void DBG_dump(const acd_char* file, acd_enum flags) = 0;

    /**
     * Dumps the future device state when the specified frame and batch event 
     * occurs, and just before drawing the batch.
     */
    virtual void DBG_deferred_dump(const acd_char* file, acd_enum flags, acd_uint frame, acd_uint batch) = 0;

    /**
     * Saves a file with an image of the current Atila library state
     */
    virtual acd_bool DBG_save(const acd_char* file) = 0;

    /**
     * Restores the Atila state from a image file previously saved
     */
    virtual acd_bool DBG_load(const acd_char* file) = 0;

    /**
     * Set the alphaTest. ONLY for activing earlyZ, is compulsory to active alphaTest in the ACDX!!!
     */
    virtual void alphaTestEnabled(acd_bool enabled) = 0;


    virtual ACDRenderTarget* getFrontBufferRT() = 0;

    virtual ACDRenderTarget* getBackBufferRT() = 0;

    virtual void copyMipmap (   ACDTexture* inTexture, 
                            acdlib::ACD_CUBEMAP_FACE inFace, 
                            acd_uint inMipmap, 
                            acd_uint inX, 
                            acd_uint inY, 
                            acd_uint inWidth, 
                            acd_uint inHeight, 
                            ACDTexture* outTexture, 
                            acdlib::ACD_CUBEMAP_FACE outFace, 
                            acd_uint outMipmap, 
                            acd_uint outX, 
                            acd_uint outY,
                            acd_uint outWidth,
                            acd_uint outHeight,
                            acdlib::ACD_TEXTURE_FILTER minFilter,
							acdlib::ACD_TEXTURE_FILTER magFilter) = 0;

    virtual void performBlitOperation2D(acd_uint sampler, acd_uint xoffset, acd_uint yoffset,
                                        acd_uint x, acd_uint y, acd_uint width, acd_uint height,
                                        acd_uint textureWidth, ACD_FORMAT internalFormat,
                                        ACDTexture2D* texture, acd_uint level) = 0;

    /**
     * Compress a given texture into a given format
     *
     * @param originalFormat Format from the given texture
     * @param compressFormat Destiny format for the conversion
     * @param width Texture width
     * @param height Texture height
     * @param originalData Texture data pointer
     *
     * @return Compressed texture data into compressFormat
     */
    virtual acd_ubyte* compressTexture(ACD_FORMAT originalFormat, ACD_FORMAT compressFormat, acd_uint width, acd_uint height, acd_ubyte* originalData, acd_uint selectionMode) = 0;

    /**
     *  Obtain which frame is being executed by the ACD 
     *
     * @return Frame which is being executed by the ACD
     * 
     */
    virtual acd_uint getCurrentFrame() = 0;

    /**
     *  Obtain which batch is being executed by the ACD 
     *
     * @return Batch which is being executed by the ACD
     * 
     */
    virtual acd_uint getCurrentBatch() = 0;
         
};

} // namepace acdlib

#endif // ACD_DEVICE
