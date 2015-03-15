#ifndef CVSINPUT_9_H_INCLUDED
#define CVSINPUT_9_H_INCLUDED


/**
    Keeps GPU input registers pointing to the corresponding GPU streams, according to
    the D3D semantics assigned to them.
*/
class CVSInput9 : public StateDataNodeController,
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
    set< UsageId > assigned_streams;
    set< UsageId > assigned_registers;
};

#endif

