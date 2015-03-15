#include "Common.h"
#include "CSemanticMapping_9.h"

void CSemanticMapping9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {}

void CSemanticMapping9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {}

void CSemanticMapping9::on_write_node_data(StateDataNode* node, size_t size, UINT offset) {}


void CSemanticMapping9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // watch assignation tables
        AssignationsRegistry::get_table(NAME_ELEMENT_STREAM)->add_observer(this);
        AssignationsRegistry::get_table(NAME_VS_INPUT)->add_observer(this);
    }
}

void CSemanticMapping9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // unwatch assignation tables
        AssignationsRegistry::get_table(NAME_VS_INPUT)->remove_observer(this);
        AssignationsRegistry::get_table(NAME_ELEMENT_STREAM)->remove_observer(this);
    }
}

void CSemanticMapping9::on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_ELEMENT_STREAM) {
        assigned_streams.insert(usage);

        // Check if there's an input register with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_registers.find(usage);
        if(it_r != assigned_registers.end()) {

            // Get input register
            ResourceId r = AssignationsRegistry::get_table(NAME_VS_INPUT)->get_assigned_to(*it_r);
            u32bit gpu_register = r.index;

            // Get input stream
            u32bit gpu_stream = resource.index;

            // Bind register to stream
            GPURegData data;
            data.uintVal = gpu_stream;
            GPUDriver::getGPUDriver()->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, gpu_register, data);

            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Mapping vs input register "; )
            D3D_DEBUG( cout << gpu_register << " to stream " << gpu_stream << endl; )
            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Usage is "; )
            D3D_DEBUG( cout << usage.name << "[" << usage.index << "]" << endl; )
        }

    }
    else if(table->get_name() == NAME_VS_INPUT) {
        assigned_registers.insert(usage);


        // Check if there's an stream with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_streams.find(usage);
        if(it_r != assigned_streams.end()) {

             // Get input stream
            ResourceId r = AssignationsRegistry::get_table(NAME_STREAMS)->get_assigned_to(*it_r);
            u32bit gpu_stream = r.index;

            // Get input register
            u32bit gpu_register = resource.index;

            // Bind register to stream
            GPURegData data;
            data.uintVal = gpu_stream;
            GPUDriver::getGPUDriver()->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, gpu_register, data);

            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Mapping vs input register "; )
            D3D_DEBUG( cout << gpu_register << " to stream " << gpu_stream << endl; )
            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Usage is "; )
            D3D_DEBUG( cout << usage.name << "[" << usage.index << "]" << endl; )
        }
        else {

            // Get input register
            u32bit gpu_register = resource.index;

            //  Unbind register
            GPURegData data;
            data.uintVal = ST_INACTIVE_ATTRIBUTE;
            GPUDriver::getGPUDriver()->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, gpu_register, data);

            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Unmapped vs input register "; )
            D3D_DEBUG( cout << gpu_register << endl; )
            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Usage is "; )
            D3D_DEBUG( cout << usage.name << "[" << usage.index << "]" << endl; )
         }

    }
}

void CSemanticMapping9::on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_ELEMENT_STREAM) {
        assigned_streams.erase(usage);

        // Check if there's an input register with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_registers.find(usage);
        if(it_r != assigned_registers.end()) {
             // Get input register
            ResourceId r = AssignationsRegistry::get_table(NAME_VS_INPUT)->get_assigned_to(*it_r);
            u32bit gpu_register = r.index;

            //  Unbind register
            GPURegData data;
            data.uintVal = ST_INACTIVE_ATTRIBUTE;
            GPUDriver::getGPUDriver()->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, gpu_register, data);

            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Unmapped vs input register "; )
            D3D_DEBUG( cout << gpu_register << endl; )
            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Usage is "; )
            D3D_DEBUG( cout << usage.name << "[" << usage.index << "]" << endl; )
          }
    }
    else if(table->get_name() == NAME_VS_INPUT) {
        assigned_registers.erase(usage);

        // Check if there's an stream with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_streams.find(usage);
        if(it_r != assigned_streams.end()) {
            // Get input register
            u32bit gpu_register = resource.index;

            //  Unbind register
            GPURegData data;
            data.uintVal = ST_INACTIVE_ATTRIBUTE;
            GPUDriver::getGPUDriver()->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, gpu_register, data);

            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Unmapped vs input register "; )
            D3D_DEBUG( cout << gpu_register << endl; )
            D3D_DEBUG( cout << "CSEMANTICMAPPING9: Usage is "; )
            D3D_DEBUG( cout << usage.name << "[" << usage.index << "]" << endl; )

        }
     }
}



