#include "Common.h"
#include "CConstants_9.h"

void CConstants9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {}

void CConstants9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {}

void CConstants9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if(node->get_id().name == NAME_VS_CONSTANT) {
        update_constant(node);
    }
    else if(node->get_id().name == NAME_PS_CONSTANT) {
        update_constant(node);
    }
    else if(node->get_id().name == NAME_ALPHA_REF) {
        // Check if alpha ref constant is assigned
        set<UsageId>::iterator it_c;
        UsageId usage(NAME_ALPHATEST_REF);
        it_c = assigned_ps_constants.find(usage);
        if(it_c != assigned_ps_constants.end()) {
        	// Get value to write
        	DWORD alpha_ref;
        	node->read_data(&alpha_ref);
        	D3D_DEBUG( cout << "CCONSTANTS9: Updating alpha test reference constant to " << (int)alpha_ref << endl; )
			float float_ref = (float)(alpha_ref & 0xFF) / 255.0f;
			GPURegData data;
			for(u32bit i = 0; i < 4; i ++) data.qfVal[i] = float_ref;

			// Get register to write
            ResourceAssignationTable* a_ps_constants;
            a_ps_constants = AssignationsRegistry::get_table(NAME_PS_CONSTANTS);
            ResourceId resource = a_ps_constants->get_assigned_to(usage);

			GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_CONSTANT, u32bit(resource.index), data);
        }
    }
}


void CConstants9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // watch assignation tables
        AssignationsRegistry::get_table(NAME_VS_CONSTANTS)->add_observer(this);
        AssignationsRegistry::get_table(NAME_PS_CONSTANTS)->add_observer(this);
        // watch constants state
        for(UINT i = 0; i < MAX_D3D_VS_CONSTANTS; i ++) {
            node->get_child(StateId(NAME_VS_CONSTANT, StateIndex(i)))->add_controller(this);
        }
        for(UINT i = 0; i < MAX_D3D_PS_CONSTANTS; i ++) {
            node->get_child(StateId(NAME_PS_CONSTANT, StateIndex(i)))->add_controller(this);
        }
        // watch alpha test
        node->get_child(StateId(NAME_ALPHA_REF))->add_controller(this);
        // Associate with device
        s_device = node;
    }
}

void CConstants9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // deassociate with device
        s_device = 0;

		// unwatch alpha test
        node->get_child(StateId(NAME_ALPHA_REF))->remove_controller(this);

        // unwatch constants state
        for(UINT i = 0; i < MAX_D3D_PS_CONSTANTS; i ++) {
            node->get_child(StateId(NAME_PS_CONSTANT, StateIndex(i)))->remove_controller(this);
        }
        for(UINT i = 0; i < MAX_D3D_VS_CONSTANTS; i ++) {
            node->get_child(StateId(NAME_VS_CONSTANT, StateIndex(i)))->remove_controller(this);
        }
        // unwatch assignation tables
        AssignationsRegistry::get_table(NAME_PS_CONSTANTS)->remove_observer(this);
        AssignationsRegistry::get_table(NAME_VS_CONSTANTS)->remove_observer(this);
    }
}

void CConstants9::on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_VS_CONSTANTS) {
        // Remember assignation
        assigned_vs_constants.insert(usage);
        // Update gpu constant value
        if(usage.name == NAME_VS_CONSTANT) {
	        StateDataNode* s_constant;
    	    s_constant = s_device->get_child(StateId(NAME_VS_CONSTANT, StateIndex(usage.index)));
    	    update_constant(s_constant);
        }
    }
    if(table->get_name() == NAME_PS_CONSTANTS) {
        // Remember assignation
        assigned_ps_constants.insert(usage);
        // Update gpu constant value
        if(usage.name == NAME_PS_CONSTANT) {
        	StateDataNode* s_constant;
        	s_constant = s_device->get_child(StateId(NAME_PS_CONSTANT, StateIndex(usage.index)));
        	update_constant(s_constant);
        }
        else if(usage.name == NAME_ALPHATEST_MINUS_ONE) {
        	D3D_DEBUG( cout << "CCONSTANTS9: Updating alpha test \"minus one\" constant" << endl; )
			GPURegData data;
			for(u32bit i = 0; i < 4; i ++) data.qfVal[i] = -1.0f;
			GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_CONSTANT, u32bit(resource.index), data);
        }
        else if(usage.name == NAME_ALPHATEST_REF) {
        	DWORD alpha_ref;
        	s_device->get_child(StateId(NAME_ALPHA_REF))->read_data(&alpha_ref);
        	D3D_DEBUG( cout << "CCONSTANTS9: Updating alpha test reference constant to " << (int)alpha_ref << endl; )
			float float_ref = (float)(alpha_ref & 0xFF) / 255.0f;
			GPURegData data;
			for(u32bit i = 0; i < 4; i ++) data.qfVal[i] = float_ref;
			GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_CONSTANT, u32bit(resource.index), data);
        }

    }

}

