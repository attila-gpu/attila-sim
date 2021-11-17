
GLInterceptor
--------------

1. Modify the GLIconfig.ini file with the name you want to use for the
   captured trace files

| outputFile=[tracefile.txt] | Sets the name for the OpenGL API call] trace file. GLInterceptor may generate two additional files with fixed file names if the logBuffers option is enabled: MemoryRegions.dat and BufferDescriptors.dat |
| -------------------------- | ------------------------------------------------------------ |

2. Set the capturing parameters

| Option                          | Details                                                      |
| ------------------------------- | ------------------------------------------------------------ |
| format=[text\|hex]              | Sets how float point numbers are dumped in the trace file:<br />hex => as 32-bit or 64-bit hexadecimal values<br />text => decimal format (may produce precision problems). |
| logBuffers=[0\|1]               | Enables (1) or disables (0) dump buffers and textures provided to the OpenGL library (required if you want to play or simulate the file). |
| dumpMode=[VERTICAL\|HORIZONTAL] | Sets how statistics are generated (to avoid the Office limitation of 255 columns ...) |
| frameStats=[filename]           | Per frame statistics file name.  Set to 'null' to disable statistics generation |
| batchStats=[filename]           | Per batch statistics file name.  Set to 'null' to disable statistics generation |
| traceReturn=[0\|1]              | Sets if the return values from OpenGL API calls are dumped in the trace (simulator or player don't require the return values) |
| firstFrame=[frame]              | Sets the start frame for dumping the OpenGL API call trace.  If set different from 1 the trace shouldn't be reproducible with GLPlayer or simulable but you can always try ... |
| lastFrame=[frame]               | Sets the last frame for which the OpenGL API trace is generated. Used to limit the of captured frames and output files sizes |

3. Place the GLIConfig.ini and opengl32.dll in the same directory that the
   main binary of the game or application for which you want to capture
   the OpenGL API call traces (may be different than the directory where
   the game/application executable is found, for example in UT2004 it must
   be placed in the /system directory).  Obviously it doesnt' work with
   Direct3D based games or games configured to use the Direct3D rendering
   path.
   The drive must have enough space for the trace (in the order of 100's of MBs
   per a hundred of frames for the tested OpenGL games) or the capturing process
   may experience 'problems'.  The capturing process may consume a significant
   amount of system memory so try to avoid using it on systems with less than
   512 MBs.
4. Start the game or application for which to capture the OpenGL API call trace.
   If you have plenty of luck the game or application won't crash, the trace
   will be correctly captured and will be reproducible with GLPlayer.  Even then
   it's very likely that the simulator won't be able to use work with the trace.
   Keep in mind that only a minimum set of OpenGL API has been implemented (and most
   of that minimum set has not been fully tested).  Keep in mind that capturing
   the same game/application and timedemo/map/scenary in different GPUs (NVidia,
   ATI, ...) may result in different API call traces and some of all of those
   captured traces may not work because of unimplemented specific OpenGL extensions
   or features
5. We can capture, reproduce and simulate traces for the following games:
   UT2004, Doom3, Quake4, Chronicles of the Riddick, Prey and
   Enemy Territories: Quake Wars.

  1.       UT2004
               => you must set the OpenGL renderer (can't be done through the game
                  menu), set the rendered to use vertex buffer objects and disable
                  vertex shaders (as the ATILA rei OpenGL library only supports
                  either fixed function T&L or ARB vertex programs).  We have tested
                  UT2004 traces captured with NVidia GPUs so traces captured with ATI
                  GPUs may not work or show render errors.
         
           DOOM3/QUAKE4/PREY/ETQW
               => same engine so same configuration should work.  Disable
                  two sided stencil (not supported in the simulator), set
                  the arb2 render path (or whatever other path that only
                   uses ARB vertex and fragment programs).  Enable vertex
                   buffers, disable index buffers.
         
           Chronicles of the Riddick
               => use arb programs, use vertex objects

------------------------------------------------------------------------------------------------

GLPlayer
---------

    1. Place GLPlayer.exe and its configuration file GLPconfig.ini wherever you want
       on your drive
    
    2. Modify the GLPconfig.ini file to point to the trace files captured with GLInterceptor
       that you want to reproduce
    
        inputFile=tracefile.txt             OpenGL API call trace file name
        bufferFile=BufferDescriptors.dat    OpenGL API call buffer descriptors file name
        memFile=MemoryRegions.dat           OpenGL API call memory regions file name
    
    2. Set the player parameters
    
        freezeFrame=0                       Sets if the player reproduces the trace in sequence
                                            (0) or stops after each frame (0).  When freezeMode
                                            is enabled use the 'f' key to move to the next frame
        fps=100                             Maximum framerate for the reproduction (freezeMode
                                            disabled
        resTrace=1                          Use (1) the resolution specified in the trace as the
                                            reproduction resolution/window size
        resViewport=1                       Use (1) the resolution specified by the first
                                            glViewport() call
        defaultRes=800x600                  Sets the default resolution if the other two methods
                                            are disabled or fail to obtain a resolution
        startFrame=0                        Sets the reproduction start frame.  If set different
                                            from 0 GLPlayer will skip and avoid rendering the
                                            first n frames of the trace
        autoCapture=0                       If set to 1 captures and dumps as bmps all the
                                            rendered frames.  Manual capturing is performed using
                                            the 'C' key.
        bufferCache=100                     (ask Carlos)
        allowUndefinedBuffers=0             (ask Carlos)
    
    3. Execute GLPlayer
    
       While reproducing the trace this keys can be used :
    
        ESC     End the reproduction
        C       Capture the current frame, dumps a PNG file (only works in freeze mode)
        D       Dump the color, depth and stencil buffers (only works in freeze mode)
        F       Render the next frame (only works in freeze mode)
        B       Render the next draw call (only work in freeze mode)
        R       Benchmark the current frame (experimental, only works win freeze mode)


------------------------------------------------------------------------------------------------

PIX Traces
-----------

Use the PIX tool in the DirectX SDK to capture single frame or whole capture PIXRun traces from
a game or an application.

Microsoft provides an API to game developers to disable PIX trace capturing capabilities so you may
not be able to capture traces from some/many games without a bit of hacking.  We have used our
independently developed DXInterceptor tools to capture D3D9 game traces and then capture them with
PIX.


PIX Player
-----------

D3DPlayer4Windows

    1.  Place D3DPlayer4Windows.exe, d3d9pixrunplayer.ini and PIXLog.cfg in a directory
    
    2.  Set the PIX player parameters defined in "d3d9pixrunplayer.ini" and "PIXLog.cfg" (for logging)
    
    3.  From a command line window:
        > D3DPlayer4Windows filename
    
        Where filename is the path to a PIXRun trace file.  The player supports PIXRun traces compiled
        with gzip (renamed to PIXRunz).
    
        While reproducing the trace this keys can be used :
    
            ESC     End the reproduction
            ENTER   Start/Stop trace reproduction
            B       Render the next draw call
            F       Render the next frame
            A       Enable/Disable frame autocapture (PNG files per draw call or frame)
            C       Capture frame (PNG file)
            D       Dump color, depth and stencil buffers (may not work for depth and stencil)


------------------------------------------------------------------------------------------------


How to run the ATTILA simulator and emulator
---------------------------------------------


    1. Copy the simulator binary (bGPU, bGPU-Uni, bGPU-emu), a configuration file (bGPU.ini) and the
       trace files into the same directory (the configuration file will be described in future documentation)
    
    2. Execute the simulator
    
        > [bGPU-Uni|bGPU|bGPU-emu]
    
        or
    
        > [bGPU-Uni|bGPU|bGPU-emu] filename
    
        or
    
        > [bGPU-Uni|bGPU|bGPU-emu] filename n
    
        or
    
        > [bGPU-Uni|bGPU|bGPU-emu] filename n m


        When no parameters are specified the trace filename and all other parameters are obtained from the
        configuration file (which must be named 'bGPU.ini').
    
        When parameters are used:
    
            filename        OpenGL API call trace file (usually tracefile.txt or tracefile.txt.gz) or PIX trace file (PIXRun or PIXRunz)
            n               Number of frames to render (if n < 10K)
                            or
                            Cycles to simulate (if n > 10K)
            m               Start simulation frame (when m > 0 the simulator will skip
                            m frames from the trace before starting the simulation)
    
    4. Get the simulation output
    
       The simulation has four outputs:
    
        a) standard output
    
            Example:
    
                > ~/gpu3d/bin/bGPU-Uni texComb256.txt 1
                Simulator Parameters.
                --------------------
    
                Input File = texComb256.txt
                Signal Trace File = signaltrace.txt
                Statistics File = stats.csv
                Statistics (Per Frame) File = stats.frame.csv
                Statistics (Per Batch) File = stats.batch.csv
                Simulation Cycles = 0
                Simulation Frames = 1
                Simulation Start Frame = 0
                Signal Trace Dump = disabled
                Signal Trace Start Cycle = 0
                Signal Trace Dump Cycles = 10000
                Statistics Generation = enabled
                Statistics (Per Frame) Generation = enabled
                Statistics (Per Batch) Generation = enabled
                Statistics Rate = 10000
                OptimizedDynamicMemory => FAST_NEW_DELETE enabled.  Ignoring third bucket!
                Using OpenGL Trace File as simulation input.
                Simulating 1 frames (1 dot : 10K cycles).


                Warning: clearZ implemented as clear z and stencil (optimization)
                ............B.Dumping frame 0
                GPUDriver => Memory usage : GPU 263 blocks | System 0 blocks
                ColorWrite => End of swap.  Cycle 132570
                ColorWrite => End of swap.  Cycle 132570
                ColorWrite => End of swap.  Cycle 132570
                ColorWrite => End of swap.  Cycle 132570
                .DAC => Cycle 141952 Color Buffer Dumped.
    
                END Cycle 141954 ----------------------------
                Bucket 0: Size 65536 Last 731 Max 0 | Bucket 1: Size 16384 Last 0 Max 0 | Bucket 2: Size 65536 Last 0 Max 0


                End of simulation
    
            Standard output information:
    
                I)   Some of the simulation parameters obtained from the configuration file
                     and the command line
                II)  Miscelaneous messages (may be described in future documentation)
                III) Simulator state:
    
                        A '.' indicates that a number of cycles (10K by default) has passed
    
                        A 'B' indicates that a OpenGL draw call (or batch) has been fully
                        processed.
    
                        Memory usage messages from the driver after each frame finalizes.
    
                        Messages from the ColorWrite pipelines with the simulated cycle at
                        which the the last fragment in a frame was processed.  Used to
                        measure the per frame simulation time
    
                        Messages from the DAC unit with the cycle when the frame is dumped
                        to a file.  Used to measure the per frame simulation time
    
                        Miscelaenous messages and warnings from the simulator and OpenGL
                        library
    
                        Miscelaneous panic and crash messages from the simulator and OpenGL
                        library
    
                    If after many cycles (millions) no 'B' is ever printed it is very likely
                    that the simulator is in a deadlock (it uses to happen a lot when using
                    buggus configurations).
    
                IV)  End of simulation message with the last simulated cycle and dynamic
                     memory usage statistics
    
        b) the rendered frames
    
            The rendered frames as PPM files
    
        c) 'latency' or per quad map frames
    
            Per pixel quad (2x2) map with some property for example overdraw or
            per pixel latency (will be described in future documentation)
    
        d) statistics files
    
            Simulator statistics file (will be described in future documentation)
    
        e) signal trace dump file
    
            Used for debugging purposes (will be described in future documentation)

