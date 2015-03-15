#include "Common.h"
#include "CRoot.h"
#include "D3DControllers.h"

CRoot *D3DControllers::croot;

void D3DControllers::initialize() {
    D3D_DEBUG( cout << "D3DControllers: Initializing" << endl; )
    croot = new CRoot();
    D3DState::get_root()->add_controller(croot);

}

void D3DControllers::finalize() {
    D3D_DEBUG( cout << "D3DControllers: Finalizing" << endl; )
    D3DState::get_root()->remove_controller(croot);
    delete croot;
}

CRoot* D3DControllers::get_root() {
    return croot;
}


D3DControllers::D3DControllers() {}

