
#include "PIXParser.h"
#include <cstdio>
#include <cstring>
#include <map>

using namespace std;

map<u32bit, AttributeDescriptor *> attributeDescriptors;
map<u32bit, EventDescriptor *> eventDescriptors;

main(int argc, char *argv[])
{
    FILE *f;

    if (argc < 2)
    {
        printf("Usage:\n");
        printf("  PIXParser <pixrunfile>\n");
        exit(-1);
    }

    f = fopen(argv[1], "rb");

    if (f == NULL)
    {
        printf("Error opening file : %s\n", argv[1]);
        exit(-1);
    }

    bool end = false;

    u32bit readChunks = 0;

    while(!end)
    {
        Chunk *chunk = new Chunk;
        readChunk(f, chunk);

        printChunk(f, readChunks, chunk);

        switch(chunk->type)
        {
            case ATTRIBUTE_DESCRIPTOR:
                {
                    map<u32bit, AttributeDescriptor *>::iterator it;
                    it = attributeDescriptors.find(chunk->attribDesc.ADID);
                    if (it == attributeDescriptors.end())
                    {
                        attributeDescriptors[chunk->attribDesc.ADID] = &chunk->attribDesc;
                    }
                    else
                    {
                        printf("Attribute Descriptor %d already exists.\n");
                        exit(-1);
                    }
                }
                break;
            case EVENT_DESCRIPTOR:
                {
                    map<u32bit, EventDescriptor *>::iterator it;
                    it = eventDescriptors.find(chunk->eventDesc.EDID);
                    if (it == eventDescriptors.end())
                    {
                        eventDescriptors[chunk->eventDesc.EDID] = &chunk->eventDesc;
                    }
                    else
                    {
                        printf("Event Descriptor %d already exists.\n");
                        exit(-1);
                    }
                }
                break;
            default:
                deleteChunk(chunk);
                break;
        }
        
                
        readChunks++;
        end = (readChunks == 1000);
    }

    fclose(f);
}

void readChunk(FILE *f, Chunk *chunk)
{
    u32bit size;
    u32bit type;
    
    fread((char *) &size, sizeof(u32bit), 1, f);
    fread((char *) &type, sizeof(u32bit), 1, f);
    
    chunk->size = size;
    chunk->type = type;

    u8bit *data = new u8bit[size - 4];
    fread((char *) data, 1, size - 4, f);
    chunk->data = data;

    switch(type)
    {
        case ATTRIBUTE_DESCRIPTOR:
            parseAttributeDescriptor(chunk, &chunk->attribDesc);
            break;
        case EVENT_DESCRIPTOR:
            parseEventDescriptor(chunk, &chunk->eventDesc);
            break;
        case EVENT:
            parseEvent(chunk, &chunk->event);
            break;
        case ASYNC_ATTRIBUTE:
            parseAsyncAttribute(chunk, &chunk->asyncAttrib);
            break;
        default:
            break;
    }
}

void parseAttributeDescriptor(Chunk *chunk, AttributeDescriptor *attrDesc)
{
    attrDesc->ADID = *((u32bit *) &chunk->data[0]);
    attrDesc->type = *((u32bit *) &chunk->data[4]);
    attrDesc->zero = *((u32bit *) &chunk->data[8]);
    u32bit size1 = copyWCharString(attrDesc->name, &chunk->data[12]);
    u32bit size2 = copyWCharString(attrDesc->format, &chunk->data[12 + size1 + 4]);
}

void parseEventDescriptor(Chunk *chunk, EventDescriptor *eventDesc)
{
    eventDesc->EDID = *((u32bit *) &chunk->data[0]);
    eventDesc->eventType = *((u32bit *) &chunk->data[4]);
    u32bit size1 = copyWCharString(eventDesc->name, &chunk->data[8]);
    eventDesc->attributeCount = *((u32bit *) &chunk->data[8 + size1 + 4]);
    eventDesc->attributes = new EventAttributeDeclaration[eventDesc->attributeCount];
    u32bit p = 8 + size1 + 4 + 4;
    for(u32bit a = 0; a < eventDesc->attributeCount; a++)
    {
        eventDesc->attributes[a].ADID = *((u32bit *) &chunk->data[p]);
        u32bit size2 = copyWCharString(eventDesc->attributes[a].initialization, &chunk->data[p + 4]);
        p = p + 4 + size2 + 4;
    }
}

