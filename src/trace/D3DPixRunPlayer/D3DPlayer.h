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

/**************************************************
Interface for execute calls to D3D stored in a
trace file.

Author: José Manuel Solís
**************************************************/

#ifndef __D3DPLAYER
#define __D3DPLAYER

class D3DPlayer  {
public:
    /********************************************
    Open a trace file

    Pre: Previous opened file has been closed
    ********************************************/
    virtual void open(std::string filename) = 0;

    /********************************************
    Closes a trace file

    Pre: File opened
    ********************************************/
    virtual void close()  = 0;

    /*******************************************
    Plays calls until a present operation is
    found or no calls left.

    Pre: File opened
    *******************************************/
    virtual bool playFrame() = 0;


    /*******************************************
    Plays one batch

    Pre: File opened
    *******************************************/
    virtual bool playBatch() = 0;

    /*******************************************
    Returns current offset in calls from the
    beggining of the trace.

    Pre: File opened
    *******************************************/
    virtual unsigned long getCallOffset() = 0;
    
    /**
     *
     *  Returns a pointer to the current IDirect3DDevice9.
     *
     */

    virtual void *getDirect3DDevice() = 0;
    
    /**
     *
     *  Returns if a frame was rendered.
     *
     */
     
    virtual bool isEndOfFrame() = 0;
};


/*******************************************
Creates a D3D9 player. For a Win32 build  window
must be a valid handle to a initialized window.
*******************************************/
D3DPlayer *createD3D9Player(HWND window);

#endif
