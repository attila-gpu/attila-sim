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
 * $RCSfile: extractTraceRegion.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2007-09-21 14:56:37 $
 *
 * extractTraceRegion implementation file
 *
 */


/**
 *
 *  @file extractTraceRegion.cpp
 *
 *  This file contains definitions the tool that extracts AGP transactions for a frame region of
 *  the input tracefile.
 *
 */


#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include "RegisterWriteBufferAGP.h"
#include "extractTraceRegion.h"
#include "AGPTraceFile.h"

using namespace std;
using namespace gpu3d;

RegisterWriteBufferAGP regWriteBuffer;
RegisterWriteBufferAGP regionStartRegisterCheckpoint;
         
multimap<UploadRegionIdentifier, UploadRegion> uploadRegions;

typedef multimap<UploadRegionIdentifier, UploadRegion>::iterator UploadRegionsIterator;
typedef multimap<UploadRegionIdentifier, UploadRegion>::value_type UploadRegionsValueType;

map<u32bit, UploadRegion> touchedUploadRegions;


ShaderProgramMap fragmentShaderPrograms[SHADER_PROGRAM_HASH_SIZE];
ShaderProgramMap vertexShaderPrograms[SHADER_PROGRAM_HASH_SIZE];

u8bit fragmentShaderCache[ShaderProgram::MAXSHADERINSTRUCTIONS * ShaderProgram::SHADERINSTRUCTIONSIZE];
u8bit vertexShaderCache[ShaderProgram::MAXSHADERINSTRUCTIONS * ShaderProgram::SHADERINSTRUCTIONSIZE];

u32bit fragmentProgramPC = 0;
u32bit fragmentProgramAddress = 0;
u32bit fragmentProgramSize = 0;

u32bit vertexProgramPC = 0;
u32bit vertexProgramAddress = 0;
u32bit vertexProgramSize = 0;

u32bit verboseLevel = 1;

#define VERBOSE(level, expr) if ((level) < verboseLevel) { expr }

bool checkAndCopyAGPTraceHeader(gzifstream *in, gzofstream *out, u32bit startFrame, u32bit extractFrames)
{
    AGPTraceFileHeader agpTraceHeader;
    
    in->read((char *) &agpTraceHeader, sizeof(agpTraceHeader));

    string str = agpTraceHeader.signature;

    if (!str.compare(AGPTRACEFILE_SIGNATURE))
    {
        agpTraceHeader.parameters.startFrame += startFrame;
        agpTraceHeader.parameters.traceFrames = extractFrames;
        out->write((char *) &agpTraceHeader, sizeof(agpTraceHeader));
        
        return true;
    }
    else
        return false;
}