How to run the ATTILA simulator or emulator with AGP traces (experimental)
---------------------------------------------------------------------------

    1. Copy the simulator (bGPU or bGPU-Uni), the OpenGL to 'AGP' trace translator tool (gl2attila)
       a configuration file (bGPU.ini) and the OpenGL trace files into the same directory
       (the configuration file will be described in future documentation)
    
    2. Execute the translator tool to generate the AGP trace file for the input OpenGL tracefile:
    
        > gl2attila
    
        or
    
        > gl2attila filename
    
        or
    
        > gl2attila filename n
    
        or
    
        > gl2attila filename n m


        When no parameters are specified the trace filename (only for the OpenGL API call
        trace file, the memory regions and buffer descriptors files have fixed file names)
        and all other parameters are obtained from the configuration file (which must
        be named 'bGPU.ini')
    
        When parameters are used:
    
            filename        Name of the OpenGL API call trace file
            n               Number of frames to translate
            m               First frame to translate (when m > 0 the translator tool will skip
                            m frames from the trace before starting with the translation process)
    
       The output of the translator tool is a file named 'attila.tracefile.gz' which stores all the
       AGP Transaction objects (the commands that control the rendering process in the ATTILA GPU)
       that would be issued to the ATTILA GPU to render the selected frames of the input OpenGL trace file.
    
       The first two steps can be skipped if the resulting AGP trace file is stored for later use.  The AGP
       trace file could be generated for all the frames in the input OpenGL trace and then reused for a
       faster simulation.  The name of the output AGP tracefile can changed as the simulator (bGPU-Uni or bGPU)
       accepts the trace file name as a parameter.  The only limitation is that some of the parameters in the
       configuration file (bGPU.ini) used to generate the translated AGP trace file should be the same in the
       configuration file (bGPU.ini) used by the simulator (memory sizes, texture block dimensions, rasterization
       tile dimensions, bytes per pixel, double buffering and fetch rate).
    
    3. Execute the simulator
    
        > [bGPU-Uni|bGPU]
    
        or
    
        > [bGPU-Uni|bGPU] filename
    
        or
    
        > [bGPU-Uni|bGPU] filename n
    
        or
    
        > [bGPU-Uni|bGPU] filename n m


        When no parameters are specified the trace filename (AGP trace file) and all other parameters
        are obtained from the configuration file (which must be named 'bGPU.ini')
    
        When parameters are used:
    
            filename        Name of the AGP trace file
            n               Number of frames to render (if n < 10K)
                            or
                            Cycles to simulate (if n > 10K)
            m               Start simulation frame (when m > 0 the simulator will skip
                            m frames from the trace before starting the simulation)
    
    4. Get the simulation output
    
       The simulation has four outputs:
    
        a) standard output
    
            Example:
    
                > ~/gpu3d/bin/bGPU-Uni texComb256.txt 1
                Simulator Parameters.
                --------------------
    
                Input File = texComb256.txt
                Signal Trace File = signaltrace.txt
                Statistics File = stats.csv
                Statistics (Per Frame) File = stats.frame.csv
                Statistics (Per Batch) File = stats.batch.csv
                Simulation Cycles = 0
                Simulation Frames = 1
                Simulation Start Frame = 0
                Signal Trace Dump = disabled
                Signal Trace Start Cycle = 0
                Signal Trace Dump Cycles = 10000
                Statistics Generation = enabled
                Statistics (Per Frame) Generation = enabled
                Statistics (Per Batch) Generation = enabled
                Statistics Rate = 10000
                OptimizedDynamicMemory => FAST_NEW_DELETE enabled.  Ignoring third bucket!
                Using AGP Trace File as simulation input.
                Simulating 1 frames (1 dot : 10K cycles).


                Warning: clearZ implemented as clear z and stencil (optimization)
                ............B.Dumping frame 0
                GPUDriver => Memory usage : GPU 263 blocks | System 0 blocks
                ColorWrite => End of swap.  Cycle 132570
                ColorWrite => End of swap.  Cycle 132570
                ColorWrite => End of swap.  Cycle 132570
                ColorWrite => End of swap.  Cycle 132570
                .DAC => Cycle 141952 Color Buffer Dumped.
    
                END Cycle 141954 ----------------------------
                Bucket 0: Size 65536 Last 731 Max 0 | Bucket 1: Size 16384 Last 0 Max 0 | Bucket 2: Size 65536 Last 0 Max 0


                End of simulation
    
            Standard output information:
    
                I)   Some of the simulation parameters obtained from the configuration file
                     and the command line
                II)  Miscelaneous messages (may be described in future documentation)
                III) Simulator state:
    
                        A '.' indicates that a number of cycles (10K by default) has passed
    
                        A 'B' indicates that a OpenGL draw call (or batch) has been fully
                        processed.
    
                        Memory usage messages from the driver after each frame finalizes.
    
                        Messages from the ColorWrite pipelines with the simulated cycle at
                        which the the last fragment in a frame was processed.  Used to
                        measure the per frame simulation time
    
                        Messages from the DAC unit with the cycle when the frame is dumped
                        to a file.  Used to measure the per frame simulation time
    
                        Miscelaenous messages and warnings from the simulator and OpenGL
                        library
    
                        Miscelaneous panic and crash messages from the simulator and OpenGL
                        library
    
                    If after many cycles (millions) no 'B' is ever printed it is very likely
                    that the simulator is in a deadlock (it uses to happen a lot when using
                    buggus configurations).
    
                IV)  End of simulation message with the last simulated cycle and dynamic
                     memory usage statistics
    
        b) the rendered frames
    
            The rendered frames as PPM files
    
        c) 'latency' or per quad map frames
    
            Per pixel quad (2x2) map with some property for example overdraw or
            per pixel latency (will be described in future documentation)
    
        d) statistics files
    
            Simulator statistics file (will be described in future documentation)
    
        e) signal trace dump file
    
            Used for debugging purposes (will be described in future documentation)


The Noisy Writer

    Victor Moya (vmoya@ac.upc.edu)

The Other Authors

    Carlos Gonzalez (cgonzale@ac.upc.edu)
    Jordi Roca (jroca@ac.upc.edu)
    Vicente Escandell (vicente@ac.upc.edu)
    Albert Murciego
    Chema Solis
    David Abella
    Christian Perez

The Managers

    Roger Espasa (roger.espasa@intel.com)
    Agustin Fernandez (agustin@ac.upc.edu)

