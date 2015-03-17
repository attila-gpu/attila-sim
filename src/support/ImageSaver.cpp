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
 * ImageSaver implementation file.
 *
 */


/**
 *
 *  @file ImageSaver.cpp
 *
 *  This file contains the implementation of the ImageSaver class.
 *
 */

#include "ImageSaver.h"

#ifdef WIN32
    #include <windows.h>
    #include <gdiplus.h>
#else
    #include <png.h>
#endif

#include <cstdlib>

namespace gpu3d
{

ImageSaver::ImageSaver()
{
#ifdef WIN32
    static Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID clsid;
    
    u32bit num = 0;             
    u32bit size = 0;
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    // Get the number of image encoders and the size of the image encoder array in bytes.
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        panic("ImageSaver", "ImageSaver", "Error obtaining PNG encoder.");

    pImageCodecInfo = (Gdiplus::ImageCodecInfo *) new u8bit[size];
    if (pImageCodecInfo == NULL)
        panic("ImageSaver", "ImageSaver", "Error obtaining PNG encoder.");

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    bool found = false;
    for(u32bit c = 0; c < num && !found; ++c)
    {
        if (wcscmp(pImageCodecInfo[c].MimeType, L"image/png") == 0)
        {
            clsid = pImageCodecInfo[c].Clsid;
            found = true;
        }    
    }
    
    delete [] pImageCodecInfo;

    encoderClsid = new u8bit[sizeof(CLSID)];
    memcpy(encoderClsid, &clsid, sizeof(CLSID));
    
    if (!found)
        panic("ImageSaver", "ImageSaver", "Error obtaining PNG encoder.");
#endif
}



ImageSaver::~ImageSaver()
{
#ifdef WIN32
    Gdiplus::GdiplusShutdown(gdiplusToken);
    delete[] encoderClsid;
#endif
}

ImageSaver &ImageSaver::getInstance()
{
    static ImageSaver *imageSaver = NULL;
    
    if (imageSaver == NULL)
        imageSaver = new ImageSaver;
    
    return *imageSaver;
}

void ImageSaver::savePNG(char *filename, u32bit xRes, u32bit yRes, u8bit *data)
{
#ifdef WIN32

    WCHAR filenameW[256];

    swprintf(filenameW, 255, L"%S.png", filename);

    CLSID clsid;
    memcpy(&clsid, encoderClsid, sizeof(CLSID));
    
    Gdiplus::Status status;
    
    Gdiplus::Bitmap *frameBitmap = new Gdiplus::Bitmap(xRes, yRes, xRes * 4, PixelFormat32bppARGB, (BYTE *) data);
    status = frameBitmap->Save(filenameW, &clsid, NULL);
    
    if (status != Gdiplus::Ok)
        panic("ImageSaver", "savePNG", "Error saving image to PNG file.");

    delete frameBitmap;

#else

    FILE *fout;
    png_structp png_ptr;
    png_infop info_ptr;
    png_byte **row_pointers;

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    //  Add file extension.
    char filenameAux[256];
    sprintf(filenameAux, "%s.png", filename);
    
    //  Open/Create the file for the current frame.
    fout = fopen(filenameAux, "wb");

    //  Check if the file was correctly created.
    GPU_ASSERT(
        if (fout == NULL)
            panic("bGPU-emu", "dumpFrame", "Error creating frame color output file.");
    )
    
    /*    
    png_init_io(png_ptr, fout);
    png_set_compression_level(png_ptr, 9);
    
    png_set_IHDR(png_ptr, info_ptr, xRes, yRes,
		 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
        
    png_set_bgr(png_ptr);
    
    row_pointers = (png_byte **)malloc(yRes * sizeof(png_byte*));

    for (int i = 0; i < yRes; i++)
	    row_pointers[i] = data + (xRes * 4) * i;

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    */
    
    free(row_pointers);

    fclose(fout);

#endif    
}

}   // namespace gpu3d