//  Inserts a new upload region in the upload region list
void insertUploadRegion(u32bit address, u32bit size, u32bit agpTransID, u32bit md)
{
    bool found = false;
    bool end = false;
    
    u32bit newRegionStart;
    u32bit newRegionEnd;
    u32bit regionStart;
    u32bit regionEnd;

    //  Compute the range of the new upload region.
    newRegionStart = address;
    newRegionEnd = address + size - 1;
    
    pair<UploadRegionsIterator, UploadRegionsIterator> relatedRegions;
    
    //  Search the upload regions list for all the regions that are related with the new region.
    relatedRegions = uploadRegions.equal_range(UploadRegionIdentifier(newRegionStart, newRegionEnd));
    
    UploadRegionsIterator currentRegion;
    
    //  Set iterator to the first region related with the new region.
    UploadRegionsIterator nextRegion = relatedRegions.first;
    
    //  Process all the upload regions related with the new upload region.
    while(nextRegion != relatedRegions.second)
    {
        //  Save current region iterator.
        currentRegion = nextRegion;
        
        //  Advance to next region.
        nextRegion++;        

        //  Get range of the current region.
        regionStart = (currentRegion->second).address;
        regionEnd = (currentRegion->second).address + (currentRegion->second).size - 1;
       
        //  Check if the new regions is the same than a previous region.
        if ((regionStart == address) && ((currentRegion->second).size == size))
        {
            //  Update the upload region with the identifier of the last AGP transaction 
            //  uploaded data to this region.
            (currentRegion->second).lastAGPTransID = agpTransID;
            (currentRegion->second).lastMD = md;
            
            //  No need to search any more regions.
            end = true;

            //  Region found in the upload region list.
            found = true;
        }        
        //  Check if the new region is inside an existing region.
        else if ((regionStart <= newRegionStart) && (regionEnd >= newRegionEnd))
        {
            //  Check if part of the existing region precedes the new region.
            if (regionStart > newRegionStart)
            {
                //  Create a preceding region.
                UploadRegion region(regionStart, newRegionStart - regionStart, (currentRegion->second).lastMD,
                                    (currentRegion->second).lastAGPTransID);
                UploadRegionIdentifier regionID(regionStart, newRegionStart - 1);
                
                //  Add the new splited region into the upload region list.
                uploadRegions.insert(UploadRegionsValueType(regionID, region));

                VERBOSE( 2,
                    cout << "Inserting Upload Region : startAddress = " << hex << regionID.startAddress;
                    cout << " endAddress = " << regionID.endAddress << dec << endl;
                )
            }
            
            //  Check if part of the existing continues after the new region.
            if (regionEnd > newRegionEnd)
            {
                //  Create new region after the new region.
                UploadRegion region(newRegionEnd + 1, regionEnd - newRegionEnd, (currentRegion->second).lastMD,
                                    (currentRegion->second).lastAGPTransID);
                                    
                UploadRegionIdentifier regionID(newRegionEnd + 1, regionEnd);
                
                //  Add the new splited region into the upload region list.
                uploadRegions.insert(UploadRegionsValueType(regionID, region));
                
                VERBOSE(2, 
                    cout << "Inserting Upload Region : startAddres = " << hex << regionID.startAddress;
                    cout << " endAddress = " << regionID.endAddress << dec << endl;
                )
            }
            
            //  Create new region
            UploadRegion region(address, size, md, agpTransID);
            UploadRegionIdentifier regionID(newRegionStart, newRegionEnd);
            
            //  Add new region
            uploadRegions.insert(UploadRegionsValueType(regionID, region));
                     
            VERBOSE(2,    
                cout << "Inserting Upload Region : startAddres = " << hex << regionID.startAddress;
                cout << " endAddress = " << regionID.endAddress << dec << endl;

                cout << "Deleting region : startAddres = " << hex << currentRegion->first.startAddress;
                cout << " endAddress = " << currentRegion->first.endAddress << dec << endl;
            )
            
            //  Delete current region.
            uploadRegions.erase(currentRegion);
                        
            //  No need to search for more regions
            end = true;
            
            //  Region found in the upload region list.
            found = true;
        }
        //  Check if existing region is inside the new region.
        else if ((newRegionStart <= regionStart) && (newRegionEnd >= regionEnd))
        {
            VERBOSE(2, 
                cout << "Deleting region : startAddres = " << hex << currentRegion->first.startAddress;
                cout << " endAddress = " << currentRegion->first.endAddress << dec << endl;
            )
                
            //  Delete current region.
            uploadRegions.erase(currentRegion);
        }
        //  Check if existing region is partly inside the new region (preceding)
        else if ((regionStart < newRegionStart) && (regionEnd > newRegionStart) && (regionEnd < newRegionEnd))
        {
            //  Create preceding region
            UploadRegion region(regionStart, newRegionStart - regionStart, (currentRegion->second).lastMD,
                                (currentRegion->second).lastAGPTransID);
            UploadRegionIdentifier regionID(regionStart, newRegionStart - 1);
            
            //  Add the new splited region into the upload region list.
            uploadRegions.insert(UploadRegionsValueType(regionID, region));

            VERBOSE(2,
                cout << "Inserting Upload Region : startAddres = " << hex << regionID.startAddress;
                cout << " endAddress = " << regionID.endAddress << dec << endl;

                cout << "Deleting region : startAddres = " << hex << currentRegion->first.startAddress;
                cout << " endAddress = " << currentRegion->first.endAddress << dec << endl;
            )
                        
            //  Delete current region.
            uploadRegions.erase(currentRegion);
        }
        //  Check if existing region is partly inside the new region (continues after the new region).
        else if ((regionStart > newRegionStart) && (regionStart < newRegionEnd) && (regionEnd > newRegionEnd))
        {
            //  Create partial region after the new region
            UploadRegion region(newRegionEnd + 1, regionEnd - newRegionEnd, (currentRegion->second).lastMD,
                                (currentRegion->second).lastAGPTransID);
                                
            UploadRegionIdentifier regionID(newRegionEnd + 1, regionEnd);
            
            //  Add the new splited region into the upload region list.
            uploadRegions.insert(UploadRegionsValueType(regionID, region));

            VERBOSE(2, 
                cout << "Inserting Upload Region : startAddres = " << hex << regionID.startAddress;
                cout << " endAddress = " << regionID.endAddress << dec << endl;

                cout << "Deleting region : startAddres = " << hex << currentRegion->first.startAddress;
                cout << " endAddress = " << currentRegion->first.endAddress << dec << endl;
            )
                        
            //  Delete current region.
            uploadRegions.erase(currentRegion);
        }
    }
        
    //  Insert new region
    if (!found)
    {
        //  Create new region
        UploadRegion region(address, size, md, agpTransID);
        UploadRegionIdentifier regionID(newRegionStart, newRegionEnd);

        //  Add new region to the region list.
        uploadRegions.insert(UploadRegionsValueType(regionID, region));
        
        VERBOSE(2, 
            cout << "Inserting Upload Region : startAddres = " << hex << regionID.startAddress;
            cout << " endAddress = " << regionID.endAddress << dec << endl;
        )
    }
}

//  Determines if the current upload region is the last one that determined the contents of the
//  region.
bool checkUploadRegion(u32bit address, u32bit size, u32bit md, u32bit agpTransID)
{
    bool found = false;
    
    u32bit newRegionStart = address;
    u32bit newRegionEnd = address + size - 1;

    pair<UploadRegionsIterator, UploadRegionsIterator> relatedRegions;
    
    //  Search the upload regions list for all the regions that are related with the new region.
    relatedRegions = uploadRegions.equal_range(UploadRegionIdentifier(newRegionStart, newRegionEnd));
    
    //  Set iterator to the first region related with the new region.
    UploadRegionsIterator nextRegion = relatedRegions.first;
    
    UploadRegionsIterator currentRegion;
    
    //  Process all the upload regions related with the new upload region.
    while ((nextRegion != relatedRegions.second) && !found)
    {    
        //  Save current region iterator.
        currentRegion = nextRegion;
        
        //  Advance to next region.
        nextRegion++;        
    
        //  Check if the agp transaction identifier for the current region is the same that the
        //  AGP transaction identifier of the region to check.
        found = ((currentRegion->second).lastAGPTransID == agpTransID);
    }

    return found;
}

