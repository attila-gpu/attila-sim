#ifndef D3DCONTROLLERS_H_INCLUDED
#define D3DCONTROLLERS_H_INCLUDED

class CRoot;

/**
    Manages SROOT controllers.
 */
class D3DControllers {
public:
    static void initialize();
    static void finalize();
    static CRoot* get_root();
private:
    D3DControllers();
    static CRoot *croot;
};

#endif

