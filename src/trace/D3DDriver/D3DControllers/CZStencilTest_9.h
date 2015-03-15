#ifndef CZSTENCILTEST_9_H_INCLUDED
#define CZSTENCILTEST_9_H_INCLUDED

/**
    Keeps Z/Stencil tests gpu state consistent with d3d.
*/
class CZStencilTest9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);
private:
    StencilUpdateFunction get_update_func(D3DSTENCILOP op);
    CompareMode get_compare(D3DCMPFUNC cmp);
};

#endif
