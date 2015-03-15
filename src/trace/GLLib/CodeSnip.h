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

#ifndef CODESNIP_H
    #define CODESNIP_H

#include <vector>
#include <string>

class CodeSnip
{
private:

    std::string text;

protected:

    void setCode(const std::string& code);

public:    
    
    // final (no virtual)
    CodeSnip& append(const CodeSnip& cn);
    CodeSnip& append(std::string instr);
    CodeSnip& clear();
    void removeComments();
    
    virtual void dump() const;
    virtual std::string toString() const;
    virtual ~CodeSnip();
};


#endif
