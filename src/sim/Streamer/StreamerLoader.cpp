/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: StreamerLoader.cpp,v $
 * $Revision: 1.26 $
 * $Author: vmoya $
 * $Date: 2007-11-23 10:06:23 $
 *
 * Streamer Loader class implementation file.
 *
 */

#include "StreamerLoader.h"
#include "ConsumerStateInfo.h"
#include "StreamerControlCommand.h"
#include "ShaderStateInfo.h"

using namespace gpu3d;

/*  Streamer Loader box constructor.  */
StreamerLoader::StreamerLoader(u32bit unitId, u32bit idxCycle, u32bit irqSize, u32bit attrCycle,
    u32bit lines, u32bit lineSz, u32bit portWidth, u32bit cacheRequests, u32bit cacheInputReqs,
    bool forceAttrLoadBypass, u32bit shNumber, char **shPrefixes,
    char *name, char *prefix, Box *parent) :

    indicesCycle(idxCycle), IRQSize(irqSize), attributesCycle(attrCycle),
    inputCacheLines(lines), inputCacheLineSize(lineSz), inputCachePortWidth(portWidth),
    inputCacheReqQSize(cacheRequests), inputCacheFillQSize(cacheInputReqs), numShaders(shNumber),
    forceAttrLoadBypass(forceAttrLoadBypass),
    unitID(unitId), Box(name, parent)

{

    u32bit i;

    /*  Check shaders and shader signals prefixes.  */
    GPU_ASSERT(

        if (indicesCycle == 0)
            panic("StreamerLoader", "StreamerLoader", "At least an index must be received per cycle.");

        if (attributesCycle == 0)
            panic("StreamerLoader", "StreamerLoader", "At least an attribute must be filled per cycle.");

        if (indicesCycle > IRQSize)
            panic("StreamerLoader", "StreamerLoader", "The input request queue must be at least indices per cycle large.");

        /*  There must at least 1 Shader.  */
        if (numShaders == 0)
            panic("StreamerLoader", "StreamerLoader", "Invalid number of Shaders (must be >0).");

        /*  The shader prefix pointer array must be not null.  */
        if (shPrefixes == NULL)
            panic("StreamerLoader", "StreamerLoader", "The shader prefix array is null.");

        /*  Check if the prefix arrays point to null.  */
        for(i = 0; i < numShaders; i++)
        {
            if (shPrefixes[i] == NULL)
                panic("StreamerLoader", "StreamerLoader", "Shader prefix array component is null.");
        }
    )

    /*  Allocate signal arrays for the shader signals.  */
    shCommSignal = new Signal*[numShaders];
    shStateSignal = new Signal*[numShaders];

    /*  Create signals from/to the shaders.  */
    for(i = 0; i < numShaders; i++)
    {
        /*  Command signals to the shaders.  */
        shCommSignal[i] = newOutputSignal("ShaderCommand", 1, 1, shPrefixes[i]);

        /*  State signals from the shaders.  */
        shStateSignal[i] = newInputSignal("ShaderState", 1, 1, shPrefixes[i]);
    }

    /*  Allocate memory for the Shader State array.  */
    shaderState = new ShaderState[numShaders];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if (shaderState == NULL)
            panic("StreamerLoader", "StreamerLoader", "Error allocating the shader state array.");
    )

    /*  Create request signal to the Memory Controller.  */
    memoryRequest = newOutputSignal("StreamerLoaderMemoryRequest", 1, 1, prefix);

    /*  Create the data signal from the Memory Controller.  */
    memoryData = newInputSignal("StreamerLoaderMemoryData", 2, 1, prefix);

    /*  Create command signal from the main Streamer box.  */
    streamerCommand = newInputSignal("StreamerLoaderCommand", 1, 1, prefix);

    /*  Create new index signal from the Streamer Output Cache.  */
    streamerNewIndex = newInputSignal("StreamerLoaderNewIndex", indicesCycle, 1, prefix);

    /*  Create deallocation signal to the Streamer Fetch.  */
    streamerLoaderDeAlloc = newOutputSignal("StreamerLoaderDeAlloc", 2 * indicesCycle, 1, prefix);


    /*  Create the input cache as a fully associative cache.  */
    cache = new InputCache(
	atoi((const char *)&prefix[2]), /*  The streamer loader unit identifier.  */
        inputCacheLines,                /*  Number of sets in the cache.  */
        1,                              /*  Number of lines per set (1 = fully associative).  */
        inputCacheLineSize,             /*  Size of a cache line in bytes.  */
        attributesCycle,                /*  Number of read ports.  */
        inputCachePortWidth,            /*  Width in bytes of the cache read/write ports.  */
        inputCacheReqQSize,             /*  Number of entries in the cache request queue.  */
        inputCacheFillQSize             /*  Number of entries in the cache input request queue.  */
        );


    /*  Allocate memory for the input request queue.  */
    inputRequestQ = new InputRequest[IRQSize];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if (inputRequestQ == NULL)
            panic("StreamerLoader", "StreamerLoader", "Error allocating memory for the Input Request Queue.");
    )


    /*  Create statistics.  */
    indices = &getSM().getNumericStatistic("Indices", u32bit(0), "StreamerLoader", prefix);
    transactions = &getSM().getNumericStatistic("MemTransactions", u32bit(0), "StreamerLoader", prefix);
    bytesRead = &getSM().getNumericStatistic("ReadBytes", u32bit(0), "StreamerLoader", prefix);
    fetches = &getSM().getNumericStatistic("Fetches", u32bit(0), "StreamerLoader", prefix);
    noFetches = &getSM().getNumericStatistic("NoFetches", u32bit(0), "StreamerLoader", prefix);
    reads = &getSM().getNumericStatistic("Reads", u32bit(0), "StreamerLoader", prefix);
    noReads = &getSM().getNumericStatistic("NoReads", u32bit(0), "StreamerLoader", prefix);
    inputs = &getSM().getNumericStatistic("Inputs", u32bit(0), "StreamerLoader", prefix);
    splitted = &getSM().getNumericStatistic("SplittedAttributes", u32bit(0), "StreamerLoader", prefix);
    mapAttributes = &getSM().getNumericStatistic("MappedAttributes", u32bit(0), "StreamerLoader", prefix);
    avgVtxSentThisCycle = &getSM().getNumericStatistic("AvgVtxSentThisCycle", u32bit(0), "StreamerLoader", prefix);
    stallsShaderBusy = &getSM().getNumericStatistic("StallsShaderBusy", u32bit(0), "StreamerLoader", prefix);

    //  Set validation mode flag to disabed.
    validationMode = false;
    
    //  Reset next log pointers.
    nextReadLog = 3;
    nextWriteLog = 0;

    /*  Create dummy last streamer command.  */
    lastStreamCom = new StreamerCommand(STCOM_RESET);

    /*  Set initial state to reset.  */
    state = ST_RESET;
}