//  Insert a new touched region.
void insertTouchedRegion(UploadRegion &upRegion)
{
    touchedUploadRegions.insert(make_pair(upRegion.lastMD, upRegion));
}

//  Check if an upload region is touched by the frames that are going to be extracted.
bool checkRegionTouched(u32bit md)
{
    return (touchedUploadRegions.find(md) != touchedUploadRegions.end());
}

//  Inserts a fragment shader program in the list of loaded fragment shader programs.
void insertFragmentProgram()
{
    //  Insert the fragment shader program that is being loaded into the loaded fragment shader program list.
    insertShaderProgram(fragmentProgramPC, fragmentProgramAddress, fragmentProgramSize, fragmentShaderPrograms);
}

//  Inserts a vertex shader program in the list of loaded vertex shader programs.
void insertVertexProgram()
{
    //  Insert the vertex shader program that is being loaded into the loaded vertex shader program list.
    insertShaderProgram(vertexProgramPC, vertexProgramAddress, vertexProgramSize, vertexShaderPrograms);
}

// Inserts a shader program in the list of loaded shaded programs
void insertShaderProgram(u32bit pc, u32bit address, u32bit size, ShaderProgramMap *shaderPrograms)
{
    //  Create region for the shader program being loaded.
    UploadRegionIdentifier programRegion(address, address + size - 1);
    
    //  Search the upload region list for the last upload to the region of data that is being loaded as shader program.
    pair<UploadRegionsIterator, UploadRegionsIterator> relatedRegions;
    
    //  Search the upload regions list for all the regions that are related with the program region.
    relatedRegions = uploadRegions.equal_range(programRegion);
    
    //  Check if an upload region for the shader program was found
    if (relatedRegions.first != relatedRegions.second)
    {    
        //  Get the identifier for the AGP transaction that loaded the upload region from where the shader program
        //  is being loaded.
        u32bit agpTransID = (relatedRegions.first->second).lastAGPTransID;
        
        //  Hash the AGP Transaction ID.
        u32bit hashKey = agpTransID % SHADER_PROGRAM_HASH_SIZE;
        
        //  Insert the loaded shader program with the AGP transaction
        shaderPrograms[hashKey].insert(make_pair(agpTransID, ShaderProgram(pc, address, size / ShaderProgram::SHADERINSTRUCTIONSIZE, agpTransID)));
        
        UploadRegionsIterator nextRegion = relatedRegions.first;
        nextRegion++;
        
        //  Check if data from more than one upload region is being loaded as shader program.
        if (nextRegion != relatedRegions.second)
        {
            VERBOSE(1,
                cout << "Program region : ";
                cout << " startAddress = " << hex << programRegion.startAddress;
                cout << " endAddress = " << hex << programRegion.endAddress;
                cout << endl;
            
                cout << "First region : ";
                cout << " startAddress = " << hex << relatedRegions.first->first.startAddress;
                cout << " endAddress = " << hex << relatedRegions.first->first.endAddress;
                cout << endl;

                cout << "Second region : ";
                cout << " startAddress = " << hex << nextRegion->first.startAddress;
                cout << " endAddress = " << hex << nextRegion->first.endAddress;
                cout << endl;
            )
                        
            cout << "extractTraceRegion::insertShaderProgram => Shader program is being loaded from data from more than one upload region.  Not supported." << endl;
            exit(-1);
        }
    }
    else
    {
        VERBOSE(1,
            cout << "Program region : ";
            cout << " startAddress = " << hex << programRegion.startAddress;
            cout << " endAddress = " << hex << programRegion.endAddress;
            cout << endl;
        )
        
        cout << "extractTraceRegion::insertShaderProgram => No upload region was found for the shader program load." << endl;
        exit(-1);
    }
}

//  Check if the current AGP transaction is loading a fragment program and upload the fragment program data in the
//  Fragment Shader cache if required.
void checkFragmentProgram(AGPTransaction *agpTrans, u32bit agpTransID)
{
    //  Check if the AGP transaction is loading a shader program and if so update the shader program cache.
    checkShaderProgram(agpTrans->getData(), agpTransID, fragmentShaderPrograms, fragmentShaderCache);
}

//  Check if the current AGP transaction is loading a vertex program and upload the vertex program data in the
//  Vertex Shader cache if required.
void checkVertexProgram(AGPTransaction *agpTrans, u32bit agpTransID)
{
    //  Check if the AGP transaction is loading a shader program and if so update the shader program cache.
    checkShaderProgram(agpTrans->getData(), agpTransID, vertexShaderPrograms, vertexShaderCache);
}

