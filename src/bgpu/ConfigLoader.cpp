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
 * $RCSfile: ConfigLoader.cpp,v $
 * $Revision: 1.40 $
 * $Author: cgonzale $
 * $Date: 2008-06-02 17:37:34 $
 *
 * ConfigLoader implementation file.
 *
 */

/**
 *
 *  @file ConfigLoader.cpp
 *
 *  This file implements the ConfigLoader class that is used
 *  to parse and load the simulator configuration.
 *
 *
 */

#include "ConfigLoader.h"
#include "support.h"
#include <cstring>
#include <cstdio>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace gpu3d;
using namespace std;

/**
 * Use free(line) to deallocate dynamic memory
 *
 * see "man getline"
 */
int ConfigLoader::getline(char **lineptr, size_t *n, FILE *file)
{
    if ( feof(file) )
        return -1;

    vector<char> linev;
    while ( !feof(file) )
    {
        char c = fgetc(file);
        linev.push_back(c);
        if ( c == '\n' )
            break;
    }

    if ( feof(file) )
        return -1;


    // update buffer size
    *n = (size_t)linev.size() + 1;

    if ( *lineptr == NULL )
        *lineptr = (char*)malloc(*n*sizeof(char));
    else
        *lineptr = (char*)realloc(*lineptr, *n*sizeof(char));

    u32bit i; // Copy contents
    for ( i = 0; i < linev.size(); i++ )
        (*lineptr)[i] = linev[i];
    (*lineptr)[i] = char(0);

    return static_cast<int>(linev.size()); // end of line is not included in the count
}

/*  ConfigLoader constructor.  */
ConfigLoader::ConfigLoader(char *filename) :

    Parser(NULL, 0)

{
    /*  Tries to open the config file.  */
    configFile = fopen(filename, "r");

    /*  Check if file was correctly opened.  */
    if (configFile == NULL)
        panic("ConfigLoader", "ConfigLoader", "Error opening configuration file.");
}

/*  Config Loader destructor.  */
ConfigLoader::~ConfigLoader()
{
    /*  Close the config file.  */
    fclose(configFile);
}

/*  Loads, parses and stores the simulator parameters from the
    configuration file.  */
void ConfigLoader::getParameters(SimParameters *simP)
{
    bool end;

    /*  Not yet the end of config file parsing.  */
    end = FALSE;

    /*  Read first line from the config file.  */
    if (getline(&str, &len, configFile) <= 0)
    {
            /*  Check if it is end of file.  */
            if (feof(configFile))
                end = TRUE;
            else
                panic("ConfigLoader", "getParameters", "Error while reading the configuration file.");

    }

    /*  Parse all the simulator sections.  */
    while (!end)
    {
        /*  Set line parse start position to 0.  */
        pos = 0;

        /*  First skip any initial spaces or tabulators.  */
        skipSpaces();

        /*  Second skip all the comment and empty lines.  */
        if ((str[pos] != '#') && (str[pos] != '\n'))
        {
            /*  Parse a section.  */
            if (!parseSection(simP))
            {
                panic("ConfigLoader", "getParameters", "Error parsing configuration file section.");
            }
        }
        else
        {
            /*  Read a new line from the config file.  */
            if (getline(&str, &len, configFile) <= 0)
            {
                /*  Check if it is end of file.  */
                if (feof(configFile))
                    end = TRUE;
                else
                    panic("ConfigLoader", "getParameters", "Error while reading the configuration file.");
            }
        }
    }

    paramsTracker.checkParamsIntegrity(*simP);
}

/*  Parse a configuration section.  */
bool ConfigLoader::parseSection(SimParameters *simP)
{
    char sectionName[100];
    Section section;

    /*  Check open section name character.  */
    if (str[pos] != '[')
        return FALSE;

    /*  Update position.  */
    pos++;

    /*  Check end of line.  */
    checkEndOfString

    /*  Try to parse section name.  */
    if (!parseId(sectionName))
        return FALSE;

    /*  Check end of the line.  */
    checkEndOfString

    /*  Check end section name character.  */
    if (str[pos] != ']')
        return FALSE;

    /*  Reset section.  */
    section = SEC_UKN;

    /*  Select section.  */

    if (!strcmp("SIMULATOR", sectionName))
    {
        section = SEC_SIM;
    }

    if (!strcmp("GPU", sectionName))
    {
        section = SEC_GPU;
    }

    if (!strcmp("COMMANDPROCESSOR", sectionName))
    {
        section = SEC_COM;
    }

    if (!strcmp("MEMORYCONTROLLER", sectionName))
    {
        section = SEC_MEM;
    }

    if (!strcmp("STREAMER", sectionName))
    {
        section = SEC_STR;
    }

    if (!strcmp("VERTEXSHADER", sectionName))
    {
        section = SEC_VSH;
    }

    if (!strcmp("PRIMITIVEASSEMBLY", sectionName))
    {
        section = SEC_PAS;
    }

    if (!strcmp("CLIPPER", sectionName))
    {
        section = SEC_CLP;
    }

    if (!strcmp("RASTERIZER", sectionName))
    {
        section = SEC_RAS;
    }

    if (!strcmp("FRAGMENTSHADER", sectionName))
    {
        section = SEC_FSH;
    }

    if (!strcmp("ZSTENCILTEST", sectionName))
    {
        section = SEC_ZST;
    }

    if (!strcmp("COLORWRITE", sectionName))
    {
        section = SEC_CWR;
    }

    if (!strcmp("DAC", sectionName))
    {
        section = SEC_DAC;
    }

    if (!strcmp("SIGNALS", sectionName))
    {
       section = SEC_SIG;

        /*  Reset signal counter.  */
        simP->numSignals = 0;
    }

    /*  Check if it was a known section.  */
    if (section == SEC_UKN)
        return FALSE;

    /*  Parse the section parameters.  */
    return parseSectionParameters(section, simP);
}

