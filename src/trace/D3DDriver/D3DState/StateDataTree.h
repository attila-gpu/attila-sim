#ifndef STATEDATATREE_H_INCLUDED
#define STATEDATATREE_H_INCLUDED

class StateDataNode;


class StateDataNodeController {
public:
    /** Note child is already added when this event is fired. */
    virtual void on_add_node_child(StateDataNode* parent, StateDataNode* child) = 0;
    /** Note child is still added when this event is fired. */
    virtual void on_remove_node_child(StateDataNode* parent, StateDataNode* child) = 0;
    /** Notification of a write operation in node at given offset and size. */
    virtual void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) = 0;
    /** Notification of controller added as watcher */
    virtual void on_added_controller(StateDataNode*) = 0;
    /** Notification of controller removed as watcher */
    virtual void on_removed_controller(StateDataNode*) = 0;

    virtual ~StateDataNodeController() {}
};

//typedef std::string StateName;

enum Names
{
    NAME_NO_NAME,
    NAME_ROOT,
    NAME_DIRECT3D_9,
    NAME_STREAM,
    NAME_SOURCE,
    NAME_OFFSET,
    NAME_STRIDE,
    NAME_SAMPLER,
    NAME_TEXTURE,
    NAME_ADDRESSU,
    NAME_ADDRESSV,
    NAME_ADDRESSW,
    NAME_MAG_FILTER,
    NAME_MIN_FILTER,
    NAME_MIP_FILTER,
    NAME_MAX_ANISOTROPY,
    NAME_DEVICE_9,
    NAME_STREAM_COUNT,
    NAME_SAMPLER_COUNT,
    NAME_FVF,
    NAME_CURRENT_VERTEX_DECLARATION,
    NAME_CURRENT_INDEX_BUFFER,
    NAME_CURRENT_VERTEX_SHADER,
    NAME_CURRENT_PIXEL_SHADER,
    NAME_VS_CONSTANT,
    NAME_PS_CONSTANT,
    NAME_IMPLICIT_RENDER_TARGET,
    NAME_IMPLICIT_DEPTHSTENCIL,
    NAME_CURRENT_RENDER_TARGET,
    NAME_CURRENT_DEPTHSTENCIL,
    NAME_BACKBUFFER_WIDTH,
    NAME_BACKBUFFER_HEIGHT,
    NAME_BACKBUFFER_FORMAT,
    NAME_DEPTHSTENCIL_FORMAT,
    NAME_CULL_MODE,
    NAME_ALPHA_TEST_ENABLED,
    NAME_ALPHA_FUNC,
    NAME_ALPHA_REF,
    NAME_Z_ENABLED,
    NAME_Z_FUNCTION,
    NAME_Z_WRITE_ENABLE,
    NAME_ALPHA_BLEND_ENABLED,
    NAME_BLEND_FACTOR,
    NAME_SRC_BLEND,
    NAME_DST_BLEND,
    NAME_BLEND_OP,
    NAME_SEPARATE_ALPHA_BLEND_ENABLED,
    NAME_SRC_BLEND_ALPHA,
    NAME_DST_BLEND_ALPHA,
    NAME_BLEND_OP_ALPHA,
    NAME_STENCIL_ENABLED,
    NAME_STENCIL_REF,
    NAME_STENCIL_FUNC,
    NAME_STENCIL_FAIL,
    NAME_STENCIL_ZFAIL,
    NAME_STENCIL_PASS,
    NAME_STENCIL_MASK,
    NAME_STENCIL_WRITE_MASK,
    NAME_COMMANDS,
    NAME_VERTEXBUFFER_9,
    NAME_TYPE,
    NAME_LENGTH,
    NAME_USAGE,
    NAME_POOL,
    NAME_DATA,
    NAME_LOCK_COUNT,
    NAME_INDEXBUFFER_9,
    NAME_FORMAT,
    NAME_TEXTURE_9,
    NAME_WIDTH,
    NAME_HEIGHT,
    NAME_LEVELS,
    NAME_LEVEL_COUNT,
    NAME_MIPMAP,
    NAME_CUBETEXTURE_9,
    NAME_FACE_POSITIVE_X,
    NAME_FACE_NEGATIVE_X,
    NAME_FACE_POSITIVE_Y,
    NAME_FACE_NEGATIVE_Y,
    NAME_FACE_POSITIVE_Z,
    NAME_FACE_NEGATIVE_Z,
    NAME_SURFACE_9,
    NAME_VOLUMETEXTURE_9,
    NAME_DEPTH,
    NAME_VOLUME_9,
    NAME_LOCK,
    NAME_SIZE,
    NAME_FLAGS,
    NAME_LEFT,
    NAME_TOP,
    NAME_RIGHT,
    NAME_BOTTOM,
    NAME_PITCH,
    NAME_FRONT,
    NAME_BACK,
    NAME_ROW_PITCH,
    NAME_SLICE_PITCH,
    NAME_DRAW_PRIMITIVE,
    NAME_PRIMITIVE_MODE,
    NAME_START_VERTEX,
    NAME_PRIMITIVE_COUNT,
    NAME_VERTEX_DECLARATION_9,
    NAME_ELEMENT_COUNT,
    NAME_VERTEX_ELEMENT,
    NAME_METHOD,
    NAME_USAGE_INDEX,
    NAME_DRAW_INDEXED_PRIMITIVE,
    NAME_START_INDEX,
    NAME_PRESENT,
    NAME_CLEAR,
    NAME_COLOR,
    NAME_Z,
    NAME_STENCIL,
    NAME_VERTEX_SHADER_9,
    NAME_CODE,
    NAME_PIXEL_SHADER_9,
    NAME_LOCKRECT,
    NAME_LOCKBOX,
    NAME_PS_CONSTANTS,
    NAME_VS_CONSTANTS,
    NAME_PS_INPUT,
    NAME_VS_INPUT,
    NAME_ALPHATEST_REF,
    NAME_ALPHATEST_MINUS_ONE,
    NAME_POSITION,
    NAME_REGISTER,
    NAME_TEXCOORD,
    NAME_VS_OUTPUT,
    NAME_IVBUFFER_MEMORY,
    NAME_SURFACE_MEMORY,
    NAME_VOLUME_MEMORY,
    NAME_ELEMENT_STREAM,
    NAME_STREAMS,
    NAME_PS_TEXTURE,
    NAME_INDEX_STREAM,
    NAME_IVBUFFER,
    NAME_MEMORY,
    NAME_VOLUME,
    NAME_SURFACE,
    NAME_S_DEVICE,
    NAME_PS_TEXTURES,
    NAME_SAMPLER_2D,
    NAME_SAMPLER_VOLUME,
    NAME_SAMPLER_CUBE,
    NAME_VERTEX_STREAM,
    NAME_ALPHA_TEST_ENABLE,
    NAME_BLENDWEIGHT,
    NAME_BLENDINDICES,
    NAME_NORMAL,
    NAME_PSIZE,
    NAME_TANGENT,
    NAME_BINORMAL,
    NAME_TESSFACTOR,
    NAME_FOG,
    NAME_SAMPLE
    
};

