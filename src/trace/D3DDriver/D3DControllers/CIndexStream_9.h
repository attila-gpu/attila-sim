#ifndef CINDEXSTREAM_9_H_INCLUDED
#define CINDEXSTREAM_9_H_INCLUDED

/**
    Reserves a gpu stream to use as index stream
    and keeps it updated.
    Add to the DEVICE node.
*/
class CIndexStream9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CIndexStream9();
    ~CIndexStream9();

private:
    bool assigned_index_stream;
    std::map< D3DFORMAT , StreamData > gpu_format;
    std::map< D3DFORMAT , DWORD> format_size;
};

#endif