/*  Streamer Loader simulation function.  */
void StreamerLoader::clock(u64bit cycle)
{
    MemoryTransaction *memTrans;
    StreamerCommand *streamCom;
    StreamerControlCommand *streamCCom, *streamCComAux;
    ShaderStateInfo *shStateInfo;
    ShaderInput *input;
    QuadFloat *iaux;
    u32bit nextAttribute;
    u32bit dummyLine;
    u32bit i;
    u32bit j;
    u32bit visited;
    u32bit verticesSentThisCycle;

    GPU_DEBUG_BOX( printf("%s => Clock %lld\n", getName(), cycle);)

    /*  Get state from the Shaders.  */
    for (i = 0; i < numShaders; i++)
    {
        /*  Get the state signal from a Shader.  */
        if (!shStateSignal[i]->read(cycle, (DynamicObject *&) shStateInfo))
        {
            /*  Something got lost.  */
            panic("StreamerLoader", "clock", "Missing state signal from a Shader.");
        }

        /*  Store shader state.  */
        shaderState[i] = shStateInfo->getState();

        /*  Delete state signal.  */
        delete shStateInfo;
    }

    /*  Get memory transactions and state from Memory Controller.  */
    while(memoryData->read(cycle, (DynamicObject *&) memTrans))
    {
        /*  Process memory transaction.  */
        processMemoryTransaction(memTrans);
    }

    /*  Clock and update the input cache.  */
    memTrans = cache->update(cycle, memoryState);

    /*  Check if a memory transaction was generated by the input cache.  */
    if (memTrans != NULL)
    {
        /*  Send request to the memory controller.  */
        memoryRequest->write(cycle, memTrans);

        /*  Update statistics.  */
        transactions->inc();
    }

//if (GPU_MOD(cycle, 1000) == 0)
//printf("%s %lld => freeInputs %d loadInputs %d readInputs %d fetchInputs %d\n",
//    getName(),cycle, freeInputs, loadInputs, readInputs, fetchInputs);

    /*  Simulate a cycle.  */
    switch(state)
    {
        case ST_RESET:
            /*  Reset state.  Set default state.  */

            GPU_DEBUG_BOX( printf("%s => RESET state.\n", getName()); )

            /*  Reset registers.  */
            for(i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
            {
                attributeMap[i] = ST_INACTIVE_ATTRIBUTE;
                attrDefValue[i][0] = 0.0f;
                attrDefValue[i][1] = 0.0f;
                attrDefValue[i][2] = 0.0f;
                attrDefValue[i][3] = 1.0f;
            }
            
            for(i = 0; i < MAX_STREAM_BUFFERS; i++)
            {
                streamAddress[i] = 0;
                streamStride[i] = 0;
                streamData[i] = SD_U32BIT;
                streamElements[i] = 0;
                streamFrequency[i] = 0;
                d3d9ColorStream[i] = false;
            }
            
            //  Attribute load bypass is disabled by default.
            attributeLoadBypass = forceAttrLoadBypass;
            
            streamStart = 0;
            
            /*  Reset counters and pointers.  */
            freeInputs = IRQSize;
            fetchInputs = readInputs = loadInputs = 0;
            nextFreeInput = nextFetchInput = nextReadInput = nextLoadInput = 0;

            /*  Reset the input cache.  */
            cache->reset();

            /*  Change state to ready.  */
            state = ST_READY;

            break;

        case ST_READY:
            /*  Ready state.  Wait for command from the Streamer main box.  */

            GPU_DEBUG_BOX( printf("%s => READY state.\n", getName()); )

            /*  Process commands from the Streamer main box.  */
            if(streamerCommand->read(cycle, (DynamicObject *&) streamCom))
                processStreamerCommand(streamCom);

            break;

        case ST_STREAMING:
            /*  Streaming state.  Wait for new indexes to load
                from the StreamerOutputCache box.  */

            GPU_DEBUG_BOX( printf("%s => STREAMING state.\n", getName()); )

            /*  Reset the number of vertices sent this cycle.  */
            verticesSentThisCycle = 0;

            /*  Send fully loaded inputs to the shader.  */

            /*  Check if there are inputs loaded..  */
            for(i = 0, visited = 0; (i < indicesCycle) && (loadInputs > 0); i++)
            {
                /*  There is a vertex loaded in the input cache.  Try to send it to a vertex shader.  */

                /*  Search for the next vertex shader ready.  */
                for(; (visited < numShaders) && (shaderState[nextShader] == SH_BUSY); visited++)
                    nextShader = GPU_MOD(nextShader + 1, numShaders);

                /*  Check if all vertex shaders are busy.  */
                if (visited != numShaders)
                {
                    /**  THIS COULD BE AVOIDED  IF IT COULD BE ENSURED THAT THE BUFFER WITH THE INPUT
                         WOULDN'T BE ERASED BEFORE BEING READ IN THE SHADER.  **/

                    /*  Create a buffer for the input.  */
                    iaux = new QuadFloat[MAX_VERTEX_ATTRIBUTES];

                    /*  Check memory allocation.  */
                    GPU_ASSERT(
                        if (iaux == NULL)
                            panic("StreamerLoader", "clock", "Input buffer could not be allocated.");
                    )

                    /*  Check for last index and hit.  */
                    if (!inputRequestQ[nextLoadInput].stCCom->isAHit())
                    {
                        /*  Copy vertex input to the auxiliar buffer.  */
                        for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
                            iaux[j] = inputRequestQ[nextLoadInput].attributes[j];

                        GPU_DEBUG_BOX(
                            printf("%s => Sending input (%d, %d) to shader.\n", getName(),
                                inputRequestQ[nextLoadInput].index, inputRequestQ[nextLoadInput].instance);
                        )

                        GPU_DEBUG_BOX(
                            for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
                            {
                                if (attributeMap[j] != ST_INACTIVE_ATTRIBUTE)
                                    printf("i[%d] = {%f, %f, %f, %f}\n", j, iaux[j][0], iaux[j][1], iaux[j][2], iaux[j][3]);
                            }
                        )
                        
                        //  Check if validation is enabled.
                        if (validationMode)
                        {
                            //  Add vertex to the log.
                            VertexInputInfo vInfo;
                            
                            vInfo.vertexID.index = inputRequestQ[nextLoadInput].index;
                            vInfo.vertexID.instance = inputRequestQ[nextLoadInput].instance;
                            QuadFloat *vAttr = iaux;
                            for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                                vInfo.attributes[a] = vAttr[a];
                            
                            VertexInputMap::iterator it;
                            
                            //  Check if the vertex was already shaded.
                            it = vertexInputLog[nextWriteLog].find(vInfo.vertexID);
                            
                            if (it != vertexInputLog[nextWriteLog].end())
                            {
                                //  If already shaded check if the values are different.
                                bool attributesAreDifferent = false;
                                
                                for(u32bit a = 0; (a < MAX_VERTEX_ATTRIBUTES) && !attributesAreDifferent; a++)
                                {
                                    //  Evaluate if the attribute values are different.
                                    attributesAreDifferent = (vAttr[a][0] != it->second.attributes[a][0]) ||
                                                             (vAttr[a][1] != it->second.attributes[a][1]) ||
                                                             (vAttr[a][2] != it->second.attributes[a][2]) ||
                                                             (vAttr[a][3] != it->second.attributes[a][3]);
                                }
                                
                                vInfo.differencesBetweenReads = attributesAreDifferent;
                                vInfo.timesRead = it->second.timesRead + 1;
                            }
                            else
                            {
                                vInfo.differencesBetweenReads = false;
                                vInfo.timesRead = 1;
                            }

                            //  Log the shaded vertex.    
                            vertexInputLog[nextWriteLog].insert(make_pair(vInfo.vertexID, vInfo));
                        }
                                                
                        /*  Create a new input.  Use the input index as identifier.  */
                        input = new ShaderInput(ShaderInputID(inputRequestQ[nextLoadInput].instance, inputRequestQ[nextLoadInput].index),
                                                unitID, inputRequestQ[nextLoadInput].oFIFOEntry, iaux);
                    }
                    else
                    {
                        GPU_ASSERT(
                            if (!inputRequestQ[nextLoadInput].stCCom->isLast())
                                panic("StreamerLoader", "clock", "Sending a hit to the shader that wasn't the last input.");
                        )

                        /*  Use default values for this fake vertex attributes.  */
                        for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
                        {
                            //iaux[j][0] = ATTRIBUTE_DEFAULT_VALUE0;
                            //iaux[j][1] = ATTRIBUTE_DEFAULT_VALUE1;
                            //iaux[j][2] = ATTRIBUTE_DEFAULT_VALUE2;
                            //iaux[j][3] = ATTRIBUTE_DEFAULT_VALUE3;
                            iaux[j][0] = 255.0f;
                            iaux[j][1] = 255.0f;
                            iaux[j][2] = 255.0f;
                            iaux[j][3] = 255.0f;
                        }
                        
                        //  Check for attribute load bypass flag.  In this case send a zero index to avoid
                        //  problems when accessing memory.
                        //  NOTE:  In a future implementation the shader and texture unit should disable memory
                        //  loads of shader inputs marked as killed.
                        if (attributeLoadBypass)
                        {
                            //  Set index attribute to zero.
                            iaux[INDEX_ATTRIBUTE][0] = 0.0f;
                            iaux[INDEX_ATTRIBUTE][1] = 0.0f;
                            iaux[INDEX_ATTRIBUTE][2] = 0.0f;
                            iaux[INDEX_ATTRIBUTE][3] = 0.0f;
                        }

                        /*  Create a new input.  Use the input index as identifier.  */
                        input = new ShaderInput(ShaderInputID(inputRequestQ[nextLoadInput].instance, inputRequestQ[nextLoadInput].index),
                                                0, inputRequestQ[nextLoadInput].oFIFOEntry, iaux);

                        /*  Set as killed (fake input, ignore when received).  */
                        input->setKilled();
                    }

                    /*  NOTE:  THIS WILL ONLY WORK IF THE LAST VERTEX IS SENT THE LAST AS IF NO MEMORY
                        RELATED DESORDER WAS INVOLVED.  */

                    /*  Set vertex input last flag.  */
                    input->setLast(inputRequestQ[nextLoadInput].stCCom->isLast());

                    /*  Copy cookies from the new input streamer control command that produced the current input/index.  */
                    input->copyParentCookies(*inputRequestQ[nextLoadInput].stCCom);

                    /*  Send the vertex to the vertex unit.  */
                    shCommSignal[nextShader]->write(cycle, input, 1);
 
                    /*  Increment vertices sent.  */
                    verticesSentThisCycle++;

                    GPU_DEBUG_BOX(
                        printf("%s => Deallocating IRQ entry for index %d instance %d sent to Shader\n",
                            getName(), inputRequestQ[nextLoadInput].index, inputRequestQ[nextLoadInput].instance);
                    )

                    /*  Create deallocation signal to the Streamer Fetch.  */
                    streamCCom = new StreamerControlCommand(STRC_DEALLOC_IRQ, nextLoadInput);

                    /*  Set the streamer loader unit identifier.  */
                    streamCCom->setUnitID(unitID);

                    /*  Copy trace information from the original stream control command.  */
                    streamCCom->copyParentCookies(*inputRequestQ[nextLoadInput].stCCom);

                    /*  Add a new cookie.  */
                    streamCCom->addCookie();

                    /*  Send deallocation signal to the Streamer Fetch.  */
                    streamerLoaderDeAlloc->write(cycle, streamCCom);

                    /*  Delete streamer control command.  */
                    delete inputRequestQ[nextLoadInput].stCCom;

                    /*  Update next shader counter.  */
                    nextShader = GPU_MOD(nextShader + 1, numShaders);

                    /*  Update pointer to the next loaded input.  */
                    nextLoadInput = GPU_MOD(nextLoadInput + 1, IRQSize);

                    /*  Update loaded inputs counter.  */
                    loadInputs--;

                    /*  Update number of free input request queue entries.  */
                    freeInputs++;

                    /*  Update statistics.  */
                    inputs->inc();
                }
                else
                {
                    /*  Update statistics.  */
                    stallsShaderBusy->inc();
                }
            }

            /*  Update statistics.  */
            avgVtxSentThisCycle->incavg(verticesSentThisCycle);

            /*  Fill vertex attributes with data.  */
            for(i = 0; i < attributesCycle; i++)
            {
                /*  Check if there are inputs to read from memory.  */
                if (readInputs > 0)
                {
                    //  Search for the next active attribute in the current input.
                    //  When the attribute load bypass flag is set all the attributes are inactive.
                    for(nextAttribute = inputRequestQ[nextReadInput].nextAttribute;
                        (nextAttribute < MAX_VERTEX_ATTRIBUTES) &&
                        ((attributeMap[nextAttribute] == ST_INACTIVE_ATTRIBUTE) || attributeLoadBypass);
                        nextAttribute++);

                    //  Check if all attributes have been read.
                    if (nextAttribute == MAX_VERTEX_ATTRIBUTES)
                    {
                        //  When the attribute load bypass flag is set the index attribute must be set to the vertex index.
                        if (attributeLoadBypass)
                        {
                            f32bit indexAux;
                            f32bit instanceAux;
                            
                            GPU_DEBUG_BOX(
                                printf("%s => Load INDEX_ATTRIBUTE with value (%d, %d)\n", getName(),
                                    inputRequestQ[nextReadInput].index, inputRequestQ[nextReadInput].instance);
                            )
                            
                            //  The index attribute stores the index in unsigned integer 32-bit format.
                            *((u32bit *) &indexAux) = inputRequestQ[nextReadInput].index;
                            *((u32bit *) &instanceAux) = inputRequestQ[nextReadInput].instance;
                            
                            //  The first component of the index attribute is used for the index.  All other components are set
                            //  to zero.
                            inputRequestQ[nextReadInput].attributes[INDEX_ATTRIBUTE][0] = indexAux;
                            inputRequestQ[nextReadInput].attributes[INDEX_ATTRIBUTE][1] = instanceAux;
                            inputRequestQ[nextReadInput].attributes[INDEX_ATTRIBUTE][2] = 0.0;
                            inputRequestQ[nextReadInput].attributes[INDEX_ATTRIBUTE][3] = 0.0;
                        }

                        GPU_DEBUG_BOX(
                            printf("%s => Input index %d instance %d fully read.\n", getName(), inputRequestQ[nextReadInput].index,
                                inputRequestQ[nextReadInput].instance);
                        )
                        
                        /*  No more attributes to read.  */

                        /*  Update pointer to the next input to read.  */
                        nextReadInput = GPU_MOD(nextReadInput + 1, IRQSize);

                        /*  Update number of inputs to read.  */
                        readInputs--;

                        /*  Update number of load inputs ready to be sent to the shader.  */
                        loadInputs++;
                    }
                    else
                    {
                        //  Check if the next attribute requires reading from the input cache.
                        if (inputRequestQ[nextReadInput].size[nextAttribute][0] == 0)
                        {
                            //  Per instance attribute already present in the per instance attribute cache.
                            
                            GPU_DEBUG_BOX(
                                printf("%s => Reading attribute from the per-instance attribute cache\n", getName());
                            )
                                                            
                            //  Load the input attribute with the read data.
                            loadAttribute(&inputRequestQ[nextReadInput], nextAttribute, buffer);

                            //  Read the next attribute.
                            nextAttribute++;
                        }
                        else
                        {
                            /*  Check if it is a splitted attribute.  */
                            if (inputRequestQ[nextReadInput].split[nextAttribute])
                            {
                                /*  Check which part of the splitted attribute is going ot be read.  */
                                if (inputRequestQ[nextReadInput].nextSplit)
                                {
                                    GPU_DEBUG_BOX(
                                        printf("%s => Reading attribute %d for input index %d instance %d at address %x\n",
                                            getName(), nextAttribute, inputRequestQ[nextReadInput].index,
                                            inputRequestQ[nextReadInput].instance, inputRequestQ[nextReadInput].address[nextAttribute][1]);
                                    )

                                    /*  Try to read the next attribute.  */
                                    if (cache->read(inputRequestQ[nextReadInput].address[nextAttribute][1],
                                        inputRequestQ[nextReadInput].line[nextAttribute][1],
                                        0,
                                        inputRequestQ[nextReadInput].size[nextAttribute][1],
                                        buffer + inputRequestQ[nextReadInput].size[nextAttribute][0]))
                                    {
                                        /*  Load the input attribute with the read data.  */
                                        loadAttribute(&inputRequestQ[nextReadInput], nextAttribute, buffer);

                                        GPU_DEBUG_BOX(
                                             printf("%s => Attribute read (second half).  Unreserving cache line.\n", getName());
                                        )

                                        /*  Unreserve the cache line.  */
                                        cache->unreserve(inputRequestQ[nextReadInput].line[nextAttribute][1], 0);

                                        /*  Read the next attribute.  */
                                        nextAttribute++;

                                        /*  Reset next half of splitted attribute flag.  */
                                        inputRequestQ[nextReadInput].nextSplit = FALSE;

                                        /*  Update statistics.  */
                                        reads->inc();
                                    }
                                    else
                                    {
                                        /*  Update statistics.  */
                                        noReads->inc();
                                    }
                                }
                                else
                                {
                                    GPU_DEBUG_BOX(
                                        printf("%s => Reading attribute %d for input index %d instance %d at address %x\n",
                                            getName(), nextAttribute, inputRequestQ[nextReadInput].index,
                                            inputRequestQ[nextReadInput].instance, inputRequestQ[nextReadInput].address[nextAttribute][0]);
                                    )

                                    /*  Try to read the next attribute.  */
                                    if (cache->read(inputRequestQ[nextReadInput].address[nextAttribute][0],
                                        inputRequestQ[nextReadInput].line[nextAttribute][0],
                                        0,
                                        inputRequestQ[nextReadInput].size[nextAttribute][0],
                                        buffer))
                                    {
                                        GPU_DEBUG_BOX(
                                            printf("%s => Attribute read (first half).  Unreserving cache line.\n", getName());
                                        )

                                        /*  Unreserve the cache line.  */
                                        cache->unreserve(inputRequestQ[nextReadInput].line[nextAttribute][0], 0);

                                        /*  Request the other line of the split attribute.  */
                                        inputRequestQ[nextReadInput].nextSplit = TRUE;

                                        /*  Update statistics.  */
                                        reads->inc();
                                    }
                                    else
                                    {
                                        /*  Update statistics.  */
                                        noReads->inc();
                                    }
                                }
                            }
                            else
                            {
                                GPU_DEBUG_BOX(
                                    printf("%s => Reading attribute %d for input index %d instance %d at address %x\n",
                                        getName(), nextAttribute, inputRequestQ[nextReadInput].index,
                                        inputRequestQ[nextReadInput].instance, inputRequestQ[nextReadInput].address[nextAttribute][0]);
                                )

                                /*  Try to read the next attribute.  */
                                if (cache->read(inputRequestQ[nextReadInput].address[nextAttribute][0],
                                    inputRequestQ[nextReadInput].line[nextAttribute][0],
                                    0,
                                    inputRequestQ[nextReadInput].size[nextAttribute][0],
                                    buffer))
                                {
                                    GPU_DEBUG_BOX(
                                        printf("%s => Attribute read.  Unreserving cache line.\n", getName());
                                    )

                                    /*  Load the input attribute with the read data.  */
                                    loadAttribute(&inputRequestQ[nextReadInput], nextAttribute, buffer);

                                    /*  Unreserve the cache line.  */
                                    cache->unreserve(inputRequestQ[nextReadInput].line[nextAttribute][0], 0);

                                    /*  Read the next attribute.  */
                                    nextAttribute++;

                                    /*  Update statistics.  */
                                    reads->inc();
                                }
                                else
                                {
                                    /*  Update statistics.  */
                                    noReads->inc();
                                }
                            }
                        }

                        /*  Update next input attribute to read.  */
                        inputRequestQ[nextReadInput].nextAttribute = nextAttribute;
                    }
                }
            }

            /*  Fetch data for the vertex attributes.  */
            for(i = 0; i < attributesCycle; i++)
            {
                /*  Check if there is an input with attributes to fetch from memory.  */
                if (fetchInputs > 0)
                {
                    //  Search for the next active attribute in the current input.
                    //  If the attribute load bypass flag is set none of the attributes is active.
                    for(nextAttribute = inputRequestQ[nextFetchInput].nextAttribute;
                        (nextAttribute < MAX_VERTEX_ATTRIBUTES) &&
                        ((attributeMap[nextAttribute] == ST_INACTIVE_ATTRIBUTE) || attributeLoadBypass);
                        nextAttribute++)
                    {
                        GPU_DEBUG_BOX(
                            printf("%s => Reset to default value attribute %d of input index %d instance %d\n",
                                getName(), nextAttribute, inputRequestQ[nextFetchInput].index,
                                inputRequestQ[nextFetchInput].instance);
                        )

                        /*  Reset to default value for inactive attribute.  */
                        inputRequestQ[nextFetchInput].attributes[nextAttribute][0] = attrDefValue[nextAttribute][0];
                        inputRequestQ[nextFetchInput].attributes[nextAttribute][1] = attrDefValue[nextAttribute][1];
                        inputRequestQ[nextFetchInput].attributes[nextAttribute][2] = attrDefValue[nextAttribute][2];
                        inputRequestQ[nextFetchInput].attributes[nextAttribute][3] = attrDefValue[nextAttribute][3];
                    }

                    /*  Check if all the input attributes have been requested.  */
                    if (nextAttribute == MAX_VERTEX_ATTRIBUTES)
                    {
                        /*  All the input attributes have been fetched.  */

                        GPU_DEBUG_BOX(
                            printf("%s => Fetch of input index %d instance %d attributes finished.\n",
                                getName(), inputRequestQ[nextFetchInput].index, inputRequestQ[nextFetchInput].instance);
                        )

                        /*  Check if is a hit (only for last index!).  */
                        if (!inputRequestQ[nextFetchInput].stCCom->isAHit())
                        {
                            /*  Reset next attribute to read for the input.  */
                            inputRequestQ[nextFetchInput].nextAttribute = 0;
                        }
                        else
                        {
                            GPU_ASSERT(
                                if (!inputRequestQ[nextFetchInput].stCCom->isLast())
                                    panic("StreamerLoader", "clock", "Only last index/input can be a hit.");
                            )

                            /*  Nothing to read for this (fake) input.  */
                            inputRequestQ[nextFetchInput].nextAttribute = MAX_VERTEX_ATTRIBUTES;
                        }

                        /*  Update pointer to next input to fetch.  */
                        nextFetchInput = GPU_MOD(nextFetchInput + 1, IRQSize);

                        /*  Update number of fetch inputs.  */
                        fetchInputs--;

                        /*  Update number of inputs to read.  */
                        readInputs++;
                    }
                    else
                    {
                        GPU_DEBUG_BOX(
                            printf("%s => Fetching attribute %d for index %d instance %d\n",
                                getName(), nextAttribute, inputRequestQ[nextFetchInput].index, inputRequestQ[nextFetchInput].instance);
                        )

                        //  Check if the attribute requires fetching data from the cache.
                        if (inputRequestQ[nextFetchInput].size == 0)
                        {
                            //  Per instance attribute already present in the per instance attribute cache.
                            
                            //  Go to the next attribute.
                            nextAttribute++;
                        }
                        else
                        {
                            /*  Check if it the attribute is splited in two lines.  */
                            if (inputRequestQ[nextFetchInput].split[nextAttribute])
                            {
                                if (inputRequestQ[nextFetchInput].nextSplit)
                                {
                                    /*  Fetch the input attribute from the cache.  */
                                    if (cache->fetch(inputRequestQ[nextFetchInput].address[nextAttribute][1],
                                        inputRequestQ[nextFetchInput].line[nextAttribute][1], dummyLine,
                                        inputRequestQ[nextFetchInput].stCCom))
                                    {
                                        /*  The attribute was fetched.  Go for the next one.  */
                                        nextAttribute++;

                                        /*  Set next half of the splitted attribute to false.  */
                                        inputRequestQ[nextFetchInput].nextSplit = FALSE;

                                        /*  Update statistics.  */
                                        fetches->inc();
                                    }
                                    else
                                    {
                                        /*  Update statistics.  */
                                        noFetches->inc();
                                    }
                                }
                                else
                                {
                                    /*  Fetch the input attribute from the cache.  */
                                    if (cache->fetch(inputRequestQ[nextFetchInput].address[nextAttribute][0],
                                        inputRequestQ[nextFetchInput].line[nextAttribute][0], dummyLine,
                                        inputRequestQ[nextFetchInput].stCCom))
                                    {
                                        /*  Set next half of the splitted attribute to false.  */
                                        inputRequestQ[nextFetchInput].nextSplit = TRUE;

                                        /*  Update statistics.  */
                                        fetches->inc();
                                    }
                                    else
                                    {
                                        /*  Update statistics.  */
                                        noFetches->inc();
                                    }
                                }
                            }
                            else
                            {
                                //  Per index attribute.
                                
                                /*  Fetch the input attribute from the cache.  */
                                if (cache->fetch(inputRequestQ[nextFetchInput].address[nextAttribute][0],
                                    inputRequestQ[nextFetchInput].line[nextAttribute][0], dummyLine,
                                    inputRequestQ[nextFetchInput].stCCom))
                                {
                                    /*  The attribute was fetched.  Go for the next one.  */
                                    nextAttribute++;

                                    /*  Update statistics.  */
                                    fetches->inc();
                                }
                                else
                                {
                                    /*  Update statistics.  */
                                    noFetches->inc();
                                }
                            }
                        }

                        /*  Update next attribute at the input request queue entry.  */
                        inputRequestQ[nextFetchInput].nextAttribute = nextAttribute;
                    }
                }
            }

            /*  Read new indexes from the Streamer Output Cache.  */
            while(streamerNewIndex->read(cycle, (DynamicObject *&) streamCCom))
            {
                /*  Check the input request queue is not full.  */
                GPU_ASSERT(
                    if (freeInputs == 0)
                        panic("StreamerLoader", "clock", "No empty input request queue entries.");
                )

                /*  Check that the new index was sent to the proper unit  */
                GPU_ASSERT(
                    if (streamCCom->getUnitID() != unitID)
                        panic("StreameLoader", "clock", "Received input index of a different streamer loader unit.");
                )

                /*  Check if the index is a hit into the output cache and not the last index in the batch.  */
                if (streamCCom->isAHit() && (!streamCCom->isLast()))
                {
                    GPU_DEBUG_BOX(
                        printf("%s => Deallocating IRQ entry for HIT index %d instance %d\n",
                            getName(), streamCCom->getIndex(), streamCCom->getInstanceIndex());
                    )

                    /*  Create deallocation signal to the Streamer Fetch.  */
                    streamCComAux = new StreamerControlCommand(STRC_DEALLOC_IRQ, streamCCom->getIndex());

                    /*  Set the streamer loader unit identifier.  */
                    streamCComAux->setUnitID(unitID);

                    /*  Copy trace information from the original stream control command.  */
                    streamCComAux->copyParentCookies(*streamCCom);

                    /*  Add a new cookie.  */
                    streamCComAux->addCookie();

                    /*  Send deallocation signal to the Streamer Fetch.  */
                    streamerLoaderDeAlloc->write(cycle, streamCComAux);

                    /*  Delete streamer command.  */
                    delete streamCCom;
                }
                else
                {
                    /*  Add new index to the Input Request Queue.  */
                    inputRequestQ[nextFreeInput].index = streamCCom->getIndex();
                    inputRequestQ[nextFreeInput].instance = streamCCom->getInstanceIndex();

                    /*  Store the output FIFO entry for the index.  */
                    inputRequestQ[nextFreeInput].oFIFOEntry = streamCCom->getOFIFOEntry();

                    /*  Store the streamer control command that carries the new index..  */
                    inputRequestQ[nextFreeInput].stCCom = streamCCom;

                    /*  Check for last index and hit.  */
                    if (!streamCCom->isAHit())
                    {
                        /*  Reset next attribute to fetch for the input.  */
                        inputRequestQ[nextFreeInput].nextAttribute = 0;

                        /*  Reset next split flag.  */
                        inputRequestQ[nextFreeInput].nextSplit = FALSE;

                        GPU_DEBUG_BOX(
                            printf("%s => Received NEW INDEX idx %d instance %d. Added to IRQ entry %d\n",
                                getName(), streamCCom->getIndex(), streamCCom->getInstanceIndex(), nextFreeInput);
                        )

                        /*  Configure the attribute info for the input.  */
                        configureAttributes(cycle, &inputRequestQ[nextFreeInput]);
                    }
                    else
                    {
                        GPU_ASSERT(
                            if (!streamCCom->isLast())
                                panic("StreamerLoader", "clock", "Only last index is queued when there is a hit.");
                        )

                        /*  Nothing to fetch or read for this fake input.  */
                        inputRequestQ[nextFreeInput].nextAttribute = MAX_VERTEX_ATTRIBUTES;
                    }

                    /*  Update pointer to the next free input entry.  */
                    nextFreeInput = GPU_MOD(nextFreeInput + 1, IRQSize);

                    /*  Update free IRQ entries counter.  */
                    freeInputs--;

                    /*  Update inputs to be fetched counter.  */
                    fetchInputs++;

                    /*  Update statistics.  */
                    indices->inc();
                }
            }

            /*  Process commands from the Streamer main box.  */
            if(streamerCommand->read(cycle, (DynamicObject *&) streamCom))
                processStreamerCommand(streamCom);

            break;

        default:
            panic("StreamerLoader", "clock", "Unsupported streamer state.");
            break;
    }

}

/*  Processes a memory transaction.  */
void StreamerLoader::processMemoryTransaction(MemoryTransaction *memTrans)
{
    /*  Process the memory transaction.  */
    switch(memTrans->getCommand())
    {
        case MT_STATE:
            /*  Memory Controller sends state.  */
            memoryState = memTrans->getState();

            GPU_DEBUG_BOX( printf("%s => MT_STATE received.\n", getName()); )

            break;

        case MT_READ_DATA:
            /*  Return data from the previous memory request.  */

            GPU_DEBUG_BOX(
                printf("%s => MT_READ_DATA received ID %d.\n",
                    getName(), memTrans->getID());
            )

            /*  Send the transaction to the input cache.  */
            cache->processMemoryTransaction(memTrans);

            /*  Update statistics.  */
            bytesRead->inc(memTrans->getSize());

            break;

        default:
//printf("%p\n", memTrans);
            panic("StreamerLoader", "processMemoryTransaction", "Illegal memory transaction received.");
            break;

    }

    /*  Delete memory transaction.  */
    delete memTrans;

}

/*  Processes a stream command.  */
void StreamerLoader::processStreamerCommand(StreamerCommand *streamCom)
{
    int i;

    /*  Delete last streamer command.  */
    delete lastStreamCom;

    /*  Store as last streamer command (to keep object info).  */
    lastStreamCom = streamCom;

    /*  Process stream command.  */
    switch(streamCom->getCommand())
    {
        case STCOM_RESET:
            /*  Reset command from the Command Processor.  */

            GPU_DEBUG_BOX( printf("%s => RESET command.\n", getName()); )

            /*  Do whatever to do.  */

            /*  Change state to reset.  */
            state = ST_RESET;

            break;

        case STCOM_REG_WRITE:
            /*  Write to register command.  */

            GPU_DEBUG_BOX( printf("%s => REG_WRITE command.\n", getName()); )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_READY)
                    panic("StreamerLoader", "processStreamerCommand", "Command not allowed in this state.");
            )

            processGPURegisterWrite(streamCom->getStreamerRegister(),
                streamCom->getStreamerSubRegister(),
                streamCom->getRegisterData());

            break;

        case STCOM_REG_READ:
            /*  Read from register command.  */

            GPU_DEBUG_BOX( printf("%s => REG_READ command.\n", getName()); )


            /*  Not supported.  */
            panic("StreamerLoader", "processStreamerCommand", "STCOM_REG_READ not supported.");

            break;

        case STCOM_START:
            /*  Start streaming (drawing) command.  */

            GPU_DEBUG_BOX(
                printf("%s => START command.\n", getName());
            )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_READY)
                    panic("Streamer", "processStreamerCommand", "Command not allowed in this state.");
            )

            /*  Check for unsupported stream formats.  */
            for(i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
            {
                /*  Check if attribute is active.  */
                if (attributeMap[i] != ST_INACTIVE_ATTRIBUTE)
                {
                    /*  Update statistics.  */
                    mapAttributes->inc();
                }
            }

            /*  Next vertex shader to send input.  */
            nextShader = 0;

            /*  Initialize streaming variables.  */




            /*  Reset counters and pointers.  */
            freeInputs = IRQSize;
            fetchInputs = readInputs = loadInputs = 0;
            nextFreeInput = nextFetchInput = nextReadInput = nextLoadInput = 0;

            /*  Reset the input cache.  */
            cache->reset();

            //  Reset the cache for per-instance attributes.
            for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
            {
                instanceAttribValid[a] = false;
                instanceAttribTag[a] = 0;
            }
            
            /*  Set streamer state to streaming.  */
            state = ST_STREAMING;

            break;

        case STCOM_END:
            /*  End streaming (drawing) command.  */

            GPU_DEBUG_BOX( printf("%s => END command.\n", getName()); )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_STREAMING)
                    panic("StreamerLoader", "processStreamerCommand", "Command not allowed in this state.");
            )

            //  Check if validation mode is enabled.
            if (validationMode)
            {
                //  Update the pointer to the next write log.
                nextWriteLog = (nextWriteLog + 1) & 0x03;
            }   

            /*  Change state to ready.  */
            state = ST_READY;

            break;


        default:
            panic("StreamerLoader", "processStreamerCommand", "Undefined streamer command.");
            break;
    }
}