typedef Names StateName;

typedef size_t StateIndex;

struct StateId {
    StateName name;
    StateIndex index;
    StateId();
    StateId(StateName, StateIndex = 0);
    bool operator<(const StateId &) const;
    bool operator==(const StateId &) const;
};

enum AccessType{
      READ_ACCESS,
      WRITE_ACCESS
};

class StateDataNode {
public:
    StateId get_id();

    set<StateDataNode*> get_childs();

    StateDataNode* get_parent();

    /** Error if child not found */
    StateDataNode* get_child(StateId id);

    /** Gets all childs with an id with given name. Error if no
        child with given name exists. */
    std::map<StateIndex, StateDataNode *> get_childs(StateName);

    /** Return a child with given id, or 0 if not found */
    StateDataNode* find_child(StateId);

    /** Finds all childs with an id with given name. */
    std::map<StateIndex, StateDataNode *> find_childs(StateName);

    size_t get_data_size();

    /** Set a new data buffer with given size, different from zero.
         Previous data is deleted. No events are broadcasted to observers.
         Error if data is mapped. */
    void set_data_size(size_t data_size);

    /** Reads into destination size bytes at the given offset.
        A size argument of 0 means size of data. */
    void read_data(void *destination, size_t size = 0, int offset = 0);

    /** Write from source size bytes at the given offset.
        A size argument of 0 means size of data. */
    void write_data(const void *source, size_t size = 0, int offset = 0);

    /** Returns a pointer to the storage of data.
        Note using read access doesn't fire the on_write_node_data event.
        Error if already mapped. */
    void* map_data(AccessType access = WRITE_ACCESS, size_t size = 0, int offset = 0);
    /** Finalizes a map access to data.
        Fires the on_write_node_data event */
    void unmap_data();

    /** Error if child exists. */
    void add_child(StateDataNode*);
    /** Error if child not exists. */
    void remove_child(StateDataNode*);

    /** Adds a controller that will be notified about changes in node.
        Note controllers are notified in the same order that were added.
        Error if already present. */
    void add_controller(StateDataNodeController*);
    /** Error if not present. */
    void remove_controller(StateDataNodeController*);

    StateDataNode(StateId id);
    StateDataNode(StateId, size_t _data_size, void *_data = 0);
    ~StateDataNode();
private:
    StateId id;
    void *data_storage;
    size_t data_size;
    bool data_mapped;
    AccessType map_access;
    size_t map_size;
    unsigned int map_offset;

    std::map<StateId, StateDataNode *> childs;
    StateDataNode *parent;

    std::list<StateDataNodeController*> controllers;
    void notify_add_node_child(StateDataNode*);
    void notify_remove_node_child(StateDataNode*);
    void notify_write_node_data(size_t size, unsigned int offset);
};

/** One instance class to locate state tree root. */
class StateDataTree {
public:
    static StateDataNode *get_root();
};

Names usage2name(::BYTE usage);

#endif // STATEDATATREE_H_INCLUDED

