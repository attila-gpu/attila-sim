#include <GLExec.h>
#include <support.h>
#include <windows.h>
#include <time.h>
 
#ifdef GLPLAYER_DEBUG
    ofstream debugDump;
#endif

#ifndef USE_FSTREAM_H
using namespace std;
#endif

static clock_t timeElapsed = 0;

int fps = 60;
char traceFileName[256] = "tracefile.txt";
char memRegFileName[256] = "MemoryRegions.dat";
char bufferDescriptorsFileName[256] = "BufferDescriptors.dat";
int frame = 1; // current frame
bool freezeFrame = false;

// synchronizing variable
bool playingFrame = false;

HWND hWnd; // main windows (global)
HDC hDC; // Handle device context
char frameText[32];

/*
 * Define function prototypes
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void enableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
void disableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
bool playFrame(GLExec& tr);
void parseConfigFile(const char* configFile);

GLExec gle;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int iCmdShow)
{
	WNDCLASS wc; // window class
	
	HGLRC hRC; // Handle GL Raster context
	MSG msg; // For messages
	BOOL quit = FALSE;
    


    parseConfigFile("config.ini");

    //GLExec gle;
    
    /**
     * Init GLExec object
     * Load opengl32.dll functions, open tracefile, open buffer file
     */
    int error = gle.init(traceFileName, bufferDescriptorsFileName, memRegFileName);
    
    if ( error == -1 )
    {
        panic("Dll", "WinMain","Error loading opengl32.dll functions");
    }
    else if ( error == -2 )
    {
        panic("Dll","WinMain","Error opening tracefile");
    }
    else if ( error == -3 )
    {
        panic("Dll","WinMain", "Error opening buffer descriptors file");
    }
    else if ( error == -4 )
    {
        panic("Dll","WinMain", "Error opening memory regions file");
    }

    
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GLSample";
	RegisterClass( &wc );

	// create main window
	hWnd = CreateWindow( 
		"GLSample", "GLPlayer 0.1", 
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
		0, 0, 800, 600,
		NULL, NULL, hInstance, NULL );    

    enableOpenGL( hWnd, &hDC, &hRC );    
    
	// program main loop
	while ( !quit )
	{
		// check for messages
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )  )
		{
			// handle or dispatch messages
			if ( msg.message == WM_QUIT ) 
				quit = TRUE;
			else 
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		} 
		else 
		{            
            

			clock_t t = clock();

			if ( freezeFrame )
			{
				static bool freezeFlag = true;
				if ( freezeFlag )
				{
					// play always the first frame
					freezeFlag = false;
					if ( playFrame(gle) )
					{
						popup("WinMain","Trace finished");
						return 0;
					}
				}
				// do nothing...
			}
			// limit clock rate setting a value for MAX_FRAMES_PER_SEC
			else if ( t - timeElapsed >= CLOCKS_PER_SEC / fps )
			{
				timeElapsed = t; // reset counter
			
				// Player code is called here...
				if ( playFrame(gle) )
				{
					popup("WinMain","Trace finished");
					return 0;
				}
				
			}
		}
	}
	
	// shutdown OpenGL
	disableOpenGL( hWnd, hDC, hRC );
	
	// destroy the window explicitly
	DestroyWindow( hWnd );
	
	return msg.wParam;	
}


// Enable OpenGL
void enableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	*hDC = GetDC( hWnd );
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( *hDC, &pfd );
	SetPixelFormat( *hDC, format, &pfd );
	
	// create and enable the render context (RC)
	*hRC = wglCreateContext( *hDC );
	wglMakeCurrent( *hDC, *hRC );	
}

// disable openGL
void disableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}


// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		
	case WM_CREATE:
		return 0;
		
	case WM_CLOSE:
		PostQuitMessage( 0 );
		return 0;
		
	case WM_DESTROY:
		return 0;
			
	case WM_CHAR:
		if ( (TCHAR)wParam == 'f' || (TCHAR)wParam == 'F' )
		{
			if ( !playingFrame )
			{
				playingFrame = true;
				if ( playFrame(gle) )
				{
					popup("WinMain","Trace finished");
					return 0;
				}
				playingFrame = false;
			}
		}

	case WM_KEYDOWN:
		switch ( wParam )
		{
			case VK_ESCAPE:
				PostQuitMessage(0);
				return 0;
		}
		return 0;
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam); // default behaviour
	}	
}


bool playFrame(GLExec& gle )
{   

	sprintf(frameText,"Playing frame %d",frame++);
	SetWindowText(hWnd,frameText);

    APICall apicall;
	bool result;

    while ( true )
    {        
        apicall = gle.getCurrentCall();

        apicall = gle.executeCurrentCall();   
        
        if ( apicall == APICall_UNDECLARED ) // if no call could be executed ( finish trace )
		{
            result = true;
			break;
		}
        else if ( (apicall == APICall_wglSwapBuffers)/* || (apicall == APICall_SwapBuffers)*/ )
		{
            result =  false; // frame has finished        
			break;
		}
    }

	SwapBuffers(hDC ); // end of frame, swap buffers
	return result;
}


void parseConfigFile( const char* configFile )
{
    ifstream config;
    config.open(configFile);    
    if ( !config.is_open() )
        return ;

    char option[1024];
    char* value;
    
    while ( !config.eof() )
    {
        config.getline(option,sizeof(option));
        value = option;

        while ( *value != '\0' && *value != '=' )
            value++;
        if ( *value == '=' )
        {
            *value = '\0';
            value++; // skip '='
            
            // opt: has an option ; value: contains the value for this option
            if ( EQ(option,"inputFile") )
                strcpy(traceFileName, value);
            else if ( EQ(option,"bufferFile") )
                strcpy(bufferDescriptorsFileName,value);
            else if ( EQ(option,"memFile") )
                strcpy(memRegFileName, value);
			else if ( EQ(option, "freezeFrame") )
			{
				if ( EQ(value,"1") || EQ(value, "YES") || EQ(value, "yes") )
					freezeFrame = true;
			}
			else if ( EQ(option, "fps") || EQ(option, "FPS") )
			{
				// no error checking...
				fps = atoi(value);
			}
        }
    }
}