/*  Process a register write.  */
void StreamerLoader::processGPURegisterWrite(GPURegister gpuReg, u32bit gpuSubReg,
     GPURegData gpuData)
{

    /*  Process Register.  */
    switch(gpuReg)
    {
        case GPU_VERTEX_ATTRIBUTE_MAP:
            /*  Mapping between stream buffers and vertex input parameters.  */

            GPU_DEBUG_BOX(
                printf("%s => GPU_VERTEX_ATTRIBUTE_MAP[%d] = %d.\n",
                    getName(), gpuSubReg, gpuData.uintVal);
            )

            /*  Check the vertex attribute ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal vertex attribute ID.");
            )

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if ((gpuData.uintVal >= MAX_STREAM_BUFFERS) && (gpuData.uintVal != ST_INACTIVE_ATTRIBUTE))
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            attributeMap[gpuSubReg] = gpuData.uintVal;

            break;

        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE:

            /*  Vertex attribute default value register.  */

            GPU_DEBUG_BOX(
                printf("%s => GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE[%d] = {%f, %f, %f, %f}.\n",
                    getName(), gpuSubReg, gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            /*  Check the vertex attribute ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal vertex attribute ID.");
            )

            /*  Write the register.  */
            attrDefValue[gpuSubReg][0] = gpuData.qfVal[0];
            attrDefValue[gpuSubReg][1] = gpuData.qfVal[1];
            attrDefValue[gpuSubReg][2] = gpuData.qfVal[2];
            attrDefValue[gpuSubReg][3] = gpuData.qfVal[3];

            break;

        case GPU_STREAM_ADDRESS:
            /*  Stream buffer address.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )


            /*  Write the register.  */
            streamAddress[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => GPU_STREAM_ADDRESS[%d] = %x.\n",
                    getName(), gpuSubReg, gpuData.uintVal);
            )

            break;

        case GPU_STREAM_STRIDE:
            /*  Stream buffer stride.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            streamStride[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG_BOX(
               printf("%s => GPU_STREAM_STRIDE[%d] = %d.\n",
                    getName(), gpuSubReg, gpuData.uintVal);
            )

            break;

        case GPU_STREAM_DATA:
            /*  Stream buffer data type.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            streamData[gpuSubReg] = gpuData.streamData;


            GPU_DEBUG_BOX(
                printf("%s => GPU_STREAMDATA[%d] = ", getName(), gpuSubReg);
                switch(gpuData.streamData)
                {
                    case SD_UNORM8:
                        printf("SD_UNORM8");
                        break;
                    case SD_SNORM8:
                        printf("SD_SNORM8");
                        break;
                    case SD_UNORM16:
                        printf("SD_UNORM16");
                        break;
                    case SD_SNORM16:
                        printf("SD_SNORM16");
                        break;
                    case SD_UNORM32:
                        printf("SD_UNORM32");
                        break;
                    case SD_SNORM32:
                        printf("SD_SNORM32");
                        break;
                    case SD_FLOAT16:
                        printf("SD_FLOAT16");
                        break;                    
                    case SD_FLOAT32:
                        printf("SD_FLOAT32");
                        break;
                    case SD_UINT8:
                        printf("SD_UINT8");
                        break;
                    case SD_SINT8:
                        printf("SD_SINT8");
                        break;
                    case SD_UINT16:
                        printf("SD_UINT16");
                        break;
                    case SD_SINT16:
                        printf("SD_SINT16");
                        break;
                    case SD_UINT32:
                        printf("SD_UINT32");
                        break;
                    case SD_SINT32:
                        printf("SD_SINT32");
                        break;
                    default:
                        panic("StreamerLoader", "processGPURegisterWrite", "Undefined format.");
                        break;                        
                }
            )

            break;

        case GPU_STREAM_ELEMENTS:
            /*  Stream buffer elements.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            streamElements[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => GPU_STREAM_ELEMENTS[%d] = %d.\n",
                    getName(), gpuSubReg, gpuData.uintVal);
            )

            break;

        case GPU_STREAM_FREQUENCY:
            
            //  Stream buffer frequency.

            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            //  Write the register.
            streamFrequency[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => GPU_STREAM_FREQUENCY[%d] = %d.\n", getName(), gpuSubReg, gpuData.uintVal);
            )

            break;

        case GPU_STREAM_START:
            /*  Streaming start position.  */

            streamStart = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => GPU_STREAM_START = %d.\n", getName(), gpuData.uintVal);
            )

            break;

        case GPU_D3D9_COLOR_STREAM:
        
            //  Sets if the color stream must be read using the component order defined by D3D9.
            
            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("StreamerLoader", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            d3d9ColorStream[gpuSubReg] = gpuData.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("%s => GPU_D3D9_COLOR_STREAM[%d] = %s.\n", getName(), gpuSubReg, gpuData.booleanVal? "TRUE" : "FALSE");
            )
            
            break;
            
        case GPU_ATTRIBUTE_LOAD_BYPASS:
        
            //  Sets if attribute load is disabled.
            attributeLoadBypass = gpuData.booleanVal || forceAttrLoadBypass;
            
            GPU_DEBUG_BOX(
                printf("%s => GPU_ATTRIBUTE_LOAD_BYPASS = %s.\n", getName(), gpuData.booleanVal? "TRUE" : "FALSE");
            )
            
            break;

        default:
            panic("StreamerLoader", "processGPURegisterWrite", "Not a Streamer register.");
            break;
    }
}