//  Check if the current AGP transaction is loading the data for a shader program.
void checkShaderProgram(u8bit *data, u32bit agpTransID, ShaderProgramMap *shaderPrograms, u8bit *shaderCache)
{
    //  Create hash key from the AGP transaction ID
    u32bit hashKey = agpTransID % SHADER_PROGRAM_HASH_SIZE;

    //  Search the current AGP transaction in the loaded shader program list.
    map<u32bit, ShaderProgram>::iterator shProg = shaderPrograms[hashKey].find(agpTransID);
    
    //  Check if the AGP transaction was found.
    if (shProg != shaderPrograms[hashKey].end())
    {
        //  Retrieve the information for the shader program load operation.
        u32bit size = shProg->second.size;
        u32bit pc = shProg->second.pc;
        
        VERBOSE(2, 
            cout << "Caching shader at PC : " << pc << " size (bytes) = " << size << endl;
            cout << "  Data : " << endl;
            for(u32bit i = 0; i < size; i++)
            {
               cout << hex << u32bit(data[i]);
               if ((i % 32) == 31)
                   cout << endl;
               else
                   cout << ", ";
            }
            cout << dec;
        )
        
        //  Copy data to shader program cache.
        memcpy(&shaderCache[pc * ShaderProgram::SHADERINSTRUCTIONSIZE], data, size);
    }
}

//  Creates and writes the AGP transactions that load the fragment shader programs at the start of the frame region to be extracted.
void loadFragmentShaderPrograms(gzofstream *outTrace)
{
    //  Load the shader programs in the fragment shader program cache.
    loadShaderPrograms(outTrace, fragmentShaderCache, GPU_FRAGMENT_PROGRAM, GPU_FRAGMENT_PROGRAM_PC, GPU_FRAGMENT_PROGRAM_SIZE,
                       GPU_LOAD_FRAGMENT_PROGRAM);
}

//  Creates and writes the AGP transactions that load the vertex shader programs at the start of the frame region to be extracted.
void loadVertexShaderPrograms(gzofstream *outTrace)
{
    //  Load the shader programs in the vertex shader program cache.
    loadShaderPrograms(outTrace, vertexShaderCache, GPU_VERTEX_PROGRAM, GPU_VERTEX_PROGRAM_PC, GPU_VERTEX_PROGRAM_SIZE,
                       GPU_LOAD_VERTEX_PROGRAM);
}