/*  Parse all the section parameters.  */
bool ConfigLoader::parseSectionParameters(Section section, SimParameters *simP)
{
    bool end;
    SigParameters *signalBuffer;
    SigParameters *auxSignalBuffer;
    u32bit allocSigParam;

    /*  Allocate some space for the signals for the Signal section.  */
    if (section == SEC_SIG)
    {
        /*  Allocate buffer for the signals.  */
        signalBuffer = new SigParameters[SIGNAL_BUFFER_SIZE];

        GPU_ASSERT(
            /*  Check allocation.  */
            if (signalBuffer == NULL)
                panic("ConfigLoader", "parseSectionParameters", "Error allocating buffer for the signal section.");

        )

        /*  Set allocated signals counter.  */
        allocSigParam = SIGNAL_BUFFER_SIZE;
    }

    /*  Not end of the section yet.  */
    end = FALSE;

    /*  Parse all the section parameters.  */
    while (!end)
    {
        /*  Set line start position.  */
        pos = 0;

        /*  Read next line.  */
        if (getline(&str, &len, configFile) > 0)
        {
            /*  First skip any initial space or tab.  */
            skipSpaces();

            /*  Second, skip empty lines and comments lines.  */
            if ((str[pos] != '#') && (str[pos] != '\n'))
            {
                /*  third, check is not the start of another section.  */
                if (str[pos] != '[')
                {
                    /*  Check end of line.  */
                    checkEndOfString

                    /*  Select section.  */
                    switch(section)
                    {
                        case SEC_SIM:

                            /*  Try to parse a Simulator parameter.  */
                            if (!parseSimSectionParameter(simP))
                                return FALSE;

                            break;

                        case SEC_GPU:

                            /*  Try to parse a GPU architecture parameter.  */
                            if (!parseGPUSectionParameter(&simP->gpu))
                                return FALSE;

                            break;

                        case SEC_COM:

                            /*  Try to parse a Command Processor parameter.  */
                            if (!parseComSectionParameter(&simP->com))
                                return FALSE;
                            break;

                        case SEC_MEM:

                            /*  Try to parse a Memory Controller parameter.  */
                            if (!parseMemSectionParameter(&simP->mem))
                                return FALSE;
                            break;

                        case SEC_STR:

                            /*  Try to parse a Streamer parameter.  */
                            if (!parseStrSectionParameter(&simP->str))
                                return FALSE;
                            break;

                        case SEC_VSH:

                            /*  Try to parse a Vertex Shader parameter.  */
                            if (!parseVShSectionParameter(&simP->vsh))
                                return FALSE;
                            break;

                        case SEC_PAS:

                            /*  Try to parse a Primitive Assembly parameter.  */
                            if (!parsePAsSectionParameter(&simP->pas))
                                return FALSE;
                            break;

                        case SEC_CLP:

                            /*  Try to parse a Clipper parameter.  */
                            if (!parseClpSectionParameter(&simP->clp))
                                return FALSE;
                            break;

                        case SEC_RAS:

                            /*  Try to parse a Rasterizer parameter.  */
                            if (!parseRasSectionParameter(&simP->ras))
                                return FALSE;
                            break;

                        case SEC_FSH:

                            /*  Try to parse a Fragment Shader parameter.  */
                            if (!parseFShSectionParameter(&simP->fsh))
                                return FALSE;
                            break;

                        case SEC_ZST:

                            /*  Try to parse a Z Stencil Test parameter.  */
                            if (!parseZSTSectionParameter(&simP->zst))
                                return FALSE;
                            break;

                        case SEC_CWR:

                            /*  Try to parse a Color Write parameter.  */
                            if (!parseCWRSectionParameter(&simP->cwr))
                                return FALSE;
                            break;

                        case SEC_DAC:

                            /*  Try to parse a DAC parameter.  */
                            if (!parseDACSectionParameter(&simP->dac))
                                return FALSE;
                            break;

                        case SEC_SIG:

                            /*  Check if the buffer with the read signal
                                parameters must be resized.  */
                            if (allocSigParam == simP->numSignals)
                            {
                                /*  Allocate more entries for signal parameters.  */
                                auxSignalBuffer = new SigParameters[allocSigParam + SIGNAL_BUFFER_SIZE];

                                /*  Check memory allocation.  */
                                GPU_ASSERT(
                                    if (auxSignalBuffer == NULL)
                                        panic("ConfigLoader", "parseSectionParameters", "Error reallocating signal buffer.");
                                )

                                /*  Copy the old buffer.  */
                                memcpy(auxSignalBuffer, signalBuffer, allocSigParam * sizeof(SigParameters));

                                /*  Delete old buffer.  */
                                delete signalBuffer;

                                /*  Move the new buffer.  */
                                signalBuffer = auxSignalBuffer;

                                /*  Update the allocated signal entries counter.  */
                                allocSigParam = allocSigParam + SIGNAL_BUFFER_SIZE;
                            }

                            /*  Try to parse a Signal parameter.  */
                            if (!parseSigSectionParameter(&signalBuffer[simP->numSignals]))
                                return FALSE;
                            else
                                simP->numSignals++;

                            break;
                    }
                }
                else
                {
                    /*  End of section.  */
                    end = TRUE;
                }
            }
        }
        else
        {
            /*  End of file or error.  */

            /*  Check end of file.  */
            if (feof(configFile))
                end = TRUE;
            else
                panic("ConfigLoader", "parseSimSection", "Error while reading config file.");
        }
    }

    /*  If is the Signal section the must signal parameters buffer
        must be moved to the simulator parameters.  */
    if (section == SEC_SIG)
    {
        simP->sig = signalBuffer;
    }

    return TRUE;
}

