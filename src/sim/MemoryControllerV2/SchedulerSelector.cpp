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
 */

#include "SchedulerSelector.h"
#include "FifoScheduler.h"
#include "RWFifoScheduler.h"
#include "BankQueueScheduler.h"
#include "GPUMemorySpecs.h"

#include <sstream>
#include <iostream>

using std::cout;
using std::stringstream;

using namespace gpu3d;
using namespace gpu3d::memorycontroller;

typedef MemoryControllerParameters MCParams;

ChannelScheduler* gpu3d::memorycontroller::createChannelScheduler(const char* name, 
                                         const char* prefix, 
                                         const MemoryControllerParameters& params,
                                         Box* parent)
{

    ChannelScheduler::MemoryClientsInfo mci;

    mci.numStampUnits = params.numStampUnits;
    mci.numTextureUnits = params.numTexUnits;
    mci.streamerLoaderUnits = params.streamerLoaderUnits;

    // Hardcoded param for now
    bool enableProtocolMonitoring = true;

    ChannelScheduler* sched = 0;

    // Select page policy
    FifoSchedulerBase::PagePolicy policy;
    if ( params.schedulerPagePolicy == MCParams::ClosePage )
        policy = FifoSchedulerBase::ClosePage;
    else if ( params.schedulerPagePolicy == MCParams::OpenPage )
        policy = FifoSchedulerBase::OpenPage;
    else
    {
        stringstream ss;
        ss << "Page policy " << (u32bit)params.schedulerPagePolicy << " unknown";
        panic("SchedulerSelector", "createChannelScheduler", ss.str().c_str());
    }


    if ( params.memSpecs == 0 ) {
        panic("SchedulerSelector", "createChannelScheduler", "GPU memory specs is NULL!");
    }

    ChannelScheduler::CommonConfig commonConfig(
        mci,
        params.banksPerMemoryChannel,
        params.burstLength,
        params.burstBytesPerCycle,
        enableProtocolMonitoring,
        *params.memSpecs,
        static_cast<ChannelScheduler::SwitchModePolicy>(params.switchModePolicy),
        params.rwfifo_maxConsecutiveReads,
        params.rwfifo_maxConsecutiveWrites,
        policy,
        params.maxChannelTransactions, // MAX on-flight transactions per channel 
        params.dedicatedChannelReadTransactions, // Dedicated reads (0 means that the number of reads an writes are maxTrans/2)
        *params.debugString,
        params.useClassicSchedulerStates,
        params.bankfifo_activeManagerMode,
        params.bankfifo_prechargeManagerMode,
        params.bankfifo_disableActiveManager,
        params.bankfifo_disablePrechargeManager,
        params.bankfifo_managerSelectionAlgorithm,
        0, // priorize hits over BankSelectionPolicy
        *params.bankfifo_bankSelectionPolicy,
        true);


    if ( params.schedulerType == MCParams::Fifo )
    {
        cout << "Creating FifoScheduler: " << prefix << "::" << name << "\n";
        sched = new FifoScheduler( name, prefix, parent, commonConfig ); //, params.fifo_maxChannelTransactions);
    }
    else if ( params.schedulerType == MCParams::RWFifo )
    {
        cout << "Creating RWFifoScheduler: " << prefix << "::" << name << "\n";
        sched = new RWFifoScheduler( name, prefix, parent, commonConfig );
                                     // params.rwfifo_maxReadChannelTransactions, params.rwfifo_maxWriteChannelTransactions, 
                                     // params.rwfifo_maxConsecutiveReads, params.rwfifo_maxConsecutiveWrites
    }
    else if ( params.schedulerType == MCParams::BankQueueFifo )
    {
        cout << "Creating BankQueueScheduler: " << prefix << "::" << name << "\n";
        sched = new BankQueueScheduler( name, prefix, parent, commonConfig );
                                        // params.fifo_maxChannelTransactions, 
                                        // params.rwfifo_maxConsecutiveReads, params.rwfifo_maxConsecutiveWrites );
    }
    else
        panic("SchedulerSelector", "createChannelScheduler", "Unknown scheduler type");

    return sched;
}