//  Creates and writes the AGP transactions that load the shader programs at the start of the frame region to be extracted.
void loadShaderPrograms(gzofstream *outTrace, u8bit *shaderProgramCache, GPURegister addressReg, GPURegister pcReg, GPURegister sizeReg,
                        GPUCommand loadCommand)
{
    //
    //  NOTE: In the current implementation the lower region of the GPU memory must be unused.
    //
    
    UploadRegionIdentifier loadRegion(0, ShaderProgram::MAXSHADERINSTRUCTIONS * ShaderProgram::SHADERINSTRUCTIONSIZE - 1);
    
    //  Search the upload region list for the region were are going to use for loading the shader programs.
    pair<UploadRegionsIterator, UploadRegionsIterator> relatedRegions;
    relatedRegions = uploadRegions.equal_range(loadRegion);
    
    //  Check if region was not used.
    if (relatedRegions.first == relatedRegions.second)
    {
        //  Create an AGP transaction to write the shader program data into the load region of GPU memory.
        AGPTransaction *agpTrans = new AGPTransaction(0, loadRegion.endAddress + 1, shaderProgramCache, 0);

        VERBOSE(2,
            cout << "Cached shader programs => Writing AGP transactions" << endl;
            agpTrans->dump();
        )
                
        //  Write agp transaction to the output tracefile.
        agpTrans->save(outTrace);
        
        //  Delete agp transaction.
        delete agpTrans;
        
        GPURegData data;        
        
        //  Create AGP transactions to set the GPU registers for the shader program load operation.
        
        //  Set shader program address register to the start of the load region.
        data.uintVal = loadRegion.startAddress;
        agpTrans = new AGPTransaction(addressReg, 0, data, 0);
        VERBOSE(2,
            agpTrans->dump();
        )
        agpTrans->save(outTrace);
        delete agpTrans;
        
        //  Set shader program PC register (load address in shader program memory) to 0.
        data.uintVal = 0;
        agpTrans = new AGPTransaction(pcReg, 0, data, 0);
        agpTrans->save(outTrace);        
        VERBOSE(2,
            agpTrans->dump();
        )
        delete agpTrans;
        
        //  Set shader program size register (size in bytes of the shader program to load) to the size of the shader program memory.
        data.uintVal = loadRegion.endAddress + 1;
        agpTrans = new AGPTransaction(sizeReg, 0, data, 0);
        agpTrans->save(outTrace);
        VERBOSE(2,
            agpTrans->dump();
        )
        delete agpTrans;
        
        //  Create an AGP transaction with the command that initiates the shader program load operation.
        agpTrans = new AGPTransaction(loadCommand);
        agpTrans->save(outTrace);
        VERBOSE(2,
            agpTrans->dump();
        )
        delete agpTrans;
    }
    else
    {
        VERBOSE(1,
            cout << "Load region : ";
            cout << " startAddress = " << hex << loadRegion.startAddress;
            cout << " endAddress = " << hex << loadRegion.endAddress;
            cout << endl;
        
            cout << "First region : ";
            cout << " startAddress = " << hex << relatedRegions.first->first.startAddress;
            cout << " endAddress = " << hex << relatedRegions.first->first.endAddress;
            cout << endl;

            cout << "Second region : ";
            cout << " startAddress = " << hex << relatedRegions.second->first.startAddress;
            cout << " endAddress = " << hex << relatedRegions.second->first.endAddress;
            cout << endl;
        )
        
        cout << "extractTraceRegion::loadShaderPrograms => Load region has been used." << endl;
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    //  Check number of parameters
    if (argc < 5)
    {
        cout << "Usage: " << endl << endl;
        cout << "extractTraceRegion <input filename> <output filename> <start frame> <frames in region>" << endl;
        exit(-1);
    }
    
    //  Get input and output file name parameters.
    string inputTracename = argv[1];
    string outputTracename = argv[2];
    
    u32bit startFrame;
    u32bit regionFrames;

    stringstream converterStream;
    
    converterStream.str(argv[3]);

    //  Get start frame parameter.
    converterStream >> startFrame;
    
    converterStream.clear();
    converterStream.str(argv[4]);
    
    //  Get end frame parameter.
    converterStream >> regionFrames;
    
    //  Check parameters.
    if (regionFrames == 0)
    {
        cout << "Error: The number of frames in the region to extract must be larger than 0" << endl;
        exit(-1);
    }
    
    //  Try to open the input file.
    gzifstream inputTrace(inputTracename.c_str());
    
    //  Check if it's open
    if (!inputTrace.is_open())    
    {
        cout << "Error: Could not open input tracefile \'" << inputTracename << "\'" << endl;
        exit(-1);
    }
    
    //  Try to open the output file.
    gzofstream outputTrace(outputTracename.c_str());
    
    //  Check if it's open
    if (!outputTrace.is_open())    
    {
        cout << "Error: Could not create output tracefile \'" << outputTracename << "\'" << endl;
        exit(-1);
    }

    //  Check and copy input tracefile header (trace parameters).
    if (!checkAndCopyAGPTraceHeader(&inputTrace, &outputTrace, startFrame, regionFrames))
    {
        cout << "Error: Input file isn't an ATTILA AGP transaction trace file" << endl;
    }

    //  Initialize the optimized dynamic memory system.
    OptimizedDynamicMemory::initialize(256,
                                       1024,
                                       512,
                                       1024,
                                       1024,
                                       1024);

    // Initialize frame counter.
    u32bit currentFrame = 0;
    
    // Initialize AGP transaction counte.r
    u32bit agpTransID = 0;
    
    cout << "Input AGP Tracefile : " << inputTracename << endl;
    cout << "Output AGP Tracefile : " << outputTracename << endl;
    cout << "Region of frames to extract : ( " << startFrame << ", " << (startFrame + regionFrames - 1) << " )" << endl;
    cout << endl;
    cout << "Starting first pass" << endl;
    cout << endl;
    cout << "Processing frames preceding the frame region" << endl;
    
    char buff[32];
    sprintf(buff, "Current frame : %5d", currentFrame);
    cout << buff;
    cout.flush();
    
    //  Read and process transactions until the start of the region to extract.
    while(currentFrame < startFrame)
    {
        AGPTransaction *nextAGPTransaction;
        
        //  Read the next AGP transaction
        nextAGPTransaction = new AGPTransaction(&inputTrace);
     
        VERBOSE(2,
            cout << "Reading Transaction => Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
            nextAGPTransaction->dump();
        )
                   
        //  In this first pass through the frames preceding the region to extract the writes to the 
        //  GPU register must be buffered and the ranges of the data uploads must be stored to keep
        //  the position of the last update to that region.
        
        //  Determine how to process this AGP transaction.
        switch(nextAGPTransaction->getAGPCommand())
        {
            case AGP_REG_WRITE:
            
                //  Write register in the register write buffer.
                regWriteBuffer.writeRegister(nextAGPTransaction->getGPURegister(),
                                             nextAGPTransaction->getGPUSubRegister(),
                                             nextAGPTransaction->getGPURegData(),
                                             nextAGPTransaction->getMD());
                                             
                //  Check for vertex and fragment program related registers.
                switch(nextAGPTransaction->getGPURegister())
                {
                    case GPU_FRAGMENT_PROGRAM:

                        fragmentProgramAddress = nextAGPTransaction->getGPURegData().uintVal;
                        
                        break;
                        
                    case GPU_FRAGMENT_PROGRAM_PC:

                        fragmentProgramPC = nextAGPTransaction->getGPURegData().uintVal;
                        break;
                        
                    case GPU_FRAGMENT_PROGRAM_SIZE:

                        fragmentProgramSize = nextAGPTransaction->getGPURegData().uintVal;
                        break;

                    case GPU_VERTEX_PROGRAM:

                        vertexProgramAddress = nextAGPTransaction->getGPURegData().uintVal;
                        
                        break;
                        
                    case GPU_VERTEX_PROGRAM_PC:

                        vertexProgramPC = nextAGPTransaction->getGPURegData().uintVal;
                        break;
                        
                    case GPU_VERTEX_PROGRAM_SIZE:

                        vertexProgramSize = nextAGPTransaction->getGPURegData().uintVal;
                        break;
                }
                
                break;
                
            case AGP_WRITE:
            
                //  Insert uploaded data region into the upload region list.
                insertUploadRegion(nextAGPTransaction->getAddress(),
                                   nextAGPTransaction->getSize(),
                                   agpTransID,
                                   nextAGPTransaction->getMD()
                                   );
                
                break;
                
            case AGP_COMMAND:
            
                //  Check for swap GPU command.
                if(nextAGPTransaction->getGPUCommand() == GPU_SWAPBUFFERS)
                {
                    currentFrame++;
                    
                    cout << '\b' << '\b' << '\b' << '\b' << '\b';
                    sprintf(buff, "%5d", currentFrame);
                    cout << buff;
                    cout.flush();
                }
                
                //  Check for Fragment Program upload commands.
                if(nextAGPTransaction->getGPUCommand() == GPU_LOAD_FRAGMENT_PROGRAM)
                {
                    //  Insert a new fragment program in the fragment program list.
                    insertFragmentProgram();
                }
                
                //  Check for Vertex Program upload commands.
                if(nextAGPTransaction->getGPUCommand() == GPU_LOAD_VERTEX_PROGRAM)
                {
                    //  Insert a new vertex program in the vertex program list.
                    insertVertexProgram();
                }   
                             
                break;
        }
        
        //  Update AGP transaction counter.
        agpTransID++;
        
        //  Delete processed AGP Transaction.
        delete nextAGPTransaction;
    }
    
    cout << endl;
    cout << endl;
    cout << "End of preceding region" << endl;
    cout << "AGP transactions processed : " << (agpTransID - 1) << endl;
    cout << endl;
    cout << "Processing frames inside the region to extract" << endl;
    sprintf(buff, "Current frame : %5d", currentFrame);
    cout << buff;
    cout.flush();

    //  Create a checkpoint of the register writes at the start of the region of frames to extract
    //  from the AGP trace.
    regionStartRegisterCheckpoint = regWriteBuffer;

    //  Keep the identifier of the first AGP transaction inside the frame region to extract.
    u32bit firstAGPTransactionInRegion = agpTransID;
        
    //  Read and process the AGP transactions in the frame region to extract.
    while(currentFrame < (startFrame + regionFrames))
    {
        AGPTransaction *nextAGPTransaction;
        
        //  Read the next AGP transaction
        nextAGPTransaction = new AGPTransaction(&inputTrace);
        
        VERBOSE(2,
            cout << "Reading Transaction => Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
            nextAGPTransaction->dump();
        )

        //  In this first pass through the frames inside the region of frames to extract
        //  we store the memory descriptors of the upload descriptors that are touched by the
        //  rendering commands.
        
        //  Determine how to process this AGP transaction.
        switch(nextAGPTransaction->getAGPCommand())
        {
            case AGP_REG_WRITE:

                //  Write register in the register write buffer.
                regWriteBuffer.writeRegister(nextAGPTransaction->getGPURegister(),
                                             nextAGPTransaction->getGPUSubRegister(),
                                             nextAGPTransaction->getGPURegData(),
                                             nextAGPTransaction->getMD());
                                             
                break;
                
            case AGP_WRITE:
                
                // Nothing to do.

                break;

            case AGP_COMMAND:
            
                //  Check for swap GPU command.
                if(nextAGPTransaction->getGPUCommand() == GPU_SWAPBUFFERS)
                {
                    currentFrame++;
                    
                    cout << '\b' << '\b' << '\b' << '\b' << '\b';
                    sprintf(buff, "%5d", currentFrame);
                    cout << buff;
                    cout.flush();
                }
                
                //  Check for LOAD FRAGMENT PROGRAM command
                if (nextAGPTransaction->getGPUCommand() == GPU_LOAD_FRAGMENT_PROGRAM)
                {
                    bool found;
                    u32bit md;
                    GPURegData data;
                    
                    //  Search the last update of the fragment program load address register.
                    found = regWriteBuffer.readRegister(GPU_FRAGMENT_PROGRAM, 0, data, md);
                    
                    //  Check if register was found in the write register buffer.
                    if(found)
                    {
                        //  Add memory descriptor to the list of touched memory regions.
                        UploadRegion upRegion(data.uintVal, 0, md, agpTransID);
                        
                        //  Add touched region.
                        insertTouchedRegion(upRegion);
                    }
                }
                
                //  Check for LOAD VERTEX PROGRAM command
                if (nextAGPTransaction->getGPUCommand() == GPU_LOAD_VERTEX_PROGRAM)
                {
                    bool found;
                    u32bit md;
                    GPURegData data;
                    
                    //  Search the last update of the vertex program load address register.
                    found = regWriteBuffer.readRegister(GPU_VERTEX_PROGRAM, 0, data, md);
                    
                    //  Check if register was found in the write register buffer.
                    if(found)
                    {
                        //  Add memory descriptor to the list of touched memory regions.
                        UploadRegion upRegion(data.uintVal, 0, md, agpTransID);
                        
                        //  Add touched region.
                        insertTouchedRegion(upRegion);
                    }
                }

                //  Check for swap DRAW command.
                if(nextAGPTransaction->getGPUCommand() == GPU_DRAW)
                {
                    //  Process all texture units.
                    for(u32bit tu = 0; tu < MAX_TEXTURES; tu++)
                    {
                        GPURegData data;
                        u32bit md;
                        bool found; 
                        
                        //  Get the enable flag for the texture unit.
                        found = regWriteBuffer.readRegister(GPU_TEXTURE_ENABLE, tu, data, md);
                        
                        //  Check if register was found and texture unit is enabled.
                        if (found && data.booleanVal)
                        {
                            //  Process all the regions and descriptors associated with the texture unit.
                            for(u32bit level = 0; level < MAX_TEXTURE_SIZE; level++)
                            {
                                for(u32bit cube = 0; cube < CUBEMAP_IMAGES; cube++)
                                {
                                    bool valid;

                                    //  Compute texture address register index
                                    u32bit regIndex = tu * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + level * CUBEMAP_IMAGES + cube;

                                    //  Retrieve the data and memory descriptors associated with this register.
                                    valid = regWriteBuffer.readRegister(GPU_TEXTURE_ADDRESS, regIndex, data, md);
                                    
                                    //  Check if the register was found in the cache and the data is valid.
                                    if (valid && (md != 0))
                                    {
                                        //  Store the region and memory descriptor.
                                        UploadRegion upRegion(data.uintVal, 0, md, agpTransID);
                                        
                                        //  Add touched region.
                                        insertTouchedRegion(upRegion);
                                    }
                                }
                            }
                        }
                    }

                    //  Process all the streams
                    for(u32bit stream = 0; stream < MAX_STREAM_BUFFERS; stream++)
                    {
                        GPURegData data;
                        u32bit md;
                        bool found;
                        
                        //  Get the address register for the stream.
                        found = regWriteBuffer.readRegister(GPU_STREAM_ADDRESS, stream, data, md);
                        
                        //  Check if the register was found and the data is valid.
                        if (found && (md != 0))
                        {
                            //  Store the region and memory descriptor.
                            UploadRegion upRegion(data.uintVal, 0, md, agpTransID);
                            
                            //  Add touched region.
                            insertTouchedRegion(upRegion);
                        }
                    }
                }

                break;
        }
        
        //  Update AGP transaction counter.
        agpTransID++;
        
        //  Delete processed AGP Transaction.
        delete nextAGPTransaction;
    }
    
    //  Store the identifier of the last AGP transaction in the frame region.
    u32bit lastAGPTransactionInRegion = agpTransID--;
    
    cout << endl;
    cout << endl;
    cout << "End of frame region to extract" << endl;
    cout << "AGP transactions processed: " << agpTransID - firstAGPTransactionInRegion << endl;
    cout << endl;
    cout << "Upload Regions : " << uploadRegions.size() << endl;
    cout << "Touched Regions : " << touchedUploadRegions.size() << endl;
    
    u32bit vshPrograms = 0;
    u32bit fshPrograms = 0;
    
    //  Compute total size of the Shader Program lists
    for(u32bit i = 0; i < SHADER_PROGRAM_HASH_SIZE; i++)
    {
        vshPrograms += vertexShaderPrograms[i].size();
        fshPrograms += fragmentShaderPrograms[i].size();
    }
    
    cout << "Vertex Shader Programs : " << vshPrograms << endl;
    cout << "Fragment Shader Programs : " << fshPrograms << endl;
    cout << endl;
    cout << "First pass finished" << endl;
    cout << endl;
    
    //
    //  First pass finished.
    //
    
    //  Close input file.
    inputTrace.close();
    
    //  Close output file.
    outputTrace.close();
    
    //
    //  Start second pass.    
    //
    
    cout << "Starting second pass" << endl;
    cout << endl;
    
    //  Try to open the input file.
    inputTrace.open(inputTracename.c_str(), ios::binary | ios::in);
    
    //  Check if it's open
    if (!inputTrace.is_open())    
    {
        cout << "Error: Could not open input tracefile \'" << inputTracename << "\'" << endl;
        exit(-1);
    }

    //  Try to open the output file.
    outputTrace.open(outputTracename.c_str(), ios::binary | ios::out);
    
    //  Check if it's open
    if (!outputTrace.is_open())    
    {
        cout << "Error: Could not create output tracefile \'" << outputTracename << "\'" << endl;
        exit(-1);
    }

    //  Check and copy input tracefile header (trace parameters).
    if (!checkAndCopyAGPTraceHeader(&inputTrace, &outputTrace, startFrame, regionFrames))
    {
        cout << "Error: Input file isn't an ATTILA AGP transaction trace file" << endl;
    }

    //  Reset the fragment and vertex shader program caches.
    memset(fragmentShaderCache, 0, sizeof(fragmentShaderCache));
    memset(vertexShaderCache, 0, sizeof(vertexShaderCache));
    
    // Initialize frame counter.
    currentFrame = 0;
    
    // Initialize AGP transaction counter.
    agpTransID = 0;
    
    //  Initialize counter for the AGP transactions written into the output AGP trace file.
    u32bit outputAGPTransactions = 0;

    cout << "Processing frames preceding the frame region" << endl;
    
    sprintf(buff, "Current frame : %5d", currentFrame);
    cout << buff;
    cout.flush();
    
    //  Read and process transactions until the start of the region to extract.
    while(currentFrame < startFrame)
    {
        AGPTransaction *nextAGPTransaction;
        
        //  Read the next AGP transaction
        nextAGPTransaction = new AGPTransaction(&inputTrace);
        
        VERBOSE(2,
            cout << "Reading Transaction => Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
            nextAGPTransaction->dump();
        )

        //  In this second pass through the frames preceding the region to extract keep only the
        //  transactions that upload data that is later used in the region of frames to extract.
        
        //  Determine how to process this AGP transaction.
        switch(nextAGPTransaction->getAGPCommand())
        {
            case AGP_REG_WRITE:
            
                //  Nothing to do.
                
                break;
                
            case AGP_WRITE:
            
                //  Search if this upload region is touched by the frames in the region to extract.
                if (checkRegionTouched(nextAGPTransaction->getMD()))
                {
                    //  Search for the last updates to the related regions before the start of the frame region to be extracted.
                    if (checkUploadRegion(nextAGPTransaction->getAddress(),
                                          nextAGPTransaction->getSize(), 
                                          nextAGPTransaction->getMD(),
                                          agpTransID))
                    {
                        //  Force AGP Transaction to preload mode.
                        nextAGPTransaction->forcePreload();
                        
                        //  Save the AGP transaction that uploads the touched region of uploaded data.
                        nextAGPTransaction->save(&outputTrace);
                        
                        VERBOSE(2,
                            cout << "Writing Transaction => Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
                            nextAGPTransaction->dump();
                        )
                                                
                        //  Update number of AGP transactions written into the output AGP trace file.
                        outputAGPTransactions++;
                    }
                }
                
                //  Search in the loaded fragment program lists for AGP transactions that load shader program data.
                checkVertexProgram(nextAGPTransaction, agpTransID);
                checkFragmentProgram(nextAGPTransaction, agpTransID);
                
                break;
                
            case AGP_COMMAND:
            
                //  Check for swap GPU command.
                if(nextAGPTransaction->getGPUCommand() == GPU_SWAPBUFFERS)
                {
                    currentFrame++;
                    
                    cout << '\b' << '\b' << '\b' << '\b' << '\b';
                    sprintf(buff, "%5d", currentFrame);
                    cout << buff;
                    cout.flush();
                }
        }
        
        //  Update AGP transaction counter.
        agpTransID++;
        
        //  Delete processed AGP Transaction.
        delete nextAGPTransaction;
    }

    cout << endl;
    cout << endl;
    cout << "End of preceding region" << endl;
    cout << "AGP transactions processed : " << (agpTransID - 1) << endl;
    cout << "AGP transactions written : " << outputAGPTransactions << endl;
    cout << endl;

    //  Create AGP transactions to load the cache shader programs at the start of the region of frames to extract.
    loadFragmentShaderPrograms(&outputTrace);
    loadVertexShaderPrograms(&outputTrace);

    outputAGPTransactions += 5 * 2;
    
    cout << "Generated AGP transactions to load shader programs at the start of the frame region" << endl;
    cout << "AGP transactions written : " << outputAGPTransactions << endl;
    cout << endl;    
    
    //  Reset found flag to enter in the loop.
    bool found = true ;
    
    //  Create AGP transactions for all the GPU registers set at the start of the region of frames to extract.
    while(found)
    {
        GPURegister gpuReg;
        u32bit index;
        GPURegData data;
        u32bit md;
        
        //  Retrieve the next register write from the buffer.
        found = regionStartRegisterCheckpoint.flushNextRegister(gpuReg, index, data, md);
        
        //  Check if a register write was found in the buffer.
        if (found)
        {
            //  Create a new AGP transaction for writing the register into the GPU.
            AGPTransaction *nextAGPTransaction = new AGPTransaction(gpuReg, index, data, md);
            
            //  Save the AGP transaction that writes the register into the GPU.
            nextAGPTransaction->save(&outputTrace);
            
            VERBOSE(2,
                cout << "Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
                nextAGPTransaction->dump();
            )
                        
            //  Update number of AGP transactions written into the output AGP trace file.
            outputAGPTransactions++;
            
            //  Delete AGP transation.
            delete nextAGPTransaction;
        }
    }

    cout << "Generated AGP transactions to load GPU registers at the start of the frame region" << endl;
    cout << "AGP transactions written : " << outputAGPTransactions << endl;
    cout << endl;
    
    cout << "Processing frames inside the region to extract" << endl;
    sprintf(buff, "Current frame : %5d", currentFrame);
    cout << buff;
    cout.flush();

    //  Copy all the transactions in the frame region that is being extracted to the output file.
    while(currentFrame < (startFrame + regionFrames))
    {
        AGPTransaction *nextAGPTransaction;
        
        //  Read the next AGP transaction
        nextAGPTransaction = new AGPTransaction(&inputTrace);
        
        VERBOSE(2,
            cout << "Reading Transaction => Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
            nextAGPTransaction->dump();
        )

        //  Check for end of frame.
        if (nextAGPTransaction->getAGPCommand() == AGP_COMMAND)
        {
            //  Check of end of frame.
            if (nextAGPTransaction->getGPUCommand() == GPU_SWAPBUFFERS)
            {
                currentFrame++;
                
                cout << '\b' << '\b' << '\b' << '\b' << '\b';
                sprintf(buff, "%5d", currentFrame);
                cout << buff;
                cout.flush();
            }
        }
        
        //  Save the AGP transaction inside the region.
        nextAGPTransaction->save(&outputTrace);
        
        VERBOSE(2,
            cout << "Writing Transaction => Frame : " << currentFrame << " | AGP Transaction : " << agpTransID << endl;        
            nextAGPTransaction->dump();
        )
        
        //  Update number of AGP transactions written into the output AGP trace file.
        outputAGPTransactions++;
        
        //  Update AGP transaction counter.
        agpTransID++;
        
        //  Delete AGP transation.
        delete nextAGPTransaction;
    }

    cout << endl;
    cout << endl;
    cout << "End of frame region to extract" << endl;
    cout << "AGP transactions processed : " << (agpTransID - 1) << endl;
    cout << "AGP transactions written : " << outputAGPTransactions << endl;
    cout << endl;
    cout << "Second pass finished" << endl;
    cout << "Closing" << endl;
        
    //
    //  End of second phase.
    //
    
    //  Close input file.
    inputTrace.close();
    
    //  Close output file
    outputTrace.close();
    
}

