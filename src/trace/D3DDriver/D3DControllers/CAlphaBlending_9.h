#ifndef CALPHABLENDING_9_H_INCLUDED
#define CALPHABLENDING_9_H_INCLUDED

/**
    Manages alpha blending state, as name suggests
*/
class CAlphaBlending9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);
private:
    CompareMode get_compare(DWORD func);
    void update_blend_functions(DWORD render_state, DWORD value, bool separate);
    void update_blend_op(DWORD state, DWORD value, bool separate);
    void get_blend_functions(DWORD function, BlendFunction* color_function, BlendFunction* alpha_function);
    BlendEquation get_blend_equation(DWORD op);

};

#endif
