
#include <cstdio>
#include "GPUTypes.h"

#ifndef _PIXParser_
#define _PIXParser_


// PIXRun specific structures and id's , see
// "PixRunFileFormat.pdf" in doc
struct AttributeDescriptor
{
    u32bit ADID;
    u32bit type;
    u32bit zero;
    u8bit *name;
    u8bit *format;
};

struct EventAttributeDeclaration
{
    u32bit ADID;
    u8bit *initialization;
};

struct EventDescriptor
{
    u32bit EDID;
    u32bit eventType;
    u8bit *name;
    u32bit attributeCount;
    EventAttributeDeclaration *attributes;
};

struct Event
{
    u32bit EDID;
    u32bit EID;
    u8bit *data;
};


struct AsyncAttribute
{
    u32bit EID;
    u32bit ADID;
    u8bit *data;
};

struct PackedCallPackage
{
    u32bit size;
    u32bit CID;
    u32bit one;
};

// Type identifiers
enum ChunkType
{
    PIX_VERSION          = 0x03E8,
    ATTRIBUTE_DESCRIPTOR = 0x03E9,
    EVENT_DESCRIPTOR     = 0x03EA,
    EVENT                = 0x03EB,
    ASYNC_ATTRIBUTE      = 0x03EC,
    FOOTER               = 0x03ED
};

enum AttributeType
{
    AT_FLOAT = 0,
    AT_STRING = 1,
    AT_HEXUINT32 = 2,
    AT_UINT32 = 3,
    AT_EVENT = 4,
    AT_UINT64 = 5,
    AT_CHILDREN = 6,
    AT_PACKEDCALLPACKAGE = 7
};

struct Chunk
{
    u32bit size;
    u32bit type;
    u8bit *data;

    union
    {
        AttributeDescriptor attribDesc;
        EventDescriptor eventDesc;
        Event event;
        AsyncAttribute asyncAttrib;
    };
};

void readChunk(FILE *f, Chunk *chunk);

void parseAttributeDescriptor(Chunk *chunk, AttributeDescriptor *attrDesc);

void parseEventDescriptor(Chunk *chunk, EventDescriptor *eventDesc);

void parseEvent(Chunk *chunk, Event *event);

void parseAsyncAttribute(Chunk *chunk, AsyncAttribute *asyncAttr);

void deleteChunk(Chunk *chunk);

void printChunk(FILE *f, u32bit chunkID, Chunk *chunk);

void printBuffer(u8bit *data, u32bit size);

void printAttributeDescriptor(AttributeDescriptor *attrDesc);

void printEventDescriptor(EventDescriptor *eventDesc);

void printEvent(Event *event, u32bit size);

void printAsyncAttribute(AsyncAttribute *asyncAttrib, u32bit size);

void printWCharString(u16bit *str);

u32bit copyWCharString(u8bit *&dest, u8bit *src);

//const u32bit ASYNC_ATTRIBUTE = 0x3EC;
//const u32bit PACKED_CALL_PACKAGE = 19;

#endif

