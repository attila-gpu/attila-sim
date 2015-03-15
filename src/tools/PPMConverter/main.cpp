#include "PPMFile.h"
#include <iostream>
#include <fstream>
#include <map>
#include "PPMInspector.h"
#include "FragmentMapConverter.h"


using namespace std;

typedef unsigned int Size;
typedef unsigned char Byte;

string getCommandName(string path)
{
#ifdef WIN32 // Windows
	string separator("\\");
#else // Linux/Unix
	string separator("/");
#endif
	size_t pos = path.find_last_of(separator);
	if ( pos != string::npos )
		return path.substr(pos);
	return path; // the path is the command
}

int main(int argc, char* argv[])
{
	string cmdName = getCommandName(argv[0]);
	/*
	if ( argc == 2 ) // obtain information about the PPM file
	{
		PPMFile ppm;
		try
		{
			ppm.read(argv[1]);
		}
		catch ( PPMFile::PPMException& e )
		{
			cout << "Input file error: " << e.toString() << endl;
			return -1;
		}
		// File opened an read correctly
		cout << "Inspecting " << argv[1] << " file...\n";
		PPMInspector inspector(ppm);
		cout << "Different RGB values in the PPMFile: " 
			 << inspector.countDiffRGBValues();
		PPMFile::Color minCol = inspector.getMinRGBValue();
		PPMFile::Color maxCol = inspector.getMaxRGBValue();

		cout << "\nMin value: " << minCol.toValue();
		cout << " RGB = {" << int(minCol.r) << "," << int(minCol.g) << "," << int(minCol.b) << "}\n";
		cout << "Max value: " << maxCol.toValue();
		cout << " RGB = {" << int(maxCol.r) << "," << int(maxCol.g) << "," << int(maxCol.b) << "}\n";
		return 0;
	}
	*/

	if ( argc != 4 )
	{
		
		cout << "----------------------------------------------------------------\n"
				"Command " << cmdName << " help:\n"
				" The command format expected is:\n  " << cmdName << " [FUNCTION]"
			    "  [COLORS_FILE] [PPMFILE]\n"
				"  - [FUNCTION] values can be 'linear' or 'group'\n"
				"  - [COLORS_FILE] can be a file with the list of RGB values or\n"
			    "  -               redN, greenN or blueN being N a value in the range\n"
				"                  [2..255]\n"
				" Example 1:\n  " << cmdName << " linear colors.dat inputFile.ppm\n"
				" Example 2:\n  " << cmdName << " group blue20 inputFile.ppm\n"
				"----------------------------------------------------------------\n";

		return -1;
	}

	if ( string("linear") != argv[1] && string("group") != argv[1] )
	{
		cout << "Invalid mapping function" << endl;
		return -1;
	}

	unsigned int colorID = 0; // 0-> use file, 1 red, 2 green, 3 blue
	unsigned int tonalities = 0;

	ifstream colors;
	colors.open(argv[2]);
	if ( !colors.is_open() )
	{
		// Check if color file is a "special" color name
		string special(argv[2]);
		unsigned int pos = special.find_last_not_of("0123456789");
		if ( pos != string::npos )
		{
			string col = special.substr(0,pos+1);
			if ( col == "red" )
				colorID = 1;
			else if ( col == "green" )
				colorID = 2;
			else if ( col == "blue" )
				colorID = 3;
			else
			{
				cout << "Input color file: '" << argv[2] << "' could not be opened" << endl;
				return -1;
			}
			tonalities = atoi(special.substr(pos+1).c_str());
			if ( 2 > tonalities || tonalities > 255 )
			{
				cout << "Input color file: '" << argv[2] << "' could not be opened" << endl;
				return -1;
			}
		}
		else
		{
			cout << "Input color file: '" << argv[2] << "' could not be opened" << endl;
			return -1;
		}
	}

	PPMFile ppm;	
	
	try
	{
		ppm.read(argv[3]);
	}
	catch ( PPMFile::PPMException& e )
	{
		cout << e.toString() << endl;
		return -1;
	}
	catch ( ... )
	{
		cout << "Unexpected exception while reading the PPM file (aborting)" << endl;
		return -1;
	}

	// Create a Fragment converter based of mapping funcion 
	// (for now only "redValue" supported)
	string mapFunc = argv[1];
	FragmentMapConverter* fmc = FragmentMapConverterFactory::create(mapFunc);
	if ( fmc == 0 )
	{
		cout << "Fragment map converter with string name: '" << argv[1] 
		     << "' not available" << endl;
		return -1;
	}

	if ( colorID == 0 )
	{
		// Process colors from file
		while ( !colors.eof() )
		{
			unsigned int r = 256;
			unsigned int g = 256;
			unsigned int b = 256;		
			colors >> r >> g >> b;
			if ( r > 255 || g > 255 || b > 255 )
				break; 
			// Color read
			fmc->addColor(r,g,b);
		}
	}
	else
	{
		// Computar número de colores necesarios (1 por tonalidad)
		const double step = double(255) / double(tonalities);
		double currentTonality = 255.0;
		for ( unsigned int i = 1; i <= tonalities; i++, currentTonality -= step )
		{
			if  ( colorID == 1 )
				fmc->addColor(Byte(currentTonality),0,0);
			else if ( colorID == 2 )
				fmc->addColor(0, Byte(currentTonality),0);
			else
				fmc->addColor(0, 0, Byte(currentTonality));
		}
	}

	if ( fmc->countColors() == 0 )
	{
		delete fmc;
		cout << "Colors not found in '" << argv[2] << "' input color file" << endl;
		return -1;
	}

	PPMFile result;

	// Apply transformation
	try
	{
		result = fmc->transform(ppm);
	}
	catch ( FragmentMapConverter::TransformationException& te )
	{
		cout << "Exception: " << te.reason() << endl;
		return -2;
	}

	try
	{
		string outputFile = string("output_") + argv[1] + "_" + argv[3];
		result.write(outputFile);
	}
	catch ( PPMFile::PPMException& e )
	{
		cout << "Exception found: " << e.toString() << endl;
		return -2;
	}
	catch ( ... )
	{
		cout  << "Unexpected exception... Aborting" << endl;
		return -3;
	}

	delete fmc;
	return 0;
}