/*  Parse Simulator section parameter.  */
bool ConfigLoader::parseSimSectionParameter(SimParameters *simP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("SIMULATOR");

    if (!parseDecimalParameter("SimCycles", id, simP->simCycles))
        return FALSE;

    if (!parseDecimalParameter("SimFrames", id, simP->simFrames))
        return FALSE;

    if (!parseStringParameter("InputFile", id, simP->inputFile))
        return FALSE;

    if (!parseStringParameter("SignalDumpFile", id, simP->signalDumpFile))
        return FALSE;

    if (!parseStringParameter("StatsFile", id, simP->statsFile))
        return FALSE;

    if (!parseStringParameter("StatsFilePerFrame", id, simP->statsFilePerFrame))
        return FALSE;

    if (!parseStringParameter("StatsFilePerBatch", id, simP->statsFilePerBatch))
        return FALSE;

    if (!parseDecimalParameter("StartFrame", id, simP->startFrame))
        return FALSE;

    if (!parseDecimalParameter("StartSignalDump", id, simP->startDump))
        return FALSE;

    if (!parseDecimalParameter("SignalDumpCycles", id, simP->dumpCycles))
        return FALSE;

    if (!parseDecimalParameter("StatisticsRate", id, simP->statsRate))
        return FALSE;

    if (!parseBooleanParameter("DumpSignalTrace", id, simP->dumpSignalTrace))
        return FALSE;

    if (!parseBooleanParameter("Statistics", id, simP->statistics))
        return FALSE;

    if (!parseBooleanParameter("PerFrameStatistics", id, simP->perFrameStatistics))
        return FALSE;

    if (!parseBooleanParameter("PerBatchStatistics", id, simP->perBatchStatistics))
        return FALSE;

    if (!parseBooleanParameter("PerCycleStatistics", id, simP->perCycleStatistics))
        return FALSE;

    if (!parseBooleanParameter("DetectStalls", id, simP->detectStalls))
        return FALSE;

    if (!parseBooleanParameter("GenerateFragmentMap", id, simP->fragmentMap))
        return FALSE;

    if (!parseDecimalParameter("FragmentMapMode", id, simP->fragmentMapMode))
        return FALSE;

    if (!parseBooleanParameter("DoubleBuffer", id, simP->doubleBuffer))
        return FALSE;

    if (!parseBooleanParameter("ForceMSAA", id, simP->forceMSAA))
        return FALSE;

    if (!parseDecimalParameter("MSAASamples", id, simP->msaaSamples))
        return FALSE;

    if (!parseBooleanParameter("ForceFP16ColorBuffer", id, simP->forceFP16ColorBuffer))
        return FALSE;

    if (!parseBooleanParameter("EnableDriverShaderTranslation", id, simP->enableDriverShTrans))
        return FALSE;

    if (!parseDecimalParameter("ObjectSize0", id, simP->objectSize0))
        return FALSE;

    if (!parseDecimalParameter("BucketSize0", id, simP->bucketSize0))
        return FALSE;

    if (!parseDecimalParameter("ObjectSize1", id, simP->objectSize1))
        return FALSE;

    if (!parseDecimalParameter("BucketSize1", id, simP->bucketSize1))
        return FALSE;

    if (!parseDecimalParameter("ObjectSize2", id, simP->objectSize2))
        return FALSE;

    if (!parseDecimalParameter("BucketSize2", id, simP->bucketSize2))
        return FALSE;

    if (!parseBooleanParameter("UseACD", id, simP->useACD))
        return FALSE;


    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [SIMULATOR] is not supported";
        panic("ConfigLoader", "parseSimSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse GPU architecture parameter.  */
bool ConfigLoader::parseGPUSectionParameter(GPUParameters *gpuP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("GPU");

    /*  Parse decimal parameters.  */

    if (!parseDecimalParameter("NumVertexShaders", id, gpuP->numVShaders))
        return FALSE;

    if (!parseDecimalParameter("NumFragmentShaders", id, gpuP->numFShaders))
        return FALSE;

    if (!parseDecimalParameter("NumStampPipes", id, gpuP->numStampUnits))
        return FALSE;

    if (!parseDecimalParameter("GPUClock", id, gpuP->gpuClock))
        return FALSE;

    if (!parseDecimalParameter("ShaderClock", id, gpuP->shaderClock))
        return FALSE;

    if (!parseDecimalParameter("MemoryClock", id, gpuP->memoryClock))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [GPU] is not supported";
        panic("ConfigLoader", "parseGPUSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Command Processor.  */
bool ConfigLoader::parseComSectionParameter(ComParameters *comP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("COMMANDPROCESSOR");

    if (!parseBooleanParameter("PipelinedBatchRendering", id, comP->pipelinedBatches))
        return FALSE;

    if (!parseBooleanParameter("DumpShaderPrograms", id, comP->dumpShaderPrograms))
        return FALSE;


    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [COMMANDPROCESSOR] is not supported";
        panic("ConfigLoader", "parseSimSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Memory Controller parameters.  */
bool ConfigLoader::parseMemSectionParameter(MemParameters *memP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("MEMORYCONTROLLER");

    /*  Parse parameters.  */

    if (!parseBooleanParameter("MemoryControllerV2", id, memP->memoryControllerV2))
        return FALSE;

    if (!parseDecimalParameter("MemorySize", id, memP->memSize))
        return FALSE;

    if (!parseDecimalParameter("MemoryClockMultiplier", id, memP->clockMultiplier))
        return FALSE;

    if (!parseDecimalParameter("MemoryFrequency", id, memP->memoryFrequency))
        return FALSE;

    if (!parseDecimalParameter("MemoryBusWidth", id, memP->busWidth))
        return FALSE;

    if (!parseDecimalParameter("MemoryBuses", id, memP->memBuses))
        return FALSE;

    if (!parseBooleanParameter("SharedBanks", id, memP->sharedBanks))
        return FALSE;

    if (!parseDecimalParameter("BankGranurality", id, memP->bankGranurality))
        return FALSE;

    if (!parseDecimalParameter("BurstLength", id, memP->burstLength))
        return FALSE;

    if (!parseDecimalParameter("ReadLatency", id, memP->readLatency))
        return FALSE;

    if (!parseDecimalParameter("WriteLatency", id, memP->writeLatency))
        return FALSE;

    if (!parseDecimalParameter("WriteToReadLatency", id, memP->writeToReadLat))
        return FALSE;

    if (!parseDecimalParameter("MemoryPageSize", id, memP->memPageSize))
        return FALSE;

    if (!parseDecimalParameter("OpenPages", id, memP->openPages))
        return FALSE;

    if (!parseDecimalParameter("PageOpenLatency", id, memP->pageOpenLat))
        return FALSE;

    if (!parseDecimalParameter("MaxConsecutiveReads", id, memP->maxConsecutiveReads))
        return FALSE;

    if (!parseDecimalParameter("MaxConsecutiveWrites", id, memP->maxConsecutiveWrites))
        return FALSE;

    if (!parseDecimalParameter("CommandProcessorBusWidth", id, memP->comProcBus))
        return FALSE;

    if (!parseDecimalParameter("StreamerFetchBusWidth", id, memP->strFetchBus))
        return FALSE;

    if (!parseDecimalParameter("StreamerLoaderBusWidth", id, memP->strLoaderBus))
        return FALSE;

    if (!parseDecimalParameter("ZStencilBusWidth", id, memP->zStencilBus))
        return FALSE;

    if (!parseDecimalParameter("ColorWriteBusWidth", id, memP->cWriteBus))
        return FALSE;

    if (!parseDecimalParameter("DACBusWidth", id, memP->dacBus))
        return FALSE;

    if (!parseDecimalParameter("TextureUnitBusWidth", id, memP->textUnitBus))
        return FALSE;

    if (!parseDecimalParameter("MappedMemorySize", id, memP->mappedMemSize))
        return FALSE;

    if (!parseDecimalParameter("WriteBufferLines", id, memP->readBufferLines))
        return FALSE;

    if (!parseDecimalParameter("ReadBufferLines", id, memP->writeBufferLines))
        return FALSE;

    if (!parseDecimalParameter("RequestQueueSize", id, memP->reqQueueSz))
        return FALSE;

    if (!parseDecimalParameter("ServiceQueueSize", id, memP->servQueueSz))
        return FALSE;


    /* Parse Memory Controller V2 exclusive parameters */

    if (!parseBooleanParameter("V2UseChannelRequestFIFOPerBank", id, memP->v2UseChannelRequestFIFOPerBank))
        return FALSE;

    if (!parseDecimalParameter("V2ChannelRequestFIFOPerBankSelection", id, memP->v2ChannelRequestFIFOPerBankSelection))
        return FALSE;

    if (!parseBooleanParameter("V2MemoryTrace", id, memP->v2MemoryTrace))
        return FALSE;

    if (!parseDecimalParameter("V2MemoryChannels", id, memP->v2MemoryChannels))
        return FALSE;

    if (!parseDecimalParameter("V2BanksPerMemoryChannel", id, memP->v2BanksPerMemoryChannel))
        return FALSE;

    if (!parseDecimalParameter("V2MemoryRowSize", id, memP->v2MemoryRowSize))
        return FALSE;

    if (!parseDecimalParameter("V2BurstBytesPerCycle", id, memP->v2BurstBytesPerCycle))
        return FALSE;

    if (!parseDecimalParameter("V2ChannelInterleaving", id, memP->v2ChannelInterleaving))
        return FALSE;

    if (!parseDecimalParameter("V2BankInterleaving", id, memP->v2BankInterleaving))
        return FALSE;

    if (!parseBooleanParameter("V2SecondInterleaving", id, memP->v2SecondInterleaving))
       return FALSE;

    if (!parseDecimalParameter("V2SecondChannelInterleaving", id, memP->v2SecondChannelInterleaving))
        return FALSE;

    if (!parseDecimalParameter("V2SecondBankInterleaving", id, memP->v2SecondBankInterleaving))
        return FALSE;

    if (!parseDecimalParameter("V2SplitterType", id, memP->v2SplitterType))
        return FALSE;

    if (!parseStringParameter("V2ChannelInterleavingMask", id, memP->v2ChannelInterleavingMask))
        return FALSE;

    if (!parseStringParameter("V2BankInterleavingMask", id, memP->v2BankInterleavingMask))
        return FALSE;

    if (!parseStringParameter("V2SecondChannelInterleavingMask", id, memP->v2SecondChannelInterleavingMask))
        return FALSE;

    if (!parseStringParameter("V2SecondBankInterleavingMask", id, memP->v2SecondBankInterleavingMask))
        return FALSE;

    if (!parseStringParameter("V2BankSelectionPolicy", id, memP->v2BankSelectionPolicy))
        return FALSE;
    
    if (!parseDecimalParameter("V2MaxChannelTransactions", id, memP->v2MaxChannelTransactions))
        return FALSE;

    if (!parseDecimalParameter("V2DedicatedChannelReadTransactions", id, memP->v2DedicatedChannelReadTransactions))
        return FALSE;

    if (!parseDecimalParameter("V2ChannelScheduler", id, memP->v2ChannelScheduler))
        return FALSE;

    if (!parseDecimalParameter("V2PagePolicy", id, memP->v2PagePolicy))
        return FALSE;

    if ( !parseStringParameter("V2MemoryType", id, memP->v2MemoryType))
        return FALSE;

    if ( !parseStringParameter("V2GDDR_Profile", id, memP->v2GDDR_Profile))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_tRRD", id, memP->v2GDDR_tRRD))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_tRCD", id, memP->v2GDDR_tRCD))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_tWTR", id, memP->v2GDDR_tWTR))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_tRTW", id, memP->v2GDDR_tRTW))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_tWR", id, memP->v2GDDR_tWR))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_tRP", id, memP->v2GDDR_tRP))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_CAS", id, memP->v2GDDR_CAS))
        return FALSE;

    if (!parseDecimalParameter("V2GDDR_WL", id, memP->v2GDDR_WL))
        return FALSE;

    if (!parseDecimalParameter("V2SwitchModePolicy", id, memP->v2SwitchModePolicy))
        return FALSE;

    if (!parseDecimalParameter("V2ActiveManagerMode", id, memP->v2ActiveManagerMode))
        return FALSE;

    if (!parseDecimalParameter("V2PrechargeManagerMode", id, memP->v2PrechargeManagerMode))
        return FALSE;

    if (!parseBooleanParameter("V2DisablePrechargeManager", id, memP->v2DisablePrechargeManager))
        return FALSE;

    if (!parseBooleanParameter("V2DisableActiveManager", id, memP->v2DisableActiveManager))
        return FALSE;

    if (!parseDecimalParameter("V2ManagerSelectionAlgorithm", id, memP->v2ManagerSelectionAlgorithm))
        return FALSE;

    if (!parseStringParameter("V2DebugString", id, memP->v2DebugString))
        return FALSE;

    if (!parseBooleanParameter("V2UseClassicSchedulerStates", id, memP->v2UseClassicSchedulerStates))
        return FALSE;

    if (!parseBooleanParameter("V2UseSplitRequestBufferPerROP", id, memP->v2UseSplitRequestBufferPerROP))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [MEMORYCONTROLLER] is not supported";
        panic("ConfigLoader", "parseMemSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Streamer parameters.  */
bool ConfigLoader::parseStrSectionParameter(StrParameters *strP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("STREAMER");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("IndicesCycle", id, strP->indicesCycle))
        return FALSE;

    if (!parseDecimalParameter("IndexBufferSize", id, strP->idxBufferSize))
        return FALSE;

    if (!parseDecimalParameter("OutputFIFOSize", id, strP->outputFIFOSize))
        return FALSE;
 
    if (!parseDecimalParameter("OutputMemorySize", id, strP->outputMemorySize))
        return FALSE;

    if (!parseDecimalParameter("VerticesCycle", id, strP->verticesCycle))
        return FALSE;

    if (!parseDecimalParameter("AttributesSentCycle", id, strP->attrSentCycle))
        return FALSE;

    if (!parseDecimalParameter("StreamerLoaderUnits", id, strP->streamerLoaderUnits))
        return FALSE;

    if (!parseDecimalParameter("SLIndicesCycle", id, strP->slIndicesCycle))
        return FALSE;

    if (!parseDecimalParameter("SLInputRequestQueueSize", id, strP->slInputReqQueueSize))
        return FALSE;

    if (!parseDecimalParameter("SLAttributesCycle", id, strP->slAttributesCycle))
        return FALSE;

    if (!parseDecimalParameter("SLInputCacheLines", id, strP->slInCacheLines))
        return FALSE;

    if (!parseDecimalParameter("SLInputCacheLineSize", id, strP->slInCacheLineSz))
        return FALSE;

    if (!parseDecimalParameter("SLInputCachePortWidth", id, strP->slInCachePortWidth))
        return FALSE;

    if (!parseDecimalParameter("SLInputCacheRequestQueueSize", id, strP->slInCacheReqQSz))
        return FALSE;

    if (!parseDecimalParameter("SLInputCacheInputQueueSize", id, strP->slInCacheInputQSz))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [STREAMER] is not supported";
        panic("ConfigLoader", "parseStrSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Vertex Shader parameters.  */
bool ConfigLoader::parseVShSectionParameter(VShParameters *vshP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Parse parameters.  */

    paramsTracker.startParamSectionDefinition("VERTEXSHADER");

    if (!parseDecimalParameter("ExecutableThreads", id, vshP->numThreads))
        return FALSE;

    if (!parseDecimalParameter("InputBuffers", id, vshP->numInputBuffers))
        return FALSE;

    if (!parseDecimalParameter("ThreadResources", id, vshP->numResources))
        return FALSE;

    if (!parseDecimalParameter("ThreadRate", id, vshP->threadRate))
        return FALSE;

    if (!parseDecimalParameter("FetchRate", id, vshP->fetchRate))
        return FALSE;

    if (!parseDecimalParameter("ThreadGroup", id, vshP->threadGroup))
        return FALSE;

    if (!parseBooleanParameter("LockedExecutionMode", id, vshP->lockedMode))
        return FALSE;

    if (!parseBooleanParameter("ScalarALU", id, vshP->scalarALU))
        return FALSE;

    if (!parseBooleanParameter("ThreadWindow", id, vshP->threadWindow))
        return FALSE;

    if (!parseDecimalParameter("FetchDelay", id, vshP->fetchDelay))
        return FALSE;

    if (!parseBooleanParameter("SwapOnBlock", id, vshP->swapOnBlock))
        return FALSE;

    if (!parseDecimalParameter("InputsPerCycle", id, vshP->inputsCycle))
        return FALSE;

    if (!parseDecimalParameter("OutputsPerCycle", id, vshP->outputsCycle))
        return FALSE;

    if (!parseDecimalParameter("OutputLatency", id, vshP->outputLatency))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [VERTEXSHADER] is not supported";
        panic("ConfigLoader", "parseVShSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Primitive Assembly parameters.  */
bool ConfigLoader::parsePAsSectionParameter(PAsParameters *pasP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("PRIMITIVEASSEMBLY");

    if (!parseDecimalParameter("VerticesCycle", id, pasP->verticesCycle))
        return FALSE;

    if (!parseDecimalParameter("TrianglesCycle", id, pasP->trianglesCycle))
        return FALSE;

    if (!parseDecimalParameter("InputBusLatency", id, pasP->inputBusLat))
        return FALSE;

    if (!parseDecimalParameter("AssemblyQueueSize", id, pasP->paQueueSize))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [PRIMITIVEASSEMBLY] is not supported";
        panic("ConfigLoader", "parsePAsSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Clipper parameters.  */
bool ConfigLoader::parseClpSectionParameter(ClpParameters *clpP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("CLIPPER");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("TrianglesCycle", id, clpP->trianglesCycle))
        return FALSE;

    if (!parseDecimalParameter("ClipperUnits", id, clpP->clipperUnits))
        return FALSE;

    if (!parseDecimalParameter("StartLatency", id, clpP->startLatency))
        return FALSE;

    if (!parseDecimalParameter("ExecLatency", id, clpP->execLatency))
        return FALSE;

    if (!parseDecimalParameter("ClipBufferSize", id, clpP->clipBufferSize))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [CLIPPER] is not supported";
        panic("ConfigLoader", "parseClpSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Rasterizer parameters.  */
bool ConfigLoader::parseRasSectionParameter(RasParameters *rasP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("RASTERIZER");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("TrianglesCycle", id, rasP->trianglesCycle))
        return FALSE;

    if (!parseDecimalParameter("SetupFIFOSize", id, rasP->setupFIFOSize))
        return FALSE;

    if (!parseDecimalParameter("SetupUnits", id, rasP->setupUnits))
        return FALSE;

    if (!parseDecimalParameter("SetupLatency", id, rasP->setupLat))
        return FALSE;

    if (!parseDecimalParameter("SetupStartLatency", id, rasP->setupStartLat))
        return FALSE;

    if (!parseDecimalParameter("TriangleInputLatency", id, rasP->trInputLat))
        return FALSE;

    if (!parseDecimalParameter("TriangleOutputLatency", id, rasP->trOutputLat))
        return FALSE;

    if (!parseBooleanParameter("TriangleSetupOnShader", id, rasP->shadedSetup))
        return FALSE;

    if (!parseDecimalParameter("TriangleShaderQueueSize", id, rasP->triangleShQSz))
        return FALSE;

    if (!parseDecimalParameter("StampsPerCycle", id, rasP->stampsCycle))
        return FALSE;

    if (!parseDecimalParameter("MSAASamplesCycle", id, rasP->samplesCycle))
        return FALSE;

    if (!parseDecimalParameter("OverScanWidth", id, rasP->overScanWidth))
        return FALSE;

    if (!parseDecimalParameter("OverScanHeight", id, rasP->overScanHeight))
        return FALSE;

    if (!parseDecimalParameter("ScanWidth", id, rasP->scanWidth))
        return FALSE;

    if (!parseDecimalParameter("ScanHeight", id, rasP->scanHeight))
        return FALSE;

    if (!parseDecimalParameter("GenWidth", id, rasP->genWidth))
        return FALSE;

    if (!parseDecimalParameter("GenHeight", id, rasP->genHeight))
        return FALSE;

    if (!parseDecimalParameter("RasterizationBatchSize", id, rasP->rastBatch))
        return FALSE;

    if (!parseDecimalParameter("BatchQueueSize", id, rasP->batchQueueSize))
        return FALSE;

    if (!parseBooleanParameter("RecursiveMode", id, rasP->recursive))
        return FALSE;

    if (!parseBooleanParameter("DisableHZ", id, rasP->disableHZ))
        return FALSE;

    if (!parseDecimalParameter("StampsPerHZBlock", id, rasP->stampsHZBlock))
        return FALSE;

    if (!parseDecimalParameter("HierarchicalZBufferSize", id, rasP->hzBufferSize))
        return FALSE;

    if (!parseDecimalParameter("HZCacheLines", id, rasP->hzCacheLines))
        return FALSE;

    if (!parseDecimalParameter("HZCacheLineSize", id, rasP->hzCacheLineSize))
        return FALSE;

    if (!parseDecimalParameter("EarlyZQueueSize", id, rasP->earlyZQueueSz))
        return FALSE;

    if (!parseDecimalParameter("HZAccessLatency", id, rasP->hzAccessLatency))
        return FALSE;

    if (!parseDecimalParameter("HZUpdateLatency", id, rasP->hzUpdateLatency))
        return FALSE;

    if (!parseDecimalParameter("HZBlocksClearedPerCycle", id, rasP->hzBlockClearCycle))
        return FALSE;

    if (!parseDecimalParameter("NumInterpolators", id, rasP->numInterpolators))
        return FALSE;

    if (!parseDecimalParameter("ShaderInputQueueSize", id, rasP->shInputQSize))
        return FALSE;

    if (!parseDecimalParameter("ShaderOutputQueueSize", id, rasP->shOutputQSize))
        return FALSE;

    if (!parseDecimalParameter("ShaderInputBatchSize", id, rasP->shInputBatchSize))
        return FALSE;

    if (!parseBooleanParameter("TiledShaderDistribution", id, rasP->tiledShDistro))
        return FALSE;

    if (!parseDecimalParameter("VertexInputQueueSize", id, rasP->vInputQSize))
        return FALSE;

    if (!parseDecimalParameter("ShadedVertexQueueSize", id, rasP->vShadedQSize))
        return FALSE;

    if (!parseDecimalParameter("TriangleInputQueueSize", id, rasP->trInputQSize))
        return FALSE;

    if (!parseDecimalParameter("TriangleOutputQueueSize", id, rasP->trOutputQSize))
        return FALSE;

    if (!parseDecimalParameter("GeneratedStampQueueSize", id, rasP->genStampQSize))
        return FALSE;

    if (!parseDecimalParameter("EarlyZTestedStampQueueSize", id, rasP->testStampQSize))
        return FALSE;

    if (!parseDecimalParameter("InterpolatedStampQueueSize", id, rasP->intStampQSize))
        return FALSE;

    if (!parseDecimalParameter("ShadedStampQueueSize", id, rasP->shadedStampQSize))
        return FALSE;

    if (!parseDecimalParameter("EmulatorStoredTriangles", id, rasP->emuStoredTriangles))
        return FALSE;

    if (!parseBooleanParameter("UseMicroPolygonRasterizer", id, rasP->useMicroPolRast))
        return FALSE;

    if (!parseBooleanParameter("PreBoundTriangles", id, rasP->preBoundTriangles))
        return FALSE;

    if (!parseBooleanParameter("CullMicroTriangles", id, rasP->cullMicroTriangles))
        return FALSE;

    if (!parseDecimalParameter("TriangleBoundOutputLatency", id, rasP->trBndOutLat))
        return FALSE;
 
    if (!parseDecimalParameter("TriangleBoundOpLatency", id, rasP->trBndOpLat))
        return FALSE;
 
    if (!parseDecimalParameter("LargeTriangleFIFOSize", id, rasP->trBndLargeTriFIFOSz))            
        return FALSE;
 
    if (!parseDecimalParameter("MicroTriangleFIFOSize", id, rasP->trBndMicroTriFIFOSz))
        return FALSE;

    if (!parseBooleanParameter("UseBoundingBoxOptimization", id, rasP->useBBOptimization))
        return FALSE;

    if (!parseDecimalParameter("SubPixelPrecisionBits", id, rasP->subPixelPrecision))
        return FALSE;
 
    if (!parseFloatParameter("LargeTriangleMinSize", id, rasP->largeTriMinSz))
        return FALSE;

    if (!parseBooleanParameter("ProcessMicroTrianglesAsFragments", id, rasP->microTrisAsFragments))
        return FALSE;
 
    if (!parseDecimalParameter("MicroTriangleSizeLimit", id, rasP->microTriSzLimit))          
        return FALSE;

    if (!parseStringParameter("MicroTriangleFlowPath", id, rasP->microTriFlowPath))
        return FALSE;

    if (!parseBooleanParameter("DumpTriangleBurstSizeHistogram", id, rasP->dumpBurstHist))
        return FALSE;
 
    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [RASTERIZER] is not supported";
        panic("ConfigLoader", "parseRasSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Fragment Shader parameters.  */
bool ConfigLoader::parseFShSectionParameter(FShParameters *fshP)
{
    char id[100];

    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("FRAGMENTSHADER");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("ExecutableThreads", id, fshP->numThreads))
        return FALSE;

    if (!parseDecimalParameter("InputBuffers", id, fshP->numInputBuffers))
        return FALSE;

    if (!parseDecimalParameter("ThreadResources", id, fshP->numResources))
        return FALSE;

    if (!parseDecimalParameter("ThreadRate", id, fshP->threadRate))
        return FALSE;

    if (!parseBooleanParameter("ScalarALU", id, fshP->scalarALU))
        return FALSE;

    if (!parseDecimalParameter("FetchRate", id, fshP->fetchRate))
        return FALSE;

    if (!parseDecimalParameter("ThreadGroup", id, fshP->threadGroup))
        return FALSE;

    if (!parseBooleanParameter("LockedExecutionMode", id, fshP->lockedMode))
        return FALSE;



    if (!parseBooleanParameter("VectorShader", id, fshP->useVectorShader))
        return FALSE;
        
    if (!parseDecimalParameter("VectorThreads", id, fshP->vectorThreads))
        return FALSE;

    if (!parseDecimalParameter("VectorResources", id, fshP->vectorResources))
        return FALSE;

    if (!parseDecimalParameter("VectorLength", id, fshP->vectorLength))
        return FALSE;

    if (!parseDecimalParameter("VectorALUWidth", id, fshP->vectorALUWidth))
        return FALSE;

    if (!parseStringParameter("VectorALUConfig", id, fshP->vectorALUConfig))
        return FALSE;
        
    if (!parseBooleanParameter("VectorWaitOnStall", id, fshP->vectorWaitOnStall))
        return FALSE;

    if (!parseBooleanParameter("VectorExplicitBlock", id, fshP->vectorExplicitBlock))
        return FALSE;


    if (!parseBooleanParameter("VertexAttributeLoadFromShader", id, fshP->vAttrLoadFromShader))
        return FALSE;
   
    if (!parseBooleanParameter("ThreadWindow", id, fshP->threadWindow))
        return FALSE;

    if (!parseDecimalParameter("FetchDelay", id, fshP->fetchDelay))
        return FALSE;

    if (!parseBooleanParameter("SwapOnBlock", id, fshP->swapOnBlock))
        return FALSE;

    if (!parseBooleanParameter("FixedLatencyALU", id, fshP->fixedLatencyALU))
        return FALSE;

    if (!parseDecimalParameter("InputsPerCycle", id, fshP->inputsCycle))
        return FALSE;

    if (!parseDecimalParameter("OutputsPerCycle", id, fshP->outputsCycle))
        return FALSE;

    if (!parseDecimalParameter("OutputLatency", id, fshP->outputLatency))
        return FALSE;

    if (!parseDecimalParameter("TextureUnits", id, fshP->textureUnits))
        return FALSE;

    if (!parseDecimalParameter("TextureRequestRate", id, fshP->textRequestRate))
        return FALSE;

    if (!parseDecimalParameter("TextureRequestGroup", id, fshP->textRequestGroup))
        return FALSE;


    if (!parseDecimalParameter("AnisotropyAlgorithm", id, fshP->anisoAlgo))
        return FALSE;

    if (!parseBooleanParameter("ForceMaxAnisotropy", id, fshP->forceMaxAniso))
        return FALSE;
        
    if (!parseDecimalParameter("MaxAnisotropy", id, fshP->maxAnisotropy))
        return FALSE;
    
    if (!parseDecimalParameter("TrilinearPrecision", id, fshP->triPrecision))
        return FALSE;
        
    if (!parseDecimalParameter("BrilinearThreshold", id, fshP->briThreshold))
        return FALSE;

    if (!parseDecimalParameter("AnisoRoundPrecision", id, fshP->anisoRoundPrec))
        return FALSE;
        
    if (!parseDecimalParameter("AnisoRoundThreshold", id, fshP->anisoRoundThres))
        return FALSE;
        
    if (!parseBooleanParameter("AnisoRatioMultOfTwo", id, fshP->anisoRatioMultOf2))
        return FALSE;
        
    if (!parseDecimalParameter("TextureBlockDimension", id, fshP->textBlockDim))
        return FALSE;

    if (!parseDecimalParameter("TextureSuperBlockDimension", id, fshP->textSBlockDim))
        return FALSE;

    if (!parseDecimalParameter("TextureRequestQueueSize", id, fshP->textReqQSize))
        return FALSE;

    if (!parseDecimalParameter("TextureAccessQueue", id, fshP->textAccessQSize))
        return FALSE;

    if (!parseDecimalParameter("TextureResultQueue", id, fshP->textResultQSize))
        return FALSE;

    if (!parseDecimalParameter("TextureWaitReadWindow", id, fshP->textWaitWSize))
        return FALSE;

    if (!parseBooleanParameter("TwoLevelTextureCache", id, fshP->twoLevelCache))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheLineSize", id, fshP->txCacheLineSz))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheWays", id, fshP->txCacheWays))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheLines", id, fshP->txCacheLines))
        return FALSE;

    if (!parseDecimalParameter("TextureCachePortWidth", id, fshP->txCachePortWidth))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheRequestQueueSize", id, fshP->txCacheReqQSz))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheInputQueue", id, fshP->txCacheInQSz))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheMissesPerCycle", id, fshP->txCacheMissCycle))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheDecompressLatency", id, fshP->txCacheDecomprLatency))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheLineSizeL1", id, fshP->txCacheLineSzL1))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheWaysL1", id, fshP->txCacheWaysL1))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheLinesL1", id, fshP->txCacheLinesL1))
        return FALSE;

    if (!parseDecimalParameter("TextureCacheInputQueueL1", id, fshP->txCacheInQSzL1))
        return FALSE;

    if (!parseDecimalParameter("AddressALULatency", id, fshP->addressALULat))
        return FALSE;

    if (!parseDecimalParameter("FilterALULatency", id, fshP->filterLat))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [FRAGMENTSHADER] is not supported";
        panic("ConfigLoader", "parseFShSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Z Stencil Test parameters.  */
bool ConfigLoader::parseZSTSectionParameter(ZSTParameters *zstP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("ZSTENCILTEST");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("StampsPerCycle", id, zstP->stampsCycle))
        return FALSE;

    if (!parseDecimalParameter("BytesPerPixel", id, zstP->bytesPixel))
        return FALSE;

    if (!parseBooleanParameter("DisableCompression", id, zstP->disableCompr))
        return FALSE;

    if (!parseDecimalParameter("ZCacheWays", id, zstP->zCacheWays))
        return FALSE;

    if (!parseDecimalParameter("ZCacheLines", id, zstP->zCacheLines))
        return FALSE;

    if (!parseDecimalParameter("ZCacheStampsPerLine", id, zstP->zCacheLineStamps))
        return FALSE;

    if (!parseDecimalParameter("ZCachePortWidth", id, zstP->zCachePortWidth))
        return FALSE;

    if (!parseBooleanParameter("ZCacheExtraReadPort", id, zstP->extraReadPort))
        return FALSE;

    if (!parseBooleanParameter("ZCacheExtraWritePort", id, zstP->extraWritePort))
        return FALSE;

    if (!parseDecimalParameter("ZCacheRequestQueueSize", id, zstP->zCacheReqQSz))
        return FALSE;

    if (!parseDecimalParameter("ZCacheInputQueueSize", id, zstP->zCacheInQSz))
        return FALSE;

    if (!parseDecimalParameter("ZCacheOutputQueueSize", id, zstP->zCacheOutQSz))
        return FALSE;

    if (!parseDecimalParameter("BlockStateMemorySize", id, zstP->blockStateMemSz))
        return FALSE;

    if (!parseDecimalParameter("BlocksClearedPerCycle", id, zstP->blockClearCycle))
        return FALSE;

    if (!parseDecimalParameter("CompressionUnitLatency", id, zstP->comprLatency))
        return FALSE;

    if (!parseDecimalParameter("DecompressionUnitLatency", id, zstP->decomprLatency))
        return FALSE;

    if (!parseDecimalParameter("CompressionAlgorithm", id, zstP->comprAlgo))
        return FALSE;
    
    //if (!parseDecimalParameter("ZQueueSize", id, zstP->zQueueSz))
    //    return FALSE;

    if (!parseDecimalParameter("InputQueueSize", id, zstP->inputQueueSize))
        return FALSE;

    if (!parseDecimalParameter("FetchQueueSize", id, zstP->fetchQueueSize))
        return FALSE;

    if (!parseDecimalParameter("ReadQueueSize", id, zstP->readQueueSize))
        return FALSE;

    if (!parseDecimalParameter("OpQueueSize", id, zstP->opQueueSize))
        return FALSE;

    if (!parseDecimalParameter("WriteQueueSize", id, zstP->writeQueueSize))
        return FALSE;

    if (!parseDecimalParameter("ZALUTestRate", id, zstP->zTestRate))
        return FALSE;

    if (!parseDecimalParameter("ZALULatency", id, zstP->zOpLatency))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [ZSTENCILTEST] is not supported";
        panic("ConfigLoader", "parseZSTSectionParameter", ss.str().c_str());
    }


    return TRUE;
}



