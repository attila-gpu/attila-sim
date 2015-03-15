#include "ResourceAssignationTable.h"
using namespace std;

bool ResourceId::operator<(const ResourceId &other) const {
    if(name < other.name)
        return true;
    else if(name > other.name)
        return false;
    else {
        return ((unsigned int)(index) < (unsigned int)(other.index));
    }
}

bool ResourceId::operator==(const ResourceId &other) const {
    return ( ( name == other.name ) &
             ( (unsigned int)(index) == (unsigned int)(other.index) ) );
}

ResourceId::ResourceId(): name(NAME_NO_NAME), index(ResourceIndex(0)) {}
ResourceId::ResourceId(ResourceName _name, ResourceIndex _index): name(_name), index(_index) {}

bool UsageId::operator<(const UsageId &other) const {
    if(name < other.name)
        return true;
    else if(name > other.name)
        return false;
    else {
        return ((unsigned int)(index) < (unsigned int)(other.index));
    }
}

bool UsageId::operator==(const UsageId &other) const {
    return ( ( name == other.name ) &
             ( (unsigned int)(index) == (unsigned int)(other.index) ) );
}

UsageId::UsageId(): name(NAME_NO_NAME), index(UsageIndex(0)) {}
UsageId::UsageId(UsageName _name, UsageIndex _index): name(_name), index(_index) {}


ResourceStatus::ResourceStatus(): assigned(false) {}
ResourceStatus::ResourceStatus(bool _assigned, UsageId _usage): assigned(_assigned), usage(_usage) {}

ResourceAssignationTable::ResourceAssignationTable(TableName _name): name(_name) {}

TableName ResourceAssignationTable::get_name() { return name; }

ResourceId ResourceAssignationTable::get_assigned_to(UsageId usage) {

    map<UsageId, list<ResourceId> >::const_iterator it_assigned;
    it_assigned = assigned.find(usage);

    if(it_assigned == assigned.end()) throw "Usage not found";

    const list<ResourceId> &assigned = (*it_assigned).second;

    return *assigned.begin();
}

bool ResourceAssignationTable::has_been_assigned(UsageId usage) {
    map<UsageId, list<ResourceId> >::const_iterator it_assigned;
    it_assigned = assigned.find(usage);

    return (it_assigned != assigned.end());
}

std::list<ResourceId> ResourceAssignationTable::get_all_assigned_to(UsageId usage) {
    map<UsageId, list<ResourceId> >::const_iterator it_assigned;
    it_assigned = assigned.find(usage);

    if(it_assigned == assigned.end()) throw "Usage not found";

    list<ResourceId> assigned = (*it_assigned).second;

    return assigned;
}

void ResourceAssignationTable::add_resource(ResourceId resource) {
    map<ResourceId, ResourceStatus>::const_iterator it_status;

    it_status = resource_status.find(resource);
    if(it_status != resource_status.end()) throw "Resource already present";

    ResourceStatus status;
    status.assigned = false;
    resource_status.insert(make_pair(resource, status));
}

void ResourceAssignationTable::remove_resource(ResourceId resource) {
    map<ResourceId, ResourceStatus>::const_iterator it_status;

    it_status = resource_status.find(resource);
    if(it_status == resource_status.end()) throw "Resource not found";

    const ResourceStatus &status = (*it_status).second;
    if(status.assigned) throw "Resource is assigned, can't be removed";

    resource_status.erase(resource);
}

void ResourceAssignationTable::assign(UsageId usage, ResourceId resource) {
    map<ResourceId, ResourceStatus>::const_iterator it_status;

    it_status = resource_status.find(resource);
    if(it_status == resource_status.end()) throw "Resource not found";

    const ResourceStatus &status = (*it_status).second;
    if(status.assigned) throw "Resource is already assigned";

    resource_status[resource] = ResourceStatus(true, usage);

    map<UsageId, list<ResourceId> >::const_iterator it_assigned;
    it_assigned = assigned.find(usage);
    if(it_assigned == assigned.end()) {
        list<ResourceId> resources;
        resources.push_back(resource);
        assigned[usage] = resources;
    }
    else {
        list<ResourceId> resources = (*it_assigned).second;
        resources.push_back(resource);
        assigned[usage] = resources;
    }

    notify_assigned(usage, resource);
}

