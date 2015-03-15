#include "Common.h"
#include "StateDataTree.h"
using namespace std;


bool StateId::operator<(const StateId &other) const {
    if(name < other.name)
        return true;
    else if(name > other.name)
        return false;
    else {
        return ((unsigned int)(index) < (unsigned int)(other.index));
    }
}

bool StateId::operator==(const StateId &other) const {
    return ( ( name == other.name ) &
             ( (unsigned int)(index) == (unsigned int)(other.index) ) );
}

StateId::StateId(): name(NAME_NO_NAME), index(StateIndex(0)) {}
StateId::StateId(StateName _name, StateIndex _index): name(_name), index(_index) {}

StateId StateDataNode::get_id() { return id; }

size_t StateDataNode::get_data_size() { return data_size; }

void StateDataNode::set_data_size(size_t _data_size) {
    ///@todo This is coded quickly, revisit for improvement
    if(data_mapped)
     throw "Data is mapped";
    if(data_storage != 0) free(data_storage);
    data_size = _data_size;
    data_storage = malloc(data_size);
    memset(data_storage, 0, data_size);
}


StateDataNode* StateDataNode::get_parent() { return parent; }

set<StateDataNode*> StateDataNode::get_childs() {
    set<StateDataNode*> _childs;
    map<StateId, StateDataNode *>::iterator it_c;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++)
        _childs.insert((*it_c).second);
    return _childs;
}

void StateDataNode::read_data(void *destination, size_t size, int offset) {

    if(data_storage == 0) throw "Node doesn't have state data";

    size_t size_to_copy = (size == 0)?data_size:size;
    void * source = (void *)&((unsigned char *)data_storage)[offset];
    memcpy(destination, source, size_to_copy);
}

void* StateDataNode::map_data(AccessType _access, size_t _size, int _offset) {

    if(data_mapped) throw "Storage already locked";

    map_access = _access;
    map_size   = (_size == 0 ) ? data_size : _size;
    map_offset = _offset;

    data_mapped = true;
    return (void *)&((unsigned char *)data_storage)[_offset];
}

void StateDataNode::unmap_data() {
    if(!data_mapped) throw "Storage not locked";

    data_mapped = false;

    if(map_access == WRITE_ACCESS) notify_write_node_data(map_size, map_offset);
}


StateDataNode* StateDataNode::get_child(StateId id) {

    map<StateId, StateDataNode*>::iterator it_child = childs.find(id);

    if(it_child == childs.end()) throw "Child not found";

    return (*it_child).second;
}

map<StateIndex, StateDataNode*> StateDataNode::get_childs(StateName name) {

    ///@todo Find a more elegant way to do the search using STL algorithms i. e.
    map<StateId, StateDataNode *>::iterator it_c;
    map<StateIndex, StateDataNode*> found_childs;
    bool found = false;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        if((*it_c).first.name == name) {
            found_childs.insert(make_pair((*it_c).first.index, (*it_c).second));
            found = true;
        }
    }
    if(!found) throw "No childs with this name";

    return found_childs;
}

StateDataNode* StateDataNode::find_child(StateId id) {
    map<StateId, StateDataNode*>::iterator it_child = childs.find(id);

    if(it_child == childs.end())
        return 0;
    else
        return (*it_child).second;
}

map<StateIndex, StateDataNode*> StateDataNode::find_childs(StateName name) {

    ///@todo Find a more elegant way to do the search using STL algorithms i. e.
    map<StateId, StateDataNode *>::iterator it_c;
    map<StateIndex, StateDataNode*> found_childs;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        if((*it_c).first.name == name) {
            found_childs.insert(make_pair((*it_c).first.index, (*it_c).second));
        }
    }
    return found_childs;
}


void StateDataNode::write_data(const void *source, size_t size, int offset) {

    if(data_storage == 0) throw "Node doesn't have state data";

    size_t size_to_copy = (size == 0)?data_size:size;
    void * destination = (void *)&((unsigned char *)data_storage)[offset];
    memcpy(destination, source, size_to_copy);
    notify_write_node_data(size_to_copy, offset);
}