void parseEvent(Chunk *chunk, Event *event)
{
    chunk->event.EDID = *((u32bit *) &chunk->data[0]);
    chunk->event.EID = *((u32bit *) &chunk->data[4]);
    chunk->event.data = &chunk->data[8];
}

void parseAsyncAttribute(Chunk *chunk, AsyncAttribute *asyncAttrib)
{
    chunk->asyncAttrib.EID = *((u32bit *) &chunk->data[0]);
    chunk->asyncAttrib.ADID = *((u32bit *) &chunk->data[4]);
    chunk->asyncAttrib.data = &chunk->data[8];
}

void deleteChunk(Chunk *chunk)
{
    delete[] chunk->data;

    switch(chunk->type)
    {
        case ATTRIBUTE_DESCRIPTOR:
            delete[] chunk->attribDesc.name;
            delete[] chunk->attribDesc.format;
            break;
        case EVENT_DESCRIPTOR:
            delete chunk->eventDesc.name;
            for(u32bit a = 0; a < chunk->eventDesc.attributeCount; a++)
            {
                delete[] chunk->eventDesc.attributes[a].initialization;                
            }
            delete[] chunk->eventDesc.attributes;
            break;        
    }

    delete chunk;
}

void printChunk(FILE *f, u32bit chunkID, Chunk *chunk)
{
    printf("--- Chunk %08d ----\n", chunkID);
    printf("Type = ");
    switch(chunk->type)
    {
        case PIX_VERSION:
            printf("PIX_VERSION");
            break;
        case ATTRIBUTE_DESCRIPTOR:
            printf("ATTRIBUTE_DESCRIPTOR");
            break;
        case EVENT_DESCRIPTOR:
            printf("EVENT_DESCRIPTOR");
            break;
        case EVENT:
            printf("EVENT");
            break;
        case ASYNC_ATTRIBUTE:
            printf("ASYNC_ATTRIBUTE");
            break;
        case FOOTER:
            printf("FOOTER");
            break;
        default:
            printf("unknown 0x%08x", chunk->type);
            break;
    }
    printf("\n");
    printf("Size = %20d\n", chunk->size);
    printf("Data = \n");

    switch(chunk->type)
    {
        case ATTRIBUTE_DESCRIPTOR:
            printAttributeDescriptor(&chunk->attribDesc);
            break;
        case EVENT_DESCRIPTOR:
            printEventDescriptor(&chunk->eventDesc);
            break;
        case EVENT:
            printEvent(&chunk->event, chunk->size);
            break;
        case ASYNC_ATTRIBUTE:
            printAsyncAttribute(&chunk->asyncAttrib, chunk->size);
            break;
        default:
            printBuffer(chunk->data, chunk->size - 4);
            break;
    }

}

void printBuffer(u8bit *data, u32bit size)
{
    u32bit p = 0;
    u32bit b;
    for(b = 0; b < size; b++)
    {
        printf("%02x ", data[b]);
        p++;
        if (p == 16)
        {
            printf("    \"");
            for(u32bit bb = 0; bb < 16; bb++)
            {
                char c = data[b - 15 + bb];
                if ((c < 0x20) || (c == 0x7F))
                    printf(" ");
                else
                    printf("%c", c);
            }
            printf("\"\n");
            p = 0;
        }
    }

    if (p > 0)
    {
        for(u32bit t = p; t < 16; t++)
            printf("   ");
        printf("    \"");
        for(u32bit bb = 0; bb < p; bb++)
        {
            char c = data[b - (p - 1) + bb];
            if ((c < 0x20) || (c == 0x7F))
                printf(" ");
            else
                printf("%c", c);
        }
        printf("\"\n");
    }
    printf("\n");
}

