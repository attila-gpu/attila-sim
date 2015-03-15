#ifndef CRASTERIZATION_9_H_INCLUDED
#define CRASTERIZATION_9_H_INCLUDED

/**
    Keeps GPU rasterizing state consistent with D3D rasterizing state:
        Interpolation.
        Wich GPU vertex shader input registers are passed to GPU
        fragment shader input registers.

*/
class CRasterization9: public StateDataNodeController, public ResourceAssignationTableObserver {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    void on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource);
    void on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource);
private:
    set< UsageId > assigned_vs_output;
    set< UsageId > assigned_ps_input;
};

#endif

