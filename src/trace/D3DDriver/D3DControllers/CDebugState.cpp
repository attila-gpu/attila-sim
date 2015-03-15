
#include "Common.h"
#include "CDebugState.h"

void CDebugState::on_add_node_child(StateDataNode* parent, StateDataNode* child) {

    print_id(parent->get_id(), cout);
    cout << ": added child ";
    print_id(child->get_id(), cout);
    cout << endl;
    print_recursive(child, 1, cout);

    add_controller_recursive(child);
}

void CDebugState::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    cout << "CDEBUGSTATE: ";
    print_id(parent->get_id(), cout);
    cout << ": removed child ";
    print_id(child->get_id(), cout);
    cout << endl;
    print_recursive(child, 1, cout);

    remove_controller_recursive(child);
}

void CDebugState::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    cout << "CDEBUGSTATE: ";
    if(node->get_parent() != 0) {
        print_id(node->get_parent()->get_id(), cout);
        cout << ": ";
    }
    print_id(node->get_id(), cout);
    cout << ": " << size << " bytes written at " << offset << ": ";
    void* data = malloc(size);
    node->read_data(data, size, offset);
    print_data(data, size, cout);
    free(data);
    cout << endl;
}

void CDebugState::on_added_controller(StateDataNode* node) {
}

void CDebugState::on_removed_controller(StateDataNode* node) {
}

void CDebugState::print_id(StateId id, ostream& out) {
    out << id.name;
    if(id.index != 0) {
        out << hex;
        out << "<" << id.index << ">";
        out << dec;
    }
}


void CDebugState::print(StateDataNode* node, ostream& out) {
    print_id(node->get_id(), out);
    out << ": ";

    if(node->get_data_size() != 0) {
        void* data = malloc(node->get_data_size());
        node->read_data(data);
        print_data(data, node->get_data_size(), out);
        free(data);
    }
}

void CDebugState::print_recursive(StateDataNode* node, unsigned int level, ostream& out) {
    out << "CDEBUGSTATE: ";
    for(unsigned int i = 0; i < level; i ++)
        out << "    ";
    level ++;

    print(node, out);
    out << endl;
    set<StateDataNode*> childs = node->get_childs();
    set<StateDataNode*>::iterator it_c;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        print_recursive(*it_c, level, out);
    }

}

void CDebugState::add_controller_recursive(StateDataNode* node) {

    node->add_controller(this);

    set<StateDataNode*> childs = node->get_childs();
    set<StateDataNode*>::iterator it_c;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        add_controller_recursive(*it_c);
    }
}

void CDebugState::remove_controller_recursive(StateDataNode* node) {

    set<StateDataNode*> childs = node->get_childs();
    set<StateDataNode*>::iterator it_c;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        remove_controller_recursive(*it_c);
    }

    node->remove_controller(this);

}

void CDebugState::print_data(void* data, size_t size, ostream& out) {
    if(size == sizeof(::BYTE)) {
        out << (unsigned int)(*(::BYTE *)data);
    }
    else if(size == sizeof(WORD)) {
        out << (unsigned int)(*((WORD *)data));
    }
    else if(size == sizeof(DWORD)) {
        out << hex << (unsigned int)(*((DWORD *)data)) << dec;
    }
    else  {
        size_t limit = (size < 256) ? size : 256;
        out << size << " bytes ";
        for(size_t i = 0; i < limit; i ++) {
            out << hex << (int)(((unsigned char *)data))[i] << " ";
        }
        out << "..." << dec;
    }
}