void printAttributeDescriptor(AttributeDescriptor *attrDesc)
{
    printf("  ADID     = 0x%08x\n", attrDesc->ADID);
    printf("  type     = 0x%08x\n", attrDesc->type);
    printf("  zero     = 0x%08x\n", attrDesc->zero);
    printf("  name     = ");
    printWCharString((u16bit *) attrDesc->name);
    printf("\n");
    printf("  format   = ");
    printWCharString((u16bit *) attrDesc->format);
    printf("\n");
}

void printEventDescriptor(EventDescriptor *eventDesc)
{
    printf("  EDID           = 0x%08x\n", eventDesc->EDID);
    printf("  eventType      = 0x%08x\n", eventDesc->eventType);
    printf("  name           = ");
    printWCharString((u16bit *) eventDesc->name);
    printf("\n");
    printf("  attributeCount = 0x%08x\n", eventDesc->attributeCount);
    for(u32bit a = 0; a < eventDesc->attributeCount; a++)
    {
        printf("  --- Event Attribute %04d ---\n", a);
        printf("    ADID = %08x\n", eventDesc->attributes[a].ADID);
        printf("    initialization = ");
        printWCharString((u16bit *) eventDesc->attributes[a].initialization);
        printf("\n");
    }
}

void printEvent(Event *event, u32bit size)
{
    printf("  EDID = %08x\n", event->EDID);
    printf("  EID  = %08x\n", event->EID);

    //  Search the event descriptor
    map<u32bit, EventDescriptor *>::iterator it;
    it = eventDescriptors.find(event->EDID);
    if (it == eventDescriptors.end())
    {
        printf("  Data : \n");
        printBuffer(event->data, size - 12);
    }
    else
    {
        printf("  Data : \n");
        printBuffer(event->data, size - 12);

        EventDescriptor *eventDesc = it->second;
        printf("  Event Info : \n");
        printf("    Event type = %08x\n", eventDesc->eventType);
        printf("    Event name = ");
        printWCharString((u16bit *) eventDesc->name);
        printf("\n");
        printf("    Attributes :\n");
        u32bit p = 0;
        for(u32bit a = 0; a < eventDesc->attributeCount; a++)
        {
            //  Search for the attribute descriptor.
            map<u32bit, AttributeDescriptor*>::iterator it;
            it = attributeDescriptors.find(eventDesc->attributes[a].ADID);
            if (it == attributeDescriptors.end())
            {
                printf("     [%04d] => Not found\n", a);
                p = 0xDEADCAFE;
            }
            else
            {
                AttributeDescriptor *attrDesc = it->second;

                printf("     [%04d] => [Info : ",a);
                printf(" | %d ", attrDesc->type);
                printf(" | ");
                printWCharString((u16bit *) attrDesc->name);
                printf(" | ");
                printWCharString((u16bit *) attrDesc->format);
                printf(" | ");
                printWCharString((u16bit *) eventDesc->attributes[a].initialization);
                printf(" ] = ");
                u8bit *init = eventDesc->attributes[a].initialization;
                if (p == 0xDEADCAFE)
                {
                    printf("Data Unsynchronized");
                }
                else if ((init[0] == 'C') && (init[2] == 'a') && (init[4] == 'l') && (init[6] == 'c') &&
                         (init[8] == 'O') && (init[10] == 'n') && (init[12] == 'L') && (init[14] == 'o') &&
                         (init[16] == 'a') && (init[18] == 'd'))
                {
                    printf("Calculated On Load");
                }
                else if ((init[0] == 'A') && (init[2] == 's') && (init[4] == 'y') && (init[6] == 'n') &&
                         (init[8] == 'c'))
                {
                    printf("Async");
                }
                else if ((init[0] == '(') && (init[2] == 'e') && (init[4] == 'd') && (init[6] == 'i') &&
                         (init[8] == 'd') && (init[10] == ')'))
                {
                    printf("%d", event->EDID);
                }
                else if ((init[0] == '(') && (init[2] == 'e') && (init[4] == 'i') && (init[6] == 'd') &&
                         (init[8] == ')'))
                {
                    printf("%d", event->EID);
                }
                else if (init[0] == '(')
                {
                    switch(attrDesc->type)
                    {
                        case AT_FLOAT:
                            printf("%f", *((f32bit *) &event->data[p]));
                            p += 4;
                            break;
                        case AT_STRING:
                            {
                                u32bit size = (*((u32bit *) &event->data[p]) + 1) * 2;
                                printWCharString((u16bit *) &event->data[p + 4]);
                                p += size + 4;
                            }
                            break;
                        case AT_HEXUINT32:
                            printf("%08x", *((u32bit *) &event->data[p]));
                            p += 4;
                            break;
                        case AT_UINT32:
                            printf("%10d", *((u32bit *) &event->data[p]));
                            p += 4;
                            break;
                        case AT_EVENT:
                            {
                                u32bit size = (*((u32bit *) &event->data[p]) + 1) * 2;
                                printWCharString((u16bit *) &event->data[p + 4]);
                                p += size + 4;
                            }
                            break;
                        case AT_UINT64:
                            printf("%20lld", *((u64bit *) &event->data[p]));
                            p += 8;
                            break;
                        case AT_CHILDREN:
                            printf("%08x", *((u32bit *) &event->data[p]));
                            p += 4;
                            break;
                        default:
                            printf("Format unknown");
                            p = 0xDEADCAFE;
                            break;
                    }
                }
                else
                {
                    printf("unknown");
                }

                printf("\n");
            }
        }
    }
}

