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

#ifndef ACDX_CODESNIP_H
    #define ACDX_CODESNIP_H

#include <vector>
#include <string>

namespace acdlib
{

class ACDXCodeSnip
{
private:

    std::string text;

protected:

    void setCode(const std::string& code);

public:    
    
    // final (no virtual)
    ACDXCodeSnip& append(const ACDXCodeSnip& cn);
    ACDXCodeSnip& append(std::string instr);
    ACDXCodeSnip& clear();
    void removeComments();
    
    virtual void dump() const;
    virtual std::string toString() const;
    virtual ~ACDXCodeSnip();
};

} // namespace acdlib


#endif // ACDX_CODESNIP_H