/*  Parse Color Write parameters.  */
bool ConfigLoader::parseCWRSectionParameter(CWRParameters *cwrP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("COLORWRITE");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("StampsPerCycle", id, cwrP->stampsCycle))
        return FALSE;

    if (!parseDecimalParameter("BytesPerPixel", id, cwrP->bytesPixel))
        return FALSE;

    if (!parseBooleanParameter("DisableCompression", id, cwrP->disableCompr))
        return FALSE;

    if (!parseDecimalParameter("ColorCacheWays", id, cwrP->cCacheWays))
        return FALSE;

    if (!parseDecimalParameter("ColorCacheLines", id, cwrP->cCacheLines))
        return FALSE;

    if (!parseDecimalParameter("ColorCacheStampsPerLine", id, cwrP->cCacheLineStamps))
        return FALSE;

    if (!parseDecimalParameter("ColorCachePortWidth", id, cwrP->cCachePortWidth))
        return FALSE;

    if (!parseBooleanParameter("ColorCacheExtraReadPort", id, cwrP->extraReadPort))
        return FALSE;

    if (!parseBooleanParameter("ColorCacheExtraWritePort", id, cwrP->extraWritePort))
        return FALSE;

    if (!parseDecimalParameter("ColorCacheRequestQueueSize", id, cwrP->cCacheReqQSz))
        return FALSE;

    if (!parseDecimalParameter("ColorCacheInputQueueSize", id, cwrP->cCacheInQSz))
        return FALSE;

    if (!parseDecimalParameter("ColorCacheOutputQueueSize", id, cwrP->cCacheOutQSz))
        return FALSE;

    if (!parseDecimalParameter("BlockStateMemorySize", id, cwrP->blockStateMemSz))
        return FALSE;

    if (!parseDecimalParameter("BlocksClearedPerCycle", id, cwrP->blockClearCycle))
        return FALSE;

    if (!parseDecimalParameter("CompressionUnitLatency", id, cwrP->comprLatency))
        return FALSE;

    if (!parseDecimalParameter("DecompressionUnitLatency", id, cwrP->decomprLatency))
        return FALSE;

    if (!parseDecimalParameter("CompressionAlgorithm", id, cwrP->comprAlgo))
        return FALSE;
    
    //if (!parseDecimalParameter("ColorQueueSize", id, cwrP->colorQueueSz))
    //    return FALSE;

    if (!parseDecimalParameter("InputQueueSize", id, cwrP->inputQueueSize))
        return FALSE;

    if (!parseDecimalParameter("FetchQueueSize", id, cwrP->fetchQueueSize))
        return FALSE;

    if (!parseDecimalParameter("ReadQueueSize", id, cwrP->readQueueSize))
        return FALSE;

    if (!parseDecimalParameter("OpQueueSize", id, cwrP->opQueueSize))
        return FALSE;

    if (!parseDecimalParameter("WriteQueueSize", id, cwrP->writeQueueSize))
        return FALSE;

    if (!parseDecimalParameter("BlendALURate", id, cwrP->blendRate))
        return FALSE;

    if (!parseDecimalParameter("BlendALULatency", id, cwrP->blendOpLatency))
        return FALSE;

    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [COLORWRITE] is not supported";
        panic("ConfigLoader", "parseCWRSectionParameter", ss.str().c_str());
    }

    return TRUE;
}