void printAsyncAttribute(AsyncAttribute *asyncAttrib, u32bit size)
{
    printf("  EID  = %08x\n", asyncAttrib->EID);
    printf("  ADID = %08x\n", asyncAttrib->ADID);

    //  Search for the attribute descriptor.
    map<u32bit, AttributeDescriptor*>::iterator it;
    it = attributeDescriptors.find(asyncAttrib->ADID);
    if (it == attributeDescriptors.end())
    {
        printf("  Data : \n");
        printBuffer(asyncAttrib->data, size - 12);
    }
    else
    {
        printf("  Data : \n");
        printBuffer(asyncAttrib->data, size - 12);

        AttributeDescriptor *attrDesc = it->second;

        printf("  Attribute Info => %d ", attrDesc->type);
        printf(" | ");
        printWCharString((u16bit *) attrDesc->name);
        printf(" | ");
        printWCharString((u16bit *) attrDesc->format);
        /*printf(" = ");
        switch(attrDesc->type)
        {
            case AT_FLOAT:
                printf("%f", *((f32bit *) &event->data[p]));
                p += 4;
                break;
            case AT_STRING:
                {
                    u32bit size = (*((u32bit *) &event->data[p]) + 1) * 2;
                    printWCharString((u16bit *) &event->data[p + 4]);
                    p += size + 4;
                }
                break;
            case AT_HEXUINT32:
                printf("%08x", *((u32bit *) &event->data[p]));
                p += 4;
                break;
            case AT_UINT32:
                printf("%010d", *((u32bit *) &event->data[p]));
                p += 4;
                break;
            case AT_EVENT:
                {
                    u32bit size = (*((u32bit *) &event->data[p]) + 1) * 2;
                    printWCharString((u16bit *) &event->data[p + 4]);
                    p += size + 4;
                }
                break;
            case AT_UINT64:
                printf("%020lld", *((u64bit *) &event->data[p]));
                p += 8;
                break;
            case AT_CHILDREN:
                printf("%08x", *((u32bit *) &event->data[p]));
                p += 4;
                break;
            default:
                printf("Format unknown");
                p = 0xDEADCAFE;
                break;
        }*/
        printf("\n");
    }
}

void printWCharString(u16bit *str)
{
    for(u32bit p = 0; (str[p] != 0); p++)
    {
        u8bit c = u8bit(str[p] & 0xFF);
        if ((c < 0x20) || (c == 0x7F))
            printf(" ");
        else
            printf("%c", c);
    }
}

u32bit copyWCharString(u8bit *&dest, u8bit *src)
{
    u32bit size = (*((u32bit *) src) + 1) * 2;

    dest = new u8bit[size];

    memcpy(dest, &src[4], size);

    return size;
}

