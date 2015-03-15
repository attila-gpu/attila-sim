#include "Common.h"
#include "CRasterization_9.h"

void CRasterization9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CRasterization9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CRasterization9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
        if(node->get_id().name == NAME_CULL_MODE) {
            D3DCULL d3d_cull;
            node->read_data(&d3d_cull);
            GPURegData gpu_cull;
            switch(d3d_cull) {
                case D3DCULL_CW:
                    gpu_cull.culling = gpu3d::FRONT;
                    break;
                case D3DCULL_CCW:
                    gpu_cull.culling = gpu3d::BACK;
                    break;
                case D3DCULL_NONE:
                    gpu_cull.culling = gpu3d::NONE;
                    break;
            }
            GPUProxy::get_instance()->writeGPURegister(GPU_CULLING, gpu_cull);
        }
}


void CRasterization9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        /// @todo watch device interpolation state
        node->get_child(StateId(NAME_CULL_MODE))->add_controller(this);

        // watch assignation tables
        AssignationsRegistry::get_table(NAME_VS_OUTPUT)->add_observer(this);
        AssignationsRegistry::get_table(NAME_PS_INPUT)->add_observer(this);
    }
}

void CRasterization9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        //  Unwatch assignation tables
        AssignationsRegistry::get_table(NAME_VS_OUTPUT)->add_observer(this);
        AssignationsRegistry::get_table(NAME_PS_INPUT)->add_observer(this);
        /// @todo unwatch device interpolation state
        node->get_child(StateId(NAME_CULL_MODE))->remove_controller(this);
    }
}

void CRasterization9::on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_VS_OUTPUT) {
        assigned_vs_output.insert(usage);
        // Check if there's an ps input register with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_ps_input.find(usage);
        if(it_r != assigned_ps_input.end()) {
            // Get assigned vs output register
            u32bit vs_output_register = resource.index;

            // Find assigned ps input register
            ResourceAssignationTable* a_ps_input;
            a_ps_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
            ResourceId r = a_ps_input->get_assigned_to(UsageId(usage));
            u32bit ps_input_register = r.index;

            // Enable vs output, except for position, that is always enabled
            GPURegData data;
            if(vs_output_register != 0) {
                data.booleanVal = true;
                GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, vs_output_register, data);
            }

            // Enable passing the vs output to the ps input register
            data.booleanVal = true;
            GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, ps_input_register, data);

            D3D_DEBUG( cout << "CRASTERIZATION9: Mapped ps input " << ps_input_register << " to vs output " << vs_output_register << endl; )
            D3D_DEBUG( cout << "CRASTERIZATION9: Usage is " << usage.name << "[" << usage.index << "]" << endl; )

            //Check vs output register index == vs input register index
            if(ps_input_register != vs_output_register) {
                D3D_DEBUG( cout << "CRASTERIZATION9: WARNING: vs input register does not match ps input register" << endl; )
            }
        }
    }
    else if(table->get_name() == NAME_PS_INPUT) {
        assigned_ps_input.insert(usage);
        // Check if there's an vs output register with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_vs_output.find(usage);
        if(it_r != assigned_vs_output.end()) {

            // Get assigned ps input register
            u32bit ps_input_register = resource.index;

            // Find assigned vs output register
            ResourceAssignationTable* a_vs_output;
            a_vs_output = AssignationsRegistry::get_table(NAME_VS_OUTPUT);
            ResourceId r = a_vs_output->get_assigned_to(UsageId(usage));
            u32bit vs_output_register = r.index;

            /// @todo Check vs output register index == vs input register index

            // Enable vs output, except for position, that is always enabled
            GPURegData data;
            if(vs_output_register != 0) {
                data.booleanVal = true;
                GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, vs_output_register, data);
            }

            // Enable passing the output to the input register
            data.booleanVal = true;
            GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, ps_input_register, data);

            D3D_DEBUG( cout << "CRASTERIZATION9: Mapped ps input " << ps_input_register << " to vs output " << vs_output_register << endl; )
            D3D_DEBUG( cout << "CRASTERIZATION9: Usage is " << usage.name << "[" << usage.index << "]" << endl; )

            //Check vs output register index == vs input register index
            if(ps_input_register != vs_output_register) {
                D3D_DEBUG( cout << "CRASTERIZATION9: WARNING: vs input register does not match ps input register" << endl; )
            }
        }
    }
}

void CRasterization9::on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_VS_OUTPUT) {
        assigned_vs_output.erase(usage);

        // Check if there's an ps input register with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_ps_input.find(usage);
        if(it_r != assigned_ps_input.end()) {
            // Get assigned vs output register
            u32bit vs_output_register = resource.index;

            // Find assigned ps input register
            ResourceAssignationTable* a_ps_input;
            a_ps_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
            ResourceId r = a_ps_input->get_assigned_to(UsageId(usage));
            u32bit ps_input_register = r.index;

            // Disable vs output, except for position!
           GPURegData data;
           if(vs_output_register != 0) {
                data.booleanVal = false;
                GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, vs_output_register, data);
            }

            // Disable passing the output to the input register
            data.booleanVal = false;
            GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, ps_input_register, data);



            D3D_DEBUG( cout << "CRASTERIZATION9: Unmapped ps input " << ps_input_register << " to vs output " << vs_output_register << endl; )
            D3D_DEBUG( cout << "CRASTERIZATION9: Usage is " << usage.name << "[" << usage.index << "]" << endl; )
        }
    }
    else if(table->get_name() == NAME_PS_INPUT) {
        assigned_ps_input.erase(usage);

        // Check if there's an vs output register with this usage
        set< UsageId >:: iterator it_r;
        it_r = assigned_vs_output.find(usage);
        if(it_r != assigned_vs_output.end()) {

            // Get assigned ps input register
            u32bit ps_input_register = resource.index;

            // Find assigned vs output register
            ResourceAssignationTable* a_vs_output;
            a_vs_output = AssignationsRegistry::get_table(NAME_VS_OUTPUT);
            ResourceId r = a_vs_output->get_assigned_to(UsageId(usage));
            u32bit vs_output_register = r.index;

            // Disable vs output, except for position!
            GPURegData data;
            if(vs_output_register != 0) {
                data.booleanVal = false;
                GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, vs_output_register, data);
            }

            // Disable passing the output to the input register
            data.booleanVal = false;
            GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, ps_input_register, data);

            D3D_DEBUG( cout << "CRASTERIZATION9: Unmapped ps input " << ps_input_register << " to vs output " << vs_output_register << endl; )
            D3D_DEBUG( cout << "CRASTERIZATION9: Usage is " << usage.name << "[" << usage.index << "]" << endl; )
        }
    }
}

