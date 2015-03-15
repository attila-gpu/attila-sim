////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXCodeGenException.h"
#include "Config/ConfigManager.h"
#include "Config/ParserConfiguration.h"
#include "Config/GeneratorConfiguration.h"
#include "Items/CppMacro.h"
#include "Parser/DXHParser.h"
#include "Generator/DXHGenerator.h"
#include "main.h"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dxcodegen;
using namespace dxcodegen::Config;
using namespace dxcodegen::Items;
using namespace dxcodegen::Parser;
using namespace dxcodegen::Generator;

////////////////////////////////////////////////////////////////////////////////

const unsigned int APP_VERSION = 100;

////////////////////////////////////////////////////////////////////////////////

void ShowBanner()
{
  cout << "DirectX Code Generator v" << APP_VERSION / 100 << "." << setw(2) << setfill('0') << APP_VERSION % 100 << endl;
  cout << "Copyright (c) 2007 David Abella. All Rights Reserved." << endl;
  cout << endl;
}

////////////////////////////////////////////////////////////////////////////////

void ShowSyntax()
{
  cout << "Syntax:" << endl;
  cout << endl;
  cout << "\tdxcodegen [options] <config-file>" << endl;
  cout << endl;

  cout << "Options:" << endl;
  cout << endl;
  cout << "\t--verbose [-V]    verbose output" << endl;
  cout << "\t--help    [-H]    show this help" << endl;
  cout << endl;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  bool verbose = false;
  string filename = "";

  if (argc >= 2 && argc <= 3)
  {
    if (string(argv[1]).substr(0, 1).compare("-") == 0)
    {
      if ((string(argv[1]).compare("-H") == 0) || (string(argv[1]).compare("--help") == 0))
      {
        ShowBanner();
        ShowSyntax();
        return 0;
      }
      else if ((string(argv[1]).compare("-V") == 0) || (string(argv[1]).compare("--verbose") == 0))
      {
        if (argc == 2 || string(argv[2]).empty())
        {
          ShowBanner();

          cout << "ERROR: needs an input config file" << endl;
          cout << endl;

          ShowSyntax();
          return -1;
        }

        ShowBanner();
        verbose = true;
        filename = argv[2];
      }
      else
      {
        ShowBanner();

        cout << "ERROR: unknow option '" << argv[1] << "'" << endl;
        cout << endl;

        ShowSyntax();
        return -1;
      }
    }
    else
    {
      ShowBanner();
      filename = argv[1];
    }
  }
  else
  {
    ShowBanner();
    ShowSyntax();
    return 0;
  }

  try
  {
    ConfigManager config(filename, verbose, false);

    ParserConfiguration configParser;
    config.GetParserConfiguration(configParser);

    DXHParser parser(configParser);
    parser.ParseFiles();

    GeneratorConfiguration configGenerator;
    config.GetGeneratorConfiguration(configGenerator);

    DXHGenerator generator(configGenerator);
    generator.AddClasses(parser.GetClasses());
    generator.AddEnumerations(parser.GetEnumerations());
    generator.AddStructures(parser.GetStructures());
    generator.GenerateCode();
	}
	catch (DXCodeGenException& e)
	{
		cout << "INTERNAL EXCEPTION: " << e.what() << endl;
    return -1;
	}
	catch (exception& e)
	{
    cout << "EXTERNAL EXCEPTION: " << e.what() << endl;
    return -1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
