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

#ifndef SHPROGRAMINFO_H
    #define SHPROGRAMINFO_H

#include <string>
#include <map>
#include <list>

#ifndef GL_VERTEX_PROGRAM_ARB
    #define GL_VERTEX_PROGRAM_ARB 0x8620
#endif

#ifndef GL_FRAGMENT_PROGRAM_ARB
    #define GL_FRAGMENT_PROGRAM_ARB 0x8804
#endif

/** 
 * This class represents statistical information of a shader program
 *
 * The current version stores only the total number of instructions
 * and the specific number of texture load instructions
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @ver 1.0
 * @date 6-7-2005
 */
class ShProgramInfo
{
private:

    friend class ShProgramManager; ///< Allows using the initializer constructor
    
    unsigned int name; ///< Program name (identifier)
    std::string source; ///< Source ASM code
    int nInstr; ///< Number of instructions in the shader
    int nTexInstr; ///< Number of texture load instructions in the shader
    unsigned int type; ///< Kind of program: GL_VERTEX_PROGRAM_ARB or GL_FRAGMENT_PROGRAM_ARB

    ShProgramInfo(unsigned int name, unsigned int type);

public:    

    /**
     * Add source code to this program via "glProgramString"
     *
     * @param source new source code for this program
     */
    void setSource(const std::string& source);

    unsigned int getName() const;
    int countInstructions() const;
    int countTextureLoads() const;
    float textureLoadsRatio() const;
    const std::string& getSource() const;
    unsigned int getType() const;
};

/**
 * This class is a singleton class that contains all ShProgramInfo created
 *  ... is the only way to create ShProgramInfo objects
 *  ... allows to compute average metrics for all the ShProgramInfo created
 *        (for example, compute the average of instructions in a program shader)
 *
 * @author Carlos González Rodríguez
 * @ver 1.0
 * @date 6-7-2005
 */
class ShProgramManager
{
private:

    ///< Created programs up to now
    std::map<unsigned int, ShProgramInfo*> progs;
    
    ///< Maintains the order of programs based on their first bind call
    std::list<unsigned int> progOrder;

    ShProgramInfo* vsh; ///< Current bound vertex shader
    ShProgramInfo* fsh; ///< Current bound fragment shader

    ///< Protect creation and copy
    ShProgramManager();
    ShProgramManager(const ShProgramManager&);
    ShProgramManager& operator=(const ShProgramManager&);

    /**
     * Finds a program with name 'name' if exists
     *
     * @param name program name
     * @return if the program exists, returns the program. If not, returns 0
     */
    ShProgramInfo* findProgram(unsigned int name) const;

public:

    static ShProgramManager& instance();

    /**
     * Finds a program with name 'name' and type 'type', if the program name does not exist
     * creates a new one with (name,type). If type is not correct this function panics.
     *
     * The program is selected as current vertex shader or fragment shader based on its type
     *
     * @param name program name, if name is 0 then any program is bound for this type
     * @param type program type
     *
     * @return true if the program is created, false if the program existed previously
     *
     */
    bool bindProgram(unsigned int name, unsigned int type);

    /**
     * Returns the current vertex or fragment shader or 0 if there is no current
     *
     * @param type program type for which we want the current
     * @return the current program for type 'type' or 0 if there is no current
     */
    ShProgramInfo* getCurrent(unsigned int type) const;

    /**
     * Computes the average number of instructions in the shader programs
     *
     * @param type Allows to compute the average based just on one type of program shaders
     *
     * @code
     *
     *    // Example: (compute the average number of instructions within fragment shaders)
     * 
     *    ShProgramManager& spm = ShProgramManager::instance();
     *    ...
     *    float avgInstrFSh = spm.getAverageInstructions(GL_FRAGMENT_SHADER_PROGRAM_ARB);
     *
     * @endcode
     *
     * @note type = 0 computes averages for all shaders ignoring their types
     */
    float getAverageInstructions(unsigned int type = 0) const;
    
    /**
     * Computes the average number of texture load instructionns in the shader programs
     *
     * @param type Allows to compute the average based just on one type of program shaders
     *
     * @note type = 0 computes averages for all shaders ignoring their types
     */
    float getAverageTextureLoads(unsigned int type = 0) const;

    /**
     * Outputs statistics in a file
     */
    bool writeInfo(std::string file);
};


#endif // SHPROGRAMINFO_H
