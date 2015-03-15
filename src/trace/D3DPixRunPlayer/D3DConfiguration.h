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

/*****************************************************************************
Singleton configuration class for d3dplayer

Author: Jos� Manuel Sol�s
*****************************************************************************/

#ifndef __D3DCONFIGURATION
#define __D3DCONFIGURATION

class D3DConfiguration {
    public:
        void load(std::string filename);
        std::string getValue(std::string section, std::string variable);
        bool existsVariable(std::string section, std::string variable);

        static D3DConfiguration &instance();
    private:
        IniFile ini;
};

#endif
