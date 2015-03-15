#ifndef CSAMPLERS_9_H_INCLUDED
#define CSAMPLERS_9_H_INCLUDED


/**
    Keeps GPU texture registers configured according to
    D3D samplers, if they are assigned to it.

    For not supported features or no d3d texture
    setted to a d3d sampler it configures texture
    units with a default texture.

    Add to the DEVICE_9 state node.
*/
class CSamplers9 : public StateDataNodeController,
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
    bool is_sampler_assigned(UINT sampler);
    u32bit get_assigned_texture_unit(UINT sampler);

    void reset_texture(u32bit texture_unit);
    u32bit default_texture;

    void update_texture(StateDataNode* s_texture, u32bit texture_unit);
    void update_addressing(StateDataNode* s_sampler, u32bit texture_unit);
    void update_filtering(StateDataNode* s_sampler, u32bit texture_unit);

    TextureFormat get_texture_format(D3DFORMAT format);
    ClampMode get_clamp(DWORD addressing);

    StateDataNode *s_device;
};

#endif // CSAMPLERS_9_H_INCLUDED