/*  Parse DAC parameters.  */
bool ConfigLoader::parseDACSectionParameter(DACParameters *dacP)
{
    char id[100];

    /*  Try to get a parameter name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    paramsTracker.startParamSectionDefinition("DAC");

    /*  Parse parameters.  */

    if (!parseDecimalParameter("BytesPerPixel", id, dacP->bytesPixel))
        return FALSE;

    if (!parseDecimalParameter("BlockSize", id, dacP->blockSize))
        return FALSE;

    if (!parseDecimalParameter("BlockUpdateLatency", id, dacP->blockUpdateLat))
        return FALSE;

    if (!parseDecimalParameter("BlocksUpdatedPerCycle", id, dacP->blocksCycle))
        return FALSE;

    if (!parseDecimalParameter("BlockRequestQueueSize", id, dacP->blockReqQSz))
        return FALSE;

    if (!parseDecimalParameter("DecompressionUnitLatency", id, dacP->decomprLatency))
        return FALSE;

    if (!parseDecimalParameter("RefreshRate", id, dacP->refreshRate))
        return FALSE;

    if (!parseBooleanParameter("SynchedRefresh", id, dacP->synchedRefresh))
        return FALSE;

    if (!parseBooleanParameter("RefreshFrame", id, dacP->refreshFrame))
        return FALSE;

    if (!parseBooleanParameter("SaveBlitSourceData", id, dacP->saveBlitSource))
        return FALSE;
        
    if ( !paramsTracker.wasAnyParamSectionDefined() ) {
        stringstream ss;
        ss << "Parameter '" << id << "' in section [DAC] is not supported";
        panic("ConfigLoader", "parseDACSectionParameter", ss.str().c_str());
    }

    return TRUE;
}

