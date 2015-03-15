/**************************************************************************
 *
 * Copyright (c) 2002, 2003 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: Drawer.h,v $
 * $Revision: 1.6 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:34 $
 *
 * Drawer class definition file. 
 *
 */

#ifndef _DRAWER_

#define _DRAWER_

#include "GPUTypes.h"
#include "QuadFloat.h"
#include "GPU.h"
#include "glut.h"

/***  Default initial window x position.  */
#define DEFAULT_WIN_X_POSITION 100

/***  Default initial window y position.  */
#define DEFAULT_WIN_Y_POSITION 100

/***  Max number of batches in a frame.  */
#define MAX_BATCHES 500

/*** Max number of vertexs in a frame.  */
#define MAX_VERTEXS 500000

/***  This structure defines a batch of vertexs.  */
typedef struct Batch
{
    PrimitiveMode primitive;
    u32bit start;
    u32bit count;
};

/**
 *
 *  OpenGL wrapper for the fake rasterizer box.
 *
 *  Implements functions to rasterizer polygons using
 *  OpenGL.
 *
 */
 
class Drawer {

private:

	static QuadFloat* vertex[];     /**<  A buffer for the current frame vertex positions.  */
	static QuadFloat* color[];      /**<  A buffer for the current frame vertex colors.  */
	static QuadFloat* oldVertex[];  /**<  A buffer for the previous frame vertex positions.  */
    static QuadFloat* oldColor[];   /**<  A buffer for the previous frame vertex colors.  */
    
    static Batch batch[];           /**<  A buffer for the current frame vertex batches.  */
    static Batch oldBatch[];        /**<  A buffer for the previous frame vertex batches.  */
    
	static u32bit vertexCounter;    /**<  Number of vertexs in the current frame.  */
    static u32bit batchCounter;     /**<  Number of batches in the current frame.  */
    static u32bit oldBatchCounter;  /**<  Number of batches for the previous frame.  */
    static u32bit oldVertexCounter; /**<  Number of vertexes fro the previous frame.  */
    static u32bit windowID;         /**<  Current window identifier.  */
    
    /**
     *
     *  Drawer constructor.
     *
     *  Drawer is an static class so the constructor function is
     *  hidden.
     *
     */
    
	Drawer();

    /**
     *
     *  Drawer copy.
     *
     *  Drawer is an static class so the copy function is
     *  hidden.
     *
     */

	Drawer( const Drawer& );

    /**
     *
     *  Display function.
     *
     */
     
    static void display();
    
public:

    /**
     *
     *  Initializes the GLUT rasterizer.
     *
     */
     
    static void init();
    
    /**
     *
     *  Sets the GLUT idle function.
     *
     *  @param func The function to call from the idle loop.
     *
     */
     
    static void setIdleFunction(void (*func)(void));

    /**
     *
     *  Starts the Drawer main loop.
     *
     */
     
    static void start();

    /**
     *
     *  Creates a new GLUT window.
     *
     *  @param w Window width.
     *  @param h Window height.
     *
     */
         
    static void createWindow(u32bit w, u32bit h);
    
    /**
     *
     *  Destroys the current GLUT window.
     *
     */
     
    static void destroyWindow();
    
    /**
     *
     *  Sets the current clear color.
     *
     *  @param color The clear color for the color buffer.
     *
     */
     
    static void setClearColor(u32bit color);
    
    /**
     *
     *  Sets the current clear z value.
     *
     *  @param zvalue Clear z value for the z buffer.
     *
     */
     
    static void setClearZ(u32bit zvalue);
    
    /**
     *
     *  Sets the current clear stencil value.
     *
     *  @param val Clear stencil value for the stencil buffer.
     *
     */
     
    static void setClearStencil(s8bit val);
    
    /**
     *
     *  Sets the viewport.
     *
     *  @param iniX Initial viewport x position.
     *  @param iniY Initial viewport y position.
     *  @param width Viewport width.
     *  @param height Viewport height.
     *
     */
     
    static void viewport(s32bit iniX, s32bit iniY, u32bit width, u32bit height);
    
    /**
     *
     *  Sets orthogonal projection matrix and mode.
     *
     *  @param left Left clip plane.
     *  @param right Right clip plane.
     *  @param bottom Bottom clip plane.
     *  @param top Top clip plane.
     *  @param near Near clip plane.
     *  @param far Far clip plane.
     *
     */
     
    static void setOrthogonal(f32bit left, f32bit right, f32bit bottom, f32bit top,
        f32bit near, f32bit far);

    /**
     *
     *  Sets the perspective projection frustum matrix and mode.
     *
     *  @param left Left frustum plane.
     *  @param right Right frustum plane.
     *  @param bottom Bottom frustum plane.
     *  @param top Top frustum plane.
     *  @param near Near frustum plane.
     *  @param far frustum plane.
     *
     */
     
    static void setPerspective(f32bit left, f32bit right, f32bit bottom, f32bit top,
        f32bit near, f32bit far);
      
    
    /**
     *
     *  Sets the face culling mode.
     *
     *  @param cullMode The face culling mode.
     *
     */
     
    static void setCulling(CullingMode cullMode);
    
    /**
     *
     *  Sets the shading mode.
     *
     *  @param shading Smooth shading mode enabled?
     *
     */
     
    static void setShading(bool shading);
    
    /**
     *
     *  Clear the color buffer.
     *
     */
     
    static void clearColorBuffer();
    
    /**
     *
     *  Swap front and back buffers.
     *
     */

    static void swapBuffers();

    /**
     *
     *  Set current primitive.
     *
     *  @param primitive The current primitive.
     *
     */
         
    static void setPrimitive(PrimitiveMode primitive);
   
    /**
     *
     *  Draw a vertex.
     *
     *  @param position The vertex position.
     *  @param color The vertex color.
     *
     */
      
    static void drawVertex(QuadFloat *position, QuadFloat *color);
   
};


#endif.
