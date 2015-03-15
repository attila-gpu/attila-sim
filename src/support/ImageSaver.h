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
 * ImageSaver definition file.
 *
 */


/**
 *
 *  @file ImageSaver.h
 *
 *  This file contains definitions and includes for the ImageSaver class.
 *
 */

#ifndef _IMAGESAVER_
#define _IMAGESAVER_

#include "GPUTypes.h"

namespace gpu3d
{

/**
 *
 *  Saves image data into picture files.
 *
 */

class ImageSaver
{
private:

#ifdef WIN32
    pointer gdiplusToken;    // ULONG_PTR is a 64-bit pointer
    u8bit* encoderClsid;
#endif

    /**
     *
     *  Constructor.
     *
     */

    ImageSaver();
    
    /**
     *
     *  Destructor.
     *
     */

    ~ImageSaver();
    
public:

    /**
     *
     *  Get the single object instantation of the ImageSaver class.
     *
     *  @return Reference to the single instantation of the ImageSaver class.
     *
     */
     
    static ImageSaver &getInstance();
    
    /**
     *
     *  Save image data to PNG file.
     *
     *  @param filename Name/path of the destination PNG file.
     *  @param xRes Horizontal resolution in pixels of the image.
     *  @param yRes Vertical resolution in pixels of the image.
     *  @param data Pointer to the image data array.
     *
     */
     
    void savePNG(char *filename, u32bit xRes, u32bit yRes, u8bit *data);
    
};

} // namespace gpu3d

#endif  // _IMAGESAVER_