void CConstants9::on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_VS_CONSTANTS) {
        // Reset gpu constant value
        D3D_DEBUG( cout << "CCONSTANTS9: Resetting VS constant " << (int)usage.index << endl; )
        GPURegData data;
        for(u32bit i = 0; i < 4; i ++)
            data.qfVal[i] = 0.0f;
        GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_CONSTANT, u32bit(resource.index), data);
        // Remember unassignation
        assigned_vs_constants.erase(usage);
    }
    if(table->get_name() == NAME_PS_CONSTANTS) {
        // Reset gpu constant value
        D3D_DEBUG( cout << "CCONSTANTS9: Resetting PS constant " << (int)usage.index << endl; )
        GPURegData data;
        for(u32bit i = 0; i < 4; i ++)
            data.qfVal[i] = 0.0f;
        GPUProxy::get_instance()->writeGPURegister(GPU_FRAGMENT_CONSTANT, u32bit(resource.index), data);
        // Remember unassignation
        assigned_ps_constants.erase(usage);
    }
}



void CConstants9::update_constant(StateDataNode* s_constant) {
    if(s_constant->get_id().name == NAME_VS_CONSTANT) {
        // Check if constant is assigned
        set<UsageId>::iterator it_c;
        UsageId usage(NAME_VS_CONSTANT, StateIndex(s_constant->get_id().index));
        it_c = assigned_vs_constants.find(usage);
        if(it_c != assigned_vs_constants.end()) {
            D3D_DEBUG( cout << "CCONSTANTS9: Updating VS constant " << (int)s_constant->get_id().index << " to "; )
            // Get value from state
            float value[4];
            s_constant->read_data(value);
            GPUProxy* gpu = GPUProxy::get_instance();
            GPURegData gpu_value;
            D3D_DEBUG( cout << "("; )
            for(u32bit i = 0; i < 4; i ++) {
                if(i != 0) { D3D_DEBUG( cout << ", "; ) }
                D3D_DEBUG( cout << value[i]; )
                gpu_value.qfVal[i] = value[i];
            }
            D3D_DEBUG( cout << ")" << endl; )
            // Get assigned gpu register
            ResourceAssignationTable* a_vs_constants;
            a_vs_constants = AssignationsRegistry::get_table(NAME_VS_CONSTANTS);
            ResourceId resource = a_vs_constants->get_assigned_to(usage);
            // Write value to gpu register
            gpu->writeGPURegister(GPU_VERTEX_CONSTANT, u32bit(resource.index), gpu_value);
        }
    }
    else if(s_constant->get_id().name == NAME_PS_CONSTANT) {
        // Check if constant is assigned
        set<UsageId>::iterator it_c;
        UsageId usage(NAME_PS_CONSTANT, StateIndex(s_constant->get_id().index));
        it_c = assigned_ps_constants.find(usage);
        if(it_c != assigned_ps_constants.end()) {
            D3D_DEBUG( cout << "CCONSTANTS9: Updating PS constant " << (int)s_constant->get_id().index << " to "; )
            // Get value from state
            float value[4];
            s_constant->read_data(value);
            GPUProxy* gpu = GPUProxy::get_instance();
            GPURegData gpu_value;
            D3D_DEBUG( cout << "("; )
            for(u32bit i = 0; i < 4; i ++) {
                if(i != 0) { D3D_DEBUG( cout << ", "; ) }
                D3D_DEBUG( cout << value[i]; )
                gpu_value.qfVal[i] = value[i];
            }
            D3D_DEBUG( cout << ")" << endl; )
            // Get assigned gpu register
            ResourceAssignationTable* a_ps_constants;
            a_ps_constants = AssignationsRegistry::get_table(NAME_PS_CONSTANTS);
            ResourceId resource = a_ps_constants->get_assigned_to(usage);
            // Write value to gpu register
            gpu->writeGPURegister(GPU_FRAGMENT_CONSTANT, u32bit(resource.index), gpu_value);
        }
    }

}
