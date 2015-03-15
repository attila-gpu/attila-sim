#ifndef CSURFACES_9_H_INCLUDED
#define CSURFACES_9_H_INCLUDED

class CLockRect9;

/**
    Manages gpu memory for surfaces.
    Add to the DEVICE node.
*/
class CSurfaces9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CSurfaces9();
    ~CSurfaces9();
private:
    CLockRect9* c_lockrect;
    UINT get_surface_size(UINT width, UINT height, D3DFORMAT format, DWORD usage);

    u32bit scan_width;
    u32bit scan_height;
    u32bit overscan_width;
    u32bit overscan_height;
};

#endif // CSURFACES_9_H_INCLUDED