void StateDataNode::add_child(StateDataNode* child) {

    if(childs.find(child->get_id()) != childs.end()) throw "Child already added";

    child->parent = this;
    childs.insert(make_pair(child->get_id(), child));
    notify_add_node_child(child);
}

void StateDataNode::remove_child(StateDataNode* child) {

    if(childs.find(child->get_id()) == childs.end()) throw "Child not found";

    notify_remove_node_child(child);
    child->parent = 0;
    childs.erase(child->get_id());
}

void StateDataNode::add_controller(StateDataNodeController* controller) {

    if(find(controllers.begin(), controllers.end(), controller) != controllers.end()) throw "Controller already added";

    controllers.push_back(controller);

    controller->on_added_controller(this);
}

void StateDataNode::remove_controller(StateDataNodeController* controller) {

    if(find(controllers.begin(), controllers.end(), controller) == controllers.end()) throw "Controller not found";

    controller->on_removed_controller(this);

    controllers.remove(controller);
}

StateDataNode::StateDataNode(StateId _id): id(_id), parent(0), data_storage(0),
data_mapped(false), map_size(0), map_offset(0), data_size(0), map_access(READ_ACCESS) {
}

StateDataNode::StateDataNode(StateId _id, size_t _data_size, void *_data)
:id(_id), parent(0), data_size(_data_size), data_mapped(false)
, map_access(READ_ACCESS), map_size(0), map_offset(0) {
    data_storage = malloc(data_size);
    if(_data != 0)
        memcpy(data_storage, _data, data_size);
    else
        memset(data_storage, 0, data_size);
}

StateDataNode::~StateDataNode() {
    std::map<StateId, StateDataNode *>::iterator it_c;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        delete (*it_c).second;
    }
    if(data_storage != 0) free(data_storage);
}

void StateDataNode::notify_add_node_child(StateDataNode* child) {
    list<StateDataNodeController*>::iterator it_controllers;
    for(it_controllers = controllers.begin(); it_controllers != controllers.end(); it_controllers ++)
        (*it_controllers)->on_add_node_child(this, child);
}

void StateDataNode::notify_remove_node_child(StateDataNode* child) {
    list<StateDataNodeController*>::iterator it_controllers;
    for(it_controllers = controllers.begin(); it_controllers != controllers.end(); it_controllers ++)
        (*it_controllers)->on_remove_node_child(this, child);
}

void StateDataNode::notify_write_node_data(size_t size, unsigned int offset) {
    list<StateDataNodeController*>::iterator it_controllers;
    for(it_controllers = controllers.begin(); it_controllers != controllers.end(); it_controllers ++)
        (*it_controllers)->on_write_node_data(this, size, offset);
}

StateDataNode *StateDataTree::get_root()
{
    StateId rootID(NAME_ROOT);
    static StateDataNode root(rootID);
    return &root;
}


Names usage2name(::BYTE usage) {
    Names str;
    switch(usage) {
    case D3DDECLUSAGE_POSITION: str = NAME_POSITION;  break;
    case D3DDECLUSAGE_BLENDWEIGHT: str = NAME_BLENDWEIGHT; break;
    case D3DDECLUSAGE_BLENDINDICES: str = NAME_BLENDINDICES; break;
    case D3DDECLUSAGE_NORMAL: str = NAME_NORMAL; break;
    case D3DDECLUSAGE_PSIZE: str = NAME_PSIZE; break;
    case D3DDECLUSAGE_TEXCOORD: str = NAME_TEXCOORD; break;
    case D3DDECLUSAGE_TANGENT: str = NAME_TANGENT; break;
    case D3DDECLUSAGE_BINORMAL: str = NAME_BINORMAL; break;
    case D3DDECLUSAGE_TESSFACTOR: str = NAME_TESSFACTOR; break;
    case D3DDECLUSAGE_POSITIONT: str = NAME_POSITION; break;
    case D3DDECLUSAGE_COLOR: str = NAME_COLOR; break;
    case D3DDECLUSAGE_FOG: str = NAME_FOG; break;
    case D3DDECLUSAGE_DEPTH: str = NAME_DEPTH; break;
    case D3DDECLUSAGE_SAMPLE: str = NAME_SAMPLE; break;
    }
    return str;
}