struct IsAvailable {
  Names name;
  bool operator() (pair<ResourceId, ResourceStatus> resource_info) const {
      return (resource_info.first.name == name) & !resource_info.second.assigned;
    }
  IsAvailable(Names _name) : name(_name) {}
};

ResourceId ResourceAssignationTable::assign(UsageId usage, ResourceName resource_name) {

    IsAvailable is_available(resource_name);
    map< ResourceId, ResourceStatus >::iterator it_status;
    it_status = find_if(resource_status.begin(), resource_status.end(), is_available);

    if(it_status == resource_status.end()) throw "No resources available";

    ResourceId available = (*it_status).first;
    assign(usage, available);

    return available;
}

void ResourceAssignationTable::unassign(UsageId usage) {

    map<UsageId, list<ResourceId> >::const_iterator it_assigned;
    it_assigned = assigned.find(usage);

    if(it_assigned == assigned.end()) throw "Usage not found";

    list<ResourceId> resources = (*it_assigned).second;
    list<ResourceId>::iterator it_resources;
    for(it_resources = resources.begin(); it_resources != resources.end(); it_resources ++) {
        ResourceId resource = *it_resources;
        resource_status[resource] = ResourceStatus(false, UsageId());
    }

    assigned.erase(usage);

    for(it_resources = resources.begin(); it_resources != resources.end(); it_resources ++) {
        ResourceId resource = *it_resources;
        notify_unassigned(usage, resource);
    }
}

void ResourceAssignationTable::add_observer(ResourceAssignationTableObserver* observer) {

    list<ResourceAssignationTableObserver*>::const_iterator it_observer;
    it_observer = find(observers.begin(), observers.end(), observer);
    if(it_observer != observers.end()) throw "Observer already present";

    observers.push_back(observer);
}

void ResourceAssignationTable::remove_observer(ResourceAssignationTableObserver* observer) {

    list<ResourceAssignationTableObserver*>::const_iterator it_observer;
    it_observer = find(observers.begin(), observers.end(), observer);
    if(it_observer == observers.end()) throw "Observer not found";

    observers.remove(*it_observer);
}

void ResourceAssignationTable::notify_assigned(UsageId usage, ResourceId resource) {

    list<ResourceAssignationTableObserver*>::iterator it_observer_list;

    for(it_observer_list = observers.begin(); it_observer_list != observers.end(); it_observer_list ++) {
        (*it_observer_list)->on_assigned(this, usage, resource);
    }
}

void ResourceAssignationTable::notify_unassigned(UsageId usage, ResourceId resource) {

    list<ResourceAssignationTableObserver*>::const_iterator it_observer_list;

    for(it_observer_list = observers.begin(); it_observer_list != observers.end(); it_observer_list ++) {
        (*it_observer_list)->on_unassigned(this, usage, resource);
    }
}

void AssignationsRegistry::add_table(ResourceAssignationTable *table) {

    map<TableName, ResourceAssignationTable *> &tables = get_tables();
    map< TableName, ResourceAssignationTable * >::iterator it_tables;
    it_tables = tables.find(table->get_name());

    if(it_tables != tables.end()) throw "Table already is present";

    tables[table->get_name()] = table;
}

void AssignationsRegistry::remove_table(ResourceAssignationTable *table) {

    map<TableName, ResourceAssignationTable *> &tables = get_tables();
    map< TableName, ResourceAssignationTable * >::iterator it_tables;
    it_tables = tables.find(table->get_name());

    if(it_tables == tables.end()) throw "Table not found";

    tables.erase(it_tables);
}

ResourceAssignationTable *AssignationsRegistry::get_table(TableName name) {

    map<TableName, ResourceAssignationTable *> &tables = get_tables();
    map< TableName, ResourceAssignationTable * >::iterator it_tables;
    it_tables = tables.find(name);

    if(it_tables == tables.end()) throw "Table not found";

    return tables[name];
}

std::map<TableName, ResourceAssignationTable *> &AssignationsRegistry::get_tables() {

    static map<TableName, ResourceAssignationTable *> tables;
    return tables;
}



