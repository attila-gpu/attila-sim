#ifndef RESOURCEASSIGNATIONTABLE_H_INCLUDED
#define RESOURCEASSIGNATIONTABLE_H_INCLUDED

#include "Common.h"

//typedef std::string ResourceName;
typedef Names ResourceName;

typedef unsigned int ResourceIndex;

struct ResourceId {
    ResourceName name;
    ResourceIndex index;
    ResourceId(ResourceName, ResourceIndex = 0);
    ResourceId();
    bool operator<(const ResourceId &) const;
    bool operator==(const ResourceId &) const;
};

//typedef std::string UsageName;
//typedef std::string TableName;

typedef Names UsageName;
typedef Names TableName;

typedef size_t UsageIndex;

struct UsageId {
    UsageName name;
    UsageIndex index;
    UsageId(UsageName, UsageIndex = 0);
    UsageId();
    bool operator<(const UsageId &) const;
    bool operator==(const UsageId &) const;
};

struct ResourceStatus {
    bool assigned;
    UsageId usage; ///<  meaningful only if assigned.
    ResourceStatus(bool _assigned, UsageId _usage);
    ResourceStatus();
};

class ResourceAssignationTable;

class ResourceAssignationTableObserver {
public:
    virtual void on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) = 0;
    virtual void on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) = 0;
    virtual ~ResourceAssignationTableObserver() {}
};

/** Relationship between a set of resources and a set of
    usages. One resource can be assigned to one usage at
    a time, usages can have assigned to many resources. */
class ResourceAssignationTable {
public:
    TableName get_name();

    /** Returns any resource assigned to the usage.
        Error if usage not assigned to any resource. */
    ResourceId get_assigned_to(UsageId usage);

    /** Returns all resources assigned to the usage.
        Error if usage not assigned to any resource. */
    std::list<ResourceId> get_all_assigned_to(UsageId usage);

    /** Returns true if usage is assigned to at least a resource,
        or false if it's not being assigned.
        @note As a quantum computing personal experiment a result
              simulaneously true and false can be returned. Take a look at
              it before to use, and there will be no problem.
    */
    bool has_been_assigned(UsageId usage);

    /** Adds a resource to the relationship.
        Error if resource already present. */
    void add_resource(ResourceId resource);

    /** Error if resource not present or is assigned to an
        usage. */
    void remove_resource(ResourceId resource);

    /** Assigns the resource to the usage.
        Error if resource is not present or
        is already assigned. */
    void assign(UsageId usage, ResourceId resource);

    /** Assigns any resource with provided name to the usage.
        Error if no resource present with this name or
        all resources with this name are already assigned. */
    ResourceId assign(UsageId usage, ResourceName resource_name);

    /** Unassigns the resource to the usage.
        Error if resource is not present or
        is already assigned. */
    void unassign(UsageId usage);

    /** Adds an observer that will be notified about assignations.
        The observers will be notified in the same order that were
        added.
        Error if observer already present */
    void add_observer(ResourceAssignationTableObserver* observer);

    /** Removes an observer from observer list.
        Error if observer not present */
    void remove_observer(ResourceAssignationTableObserver* observer);

    ResourceAssignationTable(TableName _name);

private:
    TableName name;

    std::map<ResourceId, ResourceStatus> resource_status;
    std::map<UsageId, std::list<ResourceId> > assigned;

    std::list<ResourceAssignationTableObserver*> observers;

    void notify_assigned(UsageId usage, ResourceId resource);
    void notify_unassigned(UsageId usage, ResourceId resource);
};

/** Single instance class for locating resource assignation
    tables. */
class AssignationsRegistry {
public:
    /** Error if table present */
    static void add_table(ResourceAssignationTable *table);
    /** Error if table is not present */
    static void remove_table(ResourceAssignationTable *table);
    /** Error if table not present */
    static ResourceAssignationTable *get_table(TableName name);
private:
    static std::map<TableName, ResourceAssignationTable *> &get_tables();

};

#endif // RESOURCEASSIGNATIONTABLE_H_INCLUDED
