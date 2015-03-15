// Implementar PPMInspect como una tool separada
#include "PPMFile.h"
#include "PPMInspector.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

typedef PPMFile::Color Color;
typedef PPMFile::Size Size;

int processShowAux(PPMInspector& inspector, const vector<string>& params)
{
	int valuesToShowSigned = 0;
	Size valuesToShow = 0;

	if ( params.empty() )
		valuesToShow = 1;
	else
	{
		if ( params.size() > 1 )
			cout << "Extra parameters ignored\n";
		stringstream ss(params.front()); // use the first parameter
		ss >> valuesToShowSigned;
		if ( valuesToShowSigned  <= 0 )
		{
			cout << "Error parsing the extra parameter (should be a positive integer)\n";
			return -1;
		}
		valuesToShow = Size(valuesToShowSigned);
	}

	Size diffValues = inspector.countDiffRGBValues();

	if ( valuesToShow > diffValues )
	{
		cout << "Values to be shown are greater than the total amount of different values\n";
		valuesToShow = diffValues;
	}

	if ( valuesToShow > 50 )
	{
		char answer = 0;
		while ( true )
		{
			cout << "More than 50 values selected. Are you sure ? (y/n): ";
			cin >> answer;
			if ( answer != 'y' && answer != 'n' )
				cout << "\ny or n required" << endl;
			else
				break;
		}
		if ( answer == 'n' )
		{
			cout << "Output not shown. Bye" << endl;
			return 0; // Do not show values
		}
	}
	return valuesToShow;
}

int processDiffsCommand(PPMInspector& inspector)
{
	cout << "Different values: " << inspector.countDiffRGBValues() << endl;
	return 0; // Ok
}

int processMinCommand(PPMInspector& inspector, const vector<string>& params)
{
	int valuesToShowSigned = processShowAux(inspector, params);
	if ( valuesToShowSigned <= 0 )
		return valuesToShowSigned;

	Size valuesToShow = Size(valuesToShowSigned);

	// Show values
	const vector<Color>& dv = inspector.getDifferentRGBValues(); // Get sorted values
	if ( valuesToShow == 1 )
		cout << "Min value: ";
	else
		cout << "Min values: \n";

	for ( Size i = 0; i < valuesToShow; i++ )
	{
		cout << "(" << Size(dv[i].r) << "," << Size(dv[i].g) << "," << Size(dv[i].b) 
			 << ") = " << dv[i].toValue() << endl;
	}

	return 0;
	
}

int processMaxCommand(PPMInspector& inspector, const vector<string>& params)
{
	int valuesToShowSigned = processShowAux(inspector, params);
	if ( valuesToShowSigned <= 0 )
		return valuesToShowSigned;

	Size valuesToShow = Size(valuesToShowSigned);

	// Show values
	const vector<Color>& dv = inspector.getDifferentRGBValues(); // Get sorted values
	if ( valuesToShow == 1 )
		cout << "Max value: ";
	else
		cout << "Max values: \n";

	for ( Size i = dv.size()-1; valuesToShow > 0; i--, valuesToShow-- )
	{
		cout << "(" << Size(dv[i].r) << "," << Size(dv[i].g) << "," << Size(dv[i].b) 
			 << ") = " << dv[i].toValue() << endl;
	}

	return 0;
}

int processSizeCommand(PPMInspector& inspector)
{
	const PPMFile& ppm = inspector.ppm();
	cout << "Width: " << ppm.width() << "  Height: " << ppm.height() << 
		"\nData size: " << (ppm.width() * ppm.height() * 3) << " bytes " << endl;

	return 0;
}

int generalInspection(PPMInspector& inspector)
{
	int ret = 0;
	if ( (ret = processSizeCommand(inspector)) < 0 )
		return ret;
	if ( (ret = processDiffsCommand(inspector)) < 0 )
		return ret;

	vector<string> params;
	params.push_back(string("1"));
	if ( (ret = processMinCommand(inspector,params)) < 0 )
		return ret;
	if ( (ret = processMaxCommand(inspector,params)) < 0 )
		return ret;

	return 0;
}

int main(int argc, char* argv[])
{
	if ( argc < 2 )
	{
		string cmd(argv[0]);
		cout << "Command line format: " << cmd << " inputFile.ppm [Query] [Query_params]\n";
		cout << "[Query] can be: {diffs, min, max, size}\n"
			"[Query_params] depends on the selected query\n"
			"   diffs and size queries have no extra query params\n"
		    "   max and min have an optional param (how many results are desired)\n";
		cout << "[Query] and [Query_params] can be omitted to obtain all the ppm information\n";
		return 0;
	}

	PPMFile ppm;
	try
	{
		ppm.read(argv[1]); // open file
	}
	catch (PPMFile::PPMException& e)
	{
		cout << "Exception while reading the input file: " << e.toString() << endl;
		return -1;
	}

	PPMInspector inspector(ppm); // Create a PPM inspector for the current ppm file

	if ( argc == 2 ) // general inspection
		return generalInspection(inspector);

	string cmd(argv[2]); // command string
	vector<string> params;
	for ( int i = 3; i < argc; i++ )
		params.push_back(string(argv[i]));

	if ( cmd == "diffs" )
	{
		if ( !params.empty() )
			cout << "Extra parameters ignored.\n";
		return processDiffsCommand(inspector);
	}
	else if ( cmd == "min" )
		return processMinCommand(inspector, params);
	else if ( cmd == "max" )
		return processMaxCommand(inspector, params);
	else if ( cmd == "size" )
	{
		if ( !params.empty() )
			cout << "Extra parameters ignored.\n";
		return processSizeCommand(inspector);
	}
	else
	{
		cout << "Unknown command" << endl;
		return -1;
	}

	return 0;
}




