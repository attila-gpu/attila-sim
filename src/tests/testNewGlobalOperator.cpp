#include "ShaderInstruction.h"
#include "ShaderExecInstruction.h"

#define D() ShaderExecInstruction::dumpDynamicMemoryState();
#define DMEM() ShaderExecInstruction::dumpDynamicMemoryState(true);

int main( void )
{

	u32bit pc = 10;
	u32bit startCycle = 12335;

	ShaderInstruction si;

	D();
	
	ShaderExecInstruction* sei1;
	ShaderExecInstruction* sei2;


	sei1 = new ShaderExecInstruction( si, pc, startCycle );
	sei2 = new ShaderExecInstruction( si, pc+1, startCycle+23 );

	D();

	delete sei2;
		
	delete sei1;

	sei1 = new ShaderExecInstruction( si, 666, 999 );

	sei2 = 0;
	//for ( u32bit i = 0; i < 100; i++)
		delete sei2;
	
	DMEM();
	//D();

	return 0;
}