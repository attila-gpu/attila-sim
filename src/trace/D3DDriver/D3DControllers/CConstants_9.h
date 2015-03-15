#ifndef CCONSTANTS9_H_INCLUDED
#define CCONSTANTS9_H_INCLUDED

/**
    Keep gpu vertex and pixel shader constants updated
    Add to device state
*/
class CConstants9 : public StateDataNodeController,
    public ResourceAssignationTableObserver {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    void on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource);
    void on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource);
private:
    set< UsageId > assigned_vs_constants;
    set< UsageId > assigned_ps_constants;
    StateDataNode *s_device;

    void update_constant(StateDataNode* s_constant);
};

#endif // CCONSTANTS9_H_INCLUDED

