ATTILA GPU Simulator
===============

[![Compile](https://travis-ci.org/attila-gpu/attila-sim.svg)](https://travis-ci.org/attila-gpu/attila-sim)

Introduction
-------------

ATTILA is GPU simulator with OpenGL and D3D9 API implementations that can be used
to simulate PC Windows game traces.

In the ATTILA source code included in this package you will find the classes used
to build the simulator, the API implementations to run OpenGL and D3D9 game traces
on this simulator, the API trace tools and a number of tools that had been used on
the development of this project.

Some of the techniques and algorithms implemented in the simulator may be covered
by patents (for example DXTC/S3TC compression or the Z compression algorithm).

No documentation about the source code, the simulator or the library is provided within
this package.  Check for documentation available documentation in the ATTILA wiki:

        https://attila.ac.upc.edu


How to compile on GNU/Linux
----------------------------

    Usage: make [clean | simclean]
           make [what options]

    clean      - Delete all OBJs and binary files.
    simclean   - Delete only simulator related OBJs, and binary files.

    what:
           all                  - Build ATTILA simulator and tools
           bGPU                 - Build ATTILA simulator and emulator
           gl2atila             - Build gl2atila
           extractTraceRegion   - Build extractTraceRegion

    options:

            CONFIG={ debug  | profiling | optimized | verbose }
                debug     - Compile with debug information
                profiling - Compile with profiling information
                optimized - Maximum optimization (default)

            CPU=<cpu>
                <cpu>   - Target cpu for compilation. This lets apply some optimizations.
                          Possible values: x86, x86_64, pentium4, athlon, core2

            VERBOSE={ yes | no }
                yes     - Activate debug messages
                no      - Deactive debug messages (default)

            PLATFORM=<platf>
                <platf> - Target platform for compilation.
                          Possible values: linux (default), cygwin


Contents of the package
------------------------

	 LICENCE                         MIT/FreeBSD License
	 Makefile                        Global makefile
	 Makefile.common                 Global makefile definitions
	 Makefile.defs                   Global makefile definitions
	 README.txt                      This file.
	 USAGE.txt                       Basic instructions to capture and simulate a trace.
	 bin/                            Directory where the Linux/GCC compiled binaries are placed.
	 lib/                            Directory where the Linux/GCC compiled libraries are placed.
	 src/                            Source code for the ATTILA simulator, API framework and tools.
		  ACD_DoxygenConfig.txt       Configuration file to generate DOcygen documentation for ACD classes (may be out of date).
		  ACDX_DoxygenConfig.txt      Configuration file to generate DOcygen documentation for ACDX classes (may be out of date).
		  DOxygenConfig.txt           Configuration file to generate DOxygen documentation for the simulator (quite out of date).
		  Makefile                    Makefile (old version, use the makefile in the root directory)
		  Makefile.defs               Makefile definitions (old version)
		  Makefile.new                Local makefile
		  bgpu/                       Simulator and emulator binary related classes and source code.
		  confs/                      Configuration files for the simulator (must be renamed to bGPU.ini before use).
		  D3D/                        D3D9 headers.
		  emul/                       Functional emulation classes.
		  GL/                         OpenGL headers.
		  gpu/                        Base simulation classes (Box, Signal, Statistics).
		  sim/                        Timing simulation classes.
				Cache/                  Cache related simulation classes.
				Clipper/                Clipper stage related simulated classes.
				CommandProcessor/       Command Processor stage  related simulation classes.
				DAC/                    DAC stage related simulation classe.
				FragmentOperations/     ROP (Z and Color) stage related simulation classes.
				MemoryController/       Memory Controller stage related simulation classes.  Implements a simple memory model.
				MemoryControllerV2/     Memory Controller stage related simulation classes.  Implements an accurate GDDR model.
				PrimitiveAssembly/      Primitive Assembly stage related simulation classes.
				Rasterizer/             Rasterizer (Triangle Setup, Fragment Generation, Hierarchical Z and Fragment FIFO) stage related simulation classes.
				Shader/                 Shader stage related simulation classes.
				Streamer/               Streamer (Vertex Fetch and Vertex Post-shading Cache) stage related simulation classes.
				Vector Shader/          Shader stage related simulation classes.  New implementation.
		  support/                    Common support classes.
		  TestCreator/                Framework used to create simple non-API tests for the ATTILA simulator.
		  tests/                      Tools for testing different parts of the ATTILA simulator.
		  tools/                      Miscelaneous tools.
				attilaASM/              ATTILA shader ISA tools.
				PIXparser/              PIXRun file parser.
				PPMConverter/           Coloring tools for PPMs.
				snapshotTools/          Tools for ATTILA simulator state snapshots.
				STV/                    Signal Trace Visualizer
				Trace Viewer/           Signal Trace Visualizer (old version)
		  trace/                      API related source code.
				ACD/                    The ATTILA Common Driver Layer (ACD) definition and implementation.
				ACDLTest/               Test tools for the ACDL
				ACDX/                   Extensions for the ACDL.
				ALLA/                   ATTILA Low Level API.  An experimental API to create tests for the simulator.
				AOGL/                   OpenGL API implemenation based on the ACDL.
				CodeGenerator/          Automatized code generation for the OpenGL trace tools.
				D3DCodegen/             Automatized code generation for the D3D9 PIX trace tools.
				D3DDriver/              D3D9 API implementation based on the ACDL.
				D3DPixRunPlayer/        D3D9 PIX trace reader.
				D3DPlayer4Windows/      D3D9 PIX trace player.
				D3DStadistics/          D3D9 PIX trace statistics and instrumentation tool (plugin for the D3D9 PIX trace player).
				DXInterceptor/          An independently developed D3D9 trace capturer and player (not integrated with ATTILA).
				extractTraceRegion/     Extracts a region of an AGP trace (experimental).
				gl2atila/               Runs the ATTILA API implemenation, without simulation or emulation for a trace and generates an AGP trace (experimental).
					GLInstrument/           OpenGL trace instrumentation.
				GLInstrumentTool/       OpenGL trace instrumentation.
				GLInterceptor/          OpenGL trace capturer.
				GLLib/                  OpenGL API implementation (old version not based on the ACDL).
				GLPlayer/               OpenGL trace player.
				GPUDriver/              ATTILA GPU driver.
				utils/                  Miscelaneous classes used by the API source.
	 test/                           ATTILA simulator mini-regression.
		  config/                     Configuration files used for the ATTILA simulator mini-regression test.
		  d3d/                        D3D9 traces for the ATTILA simulator mini-regression test.
		  ogl/                        OpenGL traces for the ATTILA simulator mini-regression test.
		  script/                     Scripts for the ATTILA simulator mini-regression test.


Authors
------------------------

### The Noisy Writer

   Victor Moya del Barrio (vmoya@ac.upc.edu)

### The Other Authors

   Carlos Gonzalez (cgonzale@ac.upc.edu)
   Jordi Roca (jroca@ac.upc.edu)
   Vicente Escandell (vicente@ac.upc.edu)
   Albert Murciego
   Chema Solis
   David Abella
   Christian Perez

### The Managers

   Roger Espasa (roger.espasa@intel.com)
   Agustin Fernandez (agustin@ac.upc.edu)

