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

#ifndef ACDX_SYMBOLTABLE_H
    #define ACDX_SYMBOLTABLE_H

#include <map>
#include <string>
#include <utility>
#include <stack>
#include "support.h"

namespace acdlib
{

/**
 * class implementing the assembler symbol table to manage identifiers.
 *
 * @note Implemented using templates
 *
 * @author Jordi Roca Monfort - jroca@ac.upc.es
 * @date 16/7/2004
 * @ver 1.0
 *       
 */

template <typename T>
class ACDXSymbolTable
{

private:

    /**
     * class implementing a block of the symbol table.
     *
     * Blocks are managed as stack elements.
     */

    template <typename P>
    class SymbolTableBlock
    {

    private:

        std::map<std::string,P> symbolMap;  ///< Hashing table for symbols
        
    public:

        P searchSymbol(char *id, bool &found) const
        {
            return searchSymbol(std::string(id),found);
        }
        
        P searchSymbol(std::string id, bool& found) const
        { 
            typename std::map<std::string,P>::const_iterator it = symbolMap.find(id);
            if ( it != symbolMap.end() )
            {
                found = true;
                
                return it->second;
            }
            found = false;
            return 0;
        }

        void insertSymbol(char *id,const P& info)
        {
            insertSymbol(std::string(id),info);
        }

        void insertSymbol(std::string id,const P& info)
        {
            symbolMap.insert(std::make_pair(id,info));
        }
    };

    std::stack<SymbolTableBlock<T>* > blockStack;

public:

    void pushBlock()
    {
        blockStack.push(new SymbolTableBlock<T>);
    }

    void popBlock()
    {
        if (blockStack.empty())
            panic("ACDXSymbolTable","popBlock()","Empty Stack!");

        delete blockStack.top();
        
        blockStack.pop();
    }

    T searchSymbol(std::string id, bool& found)
    {
        if (blockStack.empty())
            panic("ACDXSymbolTable","popBlock()","Empty Stack!");
            
        return blockStack.top()->searchSymbol(id,found);
    }

    T searchSymbol(std::string id)
    {
        bool found;
        return searchSymbol(id,found);
    }
    
    void insertSymbol(std::string id, const T& info)
    {
        if (blockStack.empty())
            panic("ACDXSymbolTable","popBlock()","Empty Stack!");
            
        blockStack.top()->insertSymbol(id,info);
    }
    
    ~ACDXSymbolTable()
    {
        while (!blockStack.empty())
        {
            delete blockStack.top();
            blockStack.pop();
        }
    }
};

} // namespace acdlib

#endif // ACDX_SYMBOLTABLE_H
