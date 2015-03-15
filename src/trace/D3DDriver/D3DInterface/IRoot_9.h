#ifndef IROOT_9_H
#define IROOT_9_H

class IDirect3DImp9;

/**
    Root interface for D3D9:
        Owner of IDirect3DImp9 objects.
        Implements some D3D9 functions.
*/
class IRoot9 {
public:
    IRoot9(StateDataNode* s_root);
    ~IRoot9();

    IDirect3D9* Direct3DCreate9(UINT SDKVersion);
private:
    StateDataNode *state;
    set< IDirect3DImp9* > i_childs;
};

#endif

