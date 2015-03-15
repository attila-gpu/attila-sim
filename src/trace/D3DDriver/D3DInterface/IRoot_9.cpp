#include "Common.h"
#include "IDirect3DImp_9.h"
#include "IRoot_9.h"

IRoot9::IRoot9(StateDataNode* s_root):state(s_root) {
}

IRoot9::~IRoot9() {
    set< IDirect3DImp9* > :: iterator it;
    for(it = i_childs.begin(); it != i_childs.end(); it ++)
        delete *it;
}

IDirect3D9* IRoot9::Direct3DCreate9(UINT SDKVersion) {
     D3D_DEBUG( cout << "IROOT9: Direct3DCreate9" << endl; )

    /// @note SDKVersion is ignored from here on
    IDirect3DImp9* d3d = new IDirect3DImp9(state);
    i_childs.insert(d3d);
    return d3d;
}






