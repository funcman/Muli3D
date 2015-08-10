
#include "app.h"
#include "resource.h"

#ifdef WIN32
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
#endif
#ifdef LINUX_X11
int main()
#endif
#ifdef __amigaos4__
int main(int argc, char** argv)
#endif
{
	tCreationFlags creationFlags;
	creationFlags.sWindowTitle = "LightFlare";
	
	int32 width = 640;
	int32 height = 480;

	#ifdef __amigaos4__
	if ( argc == 3 )
	{
		width = iClamp( atoi( argv[1] ), 160, 1600 );
		height = iClamp( atoi( argv[2] ), 160, 1280 );		
	}
	#endif

	#ifdef WIN32
	creationFlags.hIcon = LoadIcon( GetModuleHandle( 0 ), MAKEINTRESOURCE( IDI_ICON1 ) );
	#endif
	
	creationFlags.iWindowWidth = width;
	creationFlags.iWindowHeight = height;
	creationFlags.bWindowed = true;

	CApp theApp;
	if( !theApp.bInitialize( creationFlags ) )
		return 1;

	return theApp.iRun();
}