/*  Converts from the stream buffer data format to a 32 bit float.  */
f32bit StreamerLoader::attributeDataConvert(StreamData format, u8bit *data)
{
    f32bit res;

    switch(format)
    {
        case SD_UNORM8:
            res = f32bit(data[0]) * (1.0f / 255.0f);
            break;
        case SD_SNORM8:
            res = f32bit(*((s8bit *) data)) * (1.0f / 127.0f);
            break;
        case SD_UNORM16:
            res = f32bit(*((u16bit *) data)) * (1.0f / 65535.0f);
            break;
        case SD_SNORM16:
            res = f32bit(*((s16bit *) data)) * (1.0f / 32767.0f);
            break;
        case SD_UNORM32:
            res = f32bit(*((u32bit *) data)) * (1.0f / 4294967295.0f);
            break;
        case SD_SNORM32:
            res = f32bit(*((s32bit *) data)) * (1.0f / 2147483647.0f);
            break;
        case SD_FLOAT16:
            res = GPUMath::convertFP16ToFP32(*((f16bit *) data));
            break;                    
        case SD_FLOAT32:
            res = *((f32bit *) data);
            break;
        case SD_UINT8:
            res = f32bit(data[0]);
            break;
        case SD_SINT8:
            res = f32bit(*((s8bit *) data));
            break;
        case SD_UINT16:
            res = f32bit(*((u16bit *) data));            
            break;
        case SD_SINT16:
            res = f32bit(*((s16bit *) data));            
            break;
        case SD_UINT32:
            res = f32bit(*((u32bit *) data));            
            break;
        case SD_SINT32:
            res = f32bit(*((s32bit *) data));            
            break;
        default:
            panic("StreamerLoader", "attributeDataConvert", "Unsupported stream data format.");
            break;
    }

    return res;
}

