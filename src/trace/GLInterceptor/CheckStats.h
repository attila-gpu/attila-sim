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

#ifndef CHECKSTATS_H
    #define CHECKSTATS_H

#include "GLIStat.h"

/**
 * This statistic is used to check if a GL_ALPHA_TEST and GL_FRAGMENT_PROGRAM_ARB
 * are enabled at the same time
 */
class FPAndAlphaStat : public GLIStat
{
private:

    bool fp;
    bool alpha;
    int count; // times that fp && alpha where true during this frame

public:

    FPAndAlphaStat(const std::string& name);

    virtual int getBatchCount();

    virtual int getFrameCount();

    void setFPFlag(bool enable);

    void setAlphaFlag(bool enable);

};


#endif