/*  Parse Signal parameters.  */
bool ConfigLoader::parseSigSectionParameter(SigParameters *sigP)
{
    char id[100];
    s32bit bandwidth;
    s32bit latency;

    /*  Try to get a signal name.  */
    if (!parseId(id))
        return FALSE;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Check '='.  */
    if (str[pos] != '=')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    skipSpaces();

    checkEndOfString

    /*  Parse a decimal.  */
    if (!parseDecimal(bandwidth))
        return FALSE;

    checkEndOfString

    /*  Check bandwidth/latency separator.  */
    if (str[pos] != ':')
        return FALSE;

    /*  Update position.  */
    pos++;

    checkEndOfString

    /*  Parse a decimal.  */
    if (!parseDecimal(latency))
        return FALSE;

    /*  Set signal parameters.  */

    sigP->name = new char[strlen(id) + 1];
    memcpy(sigP->name, id, strlen(id) + 1);

    sigP->bandwidth = bandwidth;
    sigP->latency = latency;

    return TRUE;
}


/*  Parses a decimal parameter (32 bit version)  */
bool ConfigLoader::parseDecimalParameter(char *paramName, char *id, u32bit &val)
{
    s32bit aux;

    paramsTracker.registerParam(paramName);
    //registerParam(_sectionStr, paramName);

    /*  Search section parameter name.  */
    if (!strcmp(paramName, id))
    {
        /*  Try to parse a number.  */
        if (!parseDecimal(aux))
            return FALSE;

        /*  Set decimal parameter.  */
        val = aux;
        paramsTracker.setParamDefined(id);
    }

    return TRUE;
}

