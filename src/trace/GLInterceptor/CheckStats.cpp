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

#include "CheckStats.h"

using namespace std;

FPAndAlphaStat::FPAndAlphaStat(const string& name) : GLIStat(name), 
    fp(false), alpha(false),count(0)
{}

void FPAndAlphaStat::setFPFlag(bool enable)
{
    fp = enable;
}

void FPAndAlphaStat::setAlphaFlag(bool enable)
{
    alpha = enable;
}

int FPAndAlphaStat::getBatchCount()
{
    if ( fp && alpha )
    {
        count++;
        return 1;
    }
    return 0;
}

int FPAndAlphaStat::getFrameCount()
{
    int temp = count;
    count = 0;
    return temp;
}
