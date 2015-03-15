#include "Common.h"
#include "IRoot_9.h"
#include "../AD3D9/AIRoot_9.h"
#include "D3DInterface.h"

void D3DInterface::initialize(bool _useACD) {
    D3D_DEBUG( cout << "D3DInterface: Initializing" << endl; )

    acd = _useACD;

    if (acd)
        ai_root_9 = new AIRoot9();
    else
        i_root_9 = new IRoot9(D3DState::get_root());
}

void D3DInterface::finalize() {
    D3D_DEBUG( cout << "D3DInterface: Finalizing" << endl; )

    if (acd)
        delete ai_root_9;
    else
        delete i_root_9;
}

IRoot9* D3DInterface::get_root_9() {
    return i_root_9;
}

AIRoot9* D3DInterface::get_acd_root_9() {
    return ai_root_9;
}

bool D3DInterface::useACD() {
    return acd;
}

D3DInterface::D3DInterface() {}

IRoot9* D3DInterface::i_root_9 = 0;
AIRoot9* D3DInterface::ai_root_9 = 0;
bool D3DInterface::acd = false;