/*  Parses a decimal parameter (64 bit version).  */
bool ConfigLoader::parseDecimalParameter(char *paramName, char *id, u64bit &val)
{
    s64bit aux;

    paramsTracker.registerParam(paramName);

    /*  Search section parameter name.  */
    if (!strcmp(paramName, id))
    {
        /*  Try to parse a number.  */
        if (!parseDecimal(aux))
            return FALSE;

        /*  Set decimal parameter.  */
        val = aux;
        paramsTracker.setParamDefined(id);
    }

    return TRUE;
}

/*  Parses a float parameter (32 bit).  */
bool ConfigLoader::parseFloatParameter(char *paramName, char *id, f32bit &val)
{
    f32bit aux;

    paramsTracker.registerParam(paramName);

    /*  Search section parameter name.  */
    if (!strcmp(paramName, id))
    {
        /*  Try to parse a number.  */
        if (!parseFP(aux))
            return FALSE;

        /*  Set float parameter.  */
        val = aux;
        paramsTracker.setParamDefined(id);
    }

    return TRUE;
}

/*  Parse a string parameter.  */
bool ConfigLoader::parseStringParameter(char *paramName, char *id,
    char *&string)
{
    char *auxStr;

    paramsTracker.registerParam(paramName);

    if (!strcmp(paramName, id))
    {
        /*  Try to parse a string.  */
        if (!parseString(auxStr))
            return FALSE;

        /*  Return string parameter.  */
        string = auxStr;
        paramsTracker.setParamDefined(id);
    }

    return TRUE;
}