/*  Calculates the addresses and sizes for the input active attributes.  */
void StreamerLoader::configureAttributes(u64bit cycle, InputRequest *input)
{
    u32bit stBuff;
    u32bit dataSize;
    u32bit offset;
    u32bit i;

    for(i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
    {
        /*  Check if it is an active attribute.  */
        if (attributeMap[i] != ST_INACTIVE_ATTRIBUTE)
        {
            /*  Get Stream Buffer ID for the attribute.  */
            stBuff = attributeMap[i];

            GPU_ASSERT(
                if (streamElements[stBuff] == 0)
                    panic("StreamerLoader", "configureAttributes", "Zero active elements are not allowed for defined attributes.");
            )

            u32bit readIndex;
            bool needsRead;
            
            //  Check if the attribute holds per instance data.
            if (streamFrequency[stBuff] != 0)
            {
                //  Check if the per instance data is already present.
                if (instanceAttribValid[i] && (instanceAttribTag[i] == (input->instance / streamFrequency[stBuff])))
                {
                    //  The per instance data is already present.  No need to read from memory.
                    input->size[i][0] = 0;
                    input->address[i][0] = 0;
                    input->split[i] = false;
                    readIndex = 0;
                    needsRead = false;;
                }
                else
                {
                    //  Per instance attribute, use the instance to index the data stream.
                    readIndex = input->instance / streamFrequency[stBuff];
                    
                    //  Update the per instance attribute cache.
                    instanceAttribValid[i] = true;
                    instanceAttribTag[i] = readIndex;
                    needsRead = true;
                }
            }
            else
            {
                //  Per index attribute, use index to index the data stream.
                readIndex = input->index;
                needsRead = true;
            }

            if (needsRead)
            {
                /*  NOTE:  Access to the Input Cache must be 32 bit (4 bytes) aligned.  */

                /*  Get attribute data element size.  */
                dataSize = u32bit(ceil(f32bit(streamElements[stBuff] * streamDataSize[streamData[stBuff]]) / 4.0f)) * 4;

                /*  Calculate attribute address.  */
                input->address[i][0] = streamAddress[stBuff] + readIndex * streamStride[stBuff];

                /*  If the data isn't aligned to 4 bytes an additional 4 byte word must be read.  */
                if ((input->address[i][0] & 0x03) != 0)
                {
                    dataSize += 4;
                }

                /*  Set attribute size in memory.  */
                input->size[i][0] = dataSize;

                /*  Calculate offset inside the line.  */
                offset = (GPU_MOD(input->address[i][0], inputCacheLineSize)) & 0xfffffffc;

                /*  Determine if the attribute is split in two cache lines.  */
                input->split[i] = ((offset + dataSize) > inputCacheLineSize);

//if ((cycle > 6314600) && (cycle < 6324000))
//printf("StLd %lld => index %d attribute %d address %x\n", cycle, input->index, i, input->address[i][0]);

                /*  Check if splitted attribute.  */
                if (input->split[i])
                {
                    /*  Calculate second attribute address.  */
                    input->address[i][1] = (input->address[i][0] & 0xfffffffc) + inputCacheLineSize - offset;

//if ((cycle > 6314600) && (cycle < 6324000))
//printf("StLd %lld => index %d attribute %d split address 2 %x\n", cycle, input->index, i, input->address[i][1]);

                    /*  Recalculate the first attribute size.  */
                    input->size[i][0] = inputCacheLineSize - offset;

                    /*  Calculate second attribute size.  */
                    input->size[i][1] = dataSize - (inputCacheLineSize - offset);

                    /*  Update statistics.  */
                    splitted->inc();
                }
            }
        }
    }
}

/*  Fills with read data an input attribute.  */
void StreamerLoader::loadAttribute(InputRequest *input, u32bit attr, u8bit *data)
{
    StreamData format;
    u32bit elemSize;
    u32bit offset;

    //  Check if the attribute is per-instance.
    if (streamFrequency[attributeMap[attr]] != 0)
    {
        //  Check if the per instance attribute cache requires to be updated.
        if (input->size[attr][0] != 0)
        {
            //  Copy the data to the per instance attribute cache.
            for(u32bit b = 0; b < 8; b++)
                *((u32bit *) &instanceAttribData[attr][b * 4]) = *((u32bit *) &data[b * 4]);                           
        }
        else
        {
            //  Get the data from the per instance attribute cache.
            for(u32bit b = 0; b < 8; b++)
                *((u32bit *) &data[b * 4]) = *((u32bit *) &instanceAttribData[attr][b * 4]);
        }
    }
    
    /*  Get attribute data format.  */
    format = streamData[attributeMap[attr]];

    /*  Get the size in bytes of an element.  */
    elemSize = streamDataSize[streamData[attributeMap[attr]]];

    /*  NOTE: Access to the Input Cache is 32 bit aligned (4 bytes) so there may be
        some padding in the attribute buffer.  */

    /*  Calculate start offset for the attribute data inside the buffer.  */
    offset = input->address[attr][0] & 0x03;

    /*  Determine the number of attribute elements that are being loaded.  */
    switch(streamElements[attributeMap[attr]])
    {
        case 0:
            panic("StreamerLoader", "loadAttribute", "Attribute with zero elements can not request loads!.");
            break;

        case 1:
            /*  Only 1 attribute element.  */
            input->attributes[attr][0] = attributeDataConvert(format, &data[offset]);

            /*  Load the default value in the other attribute elements.  */
            input->attributes[attr][1] = attrDefValue[attr][1];
            input->attributes[attr][2] = attrDefValue[attr][2];
            input->attributes[attr][3] = attrDefValue[attr][3];

            break;

        case 2:
            /*  Only 2 attribute elements.  */
            input->attributes[attr][0] = attributeDataConvert(format, &data[offset]);
            input->attributes[attr][1] = attributeDataConvert(format, &data[offset + elemSize]);

            /*  Load the default value in the other attribute elements.  */
            input->attributes[attr][2] = attrDefValue[attr][2];
            input->attributes[attr][3] = attrDefValue[attr][3];

            break;

        case 3:
            /*  Only 3 attribute elements.  */
            input->attributes[attr][0] = attributeDataConvert(format, &data[offset]);
            input->attributes[attr][1] = attributeDataConvert(format, &data[offset + elemSize]);
            input->attributes[attr][2] = attributeDataConvert(format, &data[offset + elemSize * 2]);

            /*  Load the default value in the other attribute elements.  */
            input->attributes[attr][3] = attrDefValue[attr][3];

            break;

        case 4:
            /*  Only 2 attribute elements.  */
            input->attributes[attr][0] = attributeDataConvert(format, &data[offset]);
            input->attributes[attr][1] = attributeDataConvert(format, &data[offset + elemSize]);
            input->attributes[attr][2] = attributeDataConvert(format, &data[offset + elemSize * 2]);
            input->attributes[attr][3] = attributeDataConvert(format, &data[offset + elemSize * 3]);
        break;
    }
    
    //  Check if the D3D9 color order for the color components must be used.
    if (d3d9ColorStream[attributeMap[attr]])
    {
        //
        //  The D3D9 color formats are stored in little endian order with the alpha in highest byte:
        //
        //  For example:
        //
        //     D3DFMT_A8R8G8B8 is stored as B G R A
        //     D3DFMT_X8R8G8B8 is stored as B G R X
        //
        
        f32bit red = input->attributes[attr][2];
        f32bit green = input->attributes[attr][1];
        f32bit blue= input->attributes[attr][0];
        f32bit alpha = input->attributes[attr][3];

        input->attributes[attr][0] = red;
        input->attributes[attr][1] = green;
        input->attributes[attr][2] = blue;
        input->attributes[attr][3] = alpha;
    }
}

void StreamerLoader::setValidationMode(bool enable)
{
    validationMode = enable;
}

VertexInputMap &StreamerLoader::getVertexInputInfo()
{
    nextReadLog = (nextReadLog + 1) & 0x03;
    
    return vertexInputLog[nextReadLog];
}