/*  Parse a boolean parameter.  */
bool ConfigLoader::parseBooleanParameter(char *paramName, char *id, bool &val)
{
    bool aux;

    paramsTracker.registerParam(paramName);

    if (!strcmp(paramName, id))
    {
        /*  Try to parse a string.  */
        if (!parseBoolean(aux))
            return FALSE;

        /*  Return boolean parameter.  */
        val = aux;
        paramsTracker.setParamDefined(id);
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
//                              ParamsTracker methods                               //
//////////////////////////////////////////////////////////////////////////////////////

void ConfigLoader::ParamsTracker::startParamSectionDefinition(const string& sec)
{
    paramSectionDefinedFlag = false; 
    secStr = sec;
}

bool ConfigLoader::ParamsTracker::wasAnyParamSectionDefined() const
{
    return paramSectionDefinedFlag;
}

void ConfigLoader::ParamsTracker::setParamDefined(const std::string& paramName)
{
    map<string,bool>::iterator it = paramsSeen.find(secStr + "=" + paramName);
    if ( it == paramsSeen.end() ) {
        stringstream ss;
        ss << "Param [" << secStr << "] -> " << paramName << " was not previously registered before define it";
        panic("ConfigLoader", "ParamsTracker::setParamDefined", ss.str().c_str());
    }
    it->second =  true;
    paramSectionDefinedFlag = true;
}

void ConfigLoader::ParamsTracker::registerParam(const std::string& paramName)
{
    map<string,bool>::iterator it = paramsSeen.find(secStr + "=" + paramName);
    if ( it == paramsSeen.end() )
        paramsSeen.insert(make_pair(secStr + "=" + paramName, false)); // Add this param to the map
}

/*
bool ConfigLoader::ParamsTracker::isParamDefined(const string& sec, const string& paramName) const
{
    map<string,bool>::const_iterator it = paramsSeen.find(sec + "=" + paramName);
    return (it != paramsSeen.end() && it->second);
}
*/

void ConfigLoader::ParamsTracker::checkParamsIntegrity(const SimParameters& simP)
{   
    // Traverse params seen, all params should be true
    map<string,bool>::const_iterator it = paramsSeen.begin();
    for ( ; it != paramsSeen.end(); ++it ) {
        if ( !it->second ) {
            stringstream ss;
            size_t pos = it->first.find_first_of("=");
            string secName = it->first.substr(0,pos);
            string pName   = it->first.substr(pos+1);
            // ss << "Param '" << pName << "' from section '" << secName << "' expected and wasn't found";
            ss << "Param [" << secName << "] : '" << pName << "' expected and wasn't found";
            panic("ConfigLoader", "ParamsTracker::checkParamsIntegrity", ss.str().c_str());
        }
    }
}

