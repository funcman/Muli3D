
#include "../include/application.h"

#include "../include/input.h"
#include "../include/fileio.h"
#include "../include/graphics.h"
#include "../include/scene.h"
#include "../include/resmanager.h"

static CApplication *g_pApp = 0;

IApplication::IApplication()
{
	m_fFPS = 0.0f;
	m_fInvFPS = 0.0f;
	m_fElapsedTime = 0.0f;

	m_hWindowHandle = 0;
	m_bWindowed = true;
	m_bActive = false;
	m_strWindowTitle = "<default>";
	m_iWindowWidth = 640;
	m_iWindowHeight = 480;

	m_iFrameIdent = 0;

	m_pAppData = 0;

	m_pInput = 0;
	m_pFileIO = 0;
	m_pGraphics = 0;
	m_pScene = 0;
	m_pResManager = 0;
}

IApplication::~IApplication()
{
	SAFE_DELETE_ARRAY( m_pAppData );

	SAFE_DELETE( m_pScene );
	SAFE_DELETE( m_pResManager );
	SAFE_DELETE( m_pGraphics );
	SAFE_DELETE( m_pFileIO );
	SAFE_DELETE( m_pInput );
}

bool IApplication::bCreateSubSystems( const tCreationFlags &i_creationFlags )
{
	// NOTE: add support for other platforms here
	#ifdef WIN32
		m_pInput = new CInputWin32( this );
	#endif
	#ifdef LINUX_X11
		m_pInput = new CInputLinuxX11( this );
	#endif
	#ifdef __amigaos4__
		m_pInput = new CInputAmigaOS4( this );
	#endif

	// NOTE: should handle possible failing! maybe bad_alloc...

	if( !m_pInput->bInitialize() )
		return false;

	m_pFileIO = new CFileIO( this );
	if( !m_pFileIO->bInitialize() )
		return false;

	m_pGraphics = new CGraphics( this );
	if( !m_pGraphics->bInitialize( i_creationFlags ) )
		return false;

	m_pResManager = new CResManager( this );
	if( !m_pResManager->bInitialize() )
		return false;

	m_pScene = new CScene( this );
	if( !m_pScene->bInitialize() )
		return false;

	if( !bCreateWorld() )
	{
		DestroyWorld();
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------

#ifdef WIN32

CApplication::CApplication()
{
	g_pApp = this;

	m_iTicksPerSecond.QuadPart = 0;
	m_iStartTime.QuadPart = 0;
	m_iLastTime.QuadPart = 0;
}

CApplication::~CApplication()
{
	UnregisterClass( "<application window>", GetModuleHandle( 0 ) );
}

bool CApplication::bInitialize( const tCreationFlags &i_creationFlags )
{
	m_strWindowTitle = i_creationFlags.sWindowTitle;
	m_iWindowWidth = i_creationFlags.iWindowWidth;
	m_iWindowHeight = i_creationFlags.iWindowHeight;
	m_bWindowed = i_creationFlags.bWindowed;

	// Initialize the timer ---------------------------------------------------
	if( !QueryPerformanceFrequency( &m_iTicksPerSecond ) )
		return false;

	// Create the render window -----------------------------------------------
    WNDCLASS windowClass = { 0, &WindowProcedure, 0, 0, GetModuleHandle( 0 ),
		i_creationFlags.hIcon, 0, 0, 0, "<application window>" };
    RegisterClass( &windowClass );

	uint32 iWindowStyle = WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU;

    RECT rc;
	SetRect( &rc, 0, 0, m_iWindowWidth, m_iWindowHeight );
    AdjustWindowRect( &rc, iWindowStyle, false );

    m_hWindowHandle = CreateWindow( "<application window>", m_strWindowTitle.c_str(), iWindowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, (rc.right - rc.left), (rc.bottom - rc.top),
		0, 0, GetModuleHandle( 0 ), 0 );

	SetCursor( LoadCursor( 0, IDC_WAIT ) );

	return bCreateSubSystems( i_creationFlags );
}

int CApplication::iRun()
{
	ShowWindow( m_hWindowHandle, SW_SHOW );
	SetFocus( m_hWindowHandle );
	SetCursor( LoadCursor( 0, IDC_ARROW ) );
	m_bActive = true;

	// Begin application loop -------------------------------------------------

	while( bCheckMessages() )
	{
		BeginFrame();
		RenderWorld();
		EndFrame();

		Sleep( 1 );
	}

	// End of application loop = termination of program -----------------------

	DestroyWorld();

	return 0;
}

void CApplication::BeginFrame()
{
	if( !m_iStartTime.QuadPart )
	{
		QueryPerformanceCounter( &m_iStartTime );
		m_iLastTime.QuadPart = m_iStartTime.QuadPart;
	}

	++m_iFrameIdent;

	m_pInput->Update();			// Get latest keyboard and mouse state

	FrameMove();
	m_pScene->FrameMove();
}

bool CApplication::bCheckMessages()
{
	while( true )
	{
		MSG msgMessage;

		if( m_bActive )
		{
			if( !PeekMessage( &msgMessage, 0, 0, 0, PM_REMOVE ) )
				break;
		}
		else
		{
			if( !GetMessage( &msgMessage, 0, 0, 0 ) )
				break;
		}

		TranslateMessage( &msgMessage );
		DispatchMessage( &msgMessage );

		if( msgMessage.message == WM_QUIT )
			return false;
	}

	return true;
}

void CApplication::EndFrame()
{
	LARGE_INTEGER iCurrentTime;
	QueryPerformanceCounter( &iCurrentTime );

	float32 fTimeDifference = (float32)(iCurrentTime.QuadPart - m_iLastTime.QuadPart);
	m_fElapsedTime += fTimeDifference / (float32)m_iTicksPerSecond.QuadPart;
	m_fFPS = (float32)m_iTicksPerSecond.QuadPart / fTimeDifference;

	m_fInvFPS = 1.0f / m_fFPS;
	m_iLastTime.QuadPart = iCurrentTime.QuadPart;
}

LRESULT CALLBACK CApplication::WindowProcedure( HWND i_hWnd, UINT i_uMsg, WPARAM i_wParam, LPARAM i_lParam )
{
	return g_pApp->MessageProcedure( i_hWnd, i_uMsg, i_wParam, i_lParam );
}

LRESULT CApplication::MessageProcedure( HWND i_hWnd, UINT i_uMsg, WPARAM i_wParam, LPARAM i_lParam )
{
	switch( i_uMsg )
    {
	case WM_ACTIVATE:
		m_bActive = (LOWORD( i_wParam ) != WA_INACTIVE);
		break;

	case WM_ENTERSIZEMOVE:
		m_bActive = false;
        break;

	case WM_EXITSIZEMOVE:
        m_bActive = true;
        break;

    case WM_SETCURSOR:
        // Turn off Windows cursor in fullscreen mode
        if( m_bActive && !m_bWindowed )
        {
            SetCursor( 0 );
            return true; // prevent Windows from setting cursor to window class cursor
        }
        break;

    case WM_POWERBROADCAST:
        return false; // don't allow suspend/standby

    case WM_SYSCOMMAND:
        // Prevent moving/sizing and power loss in fullscreen mode
        switch( i_wParam )
        {
            case SC_MOVE:
            case SC_SIZE:
            case SC_MAXIMIZE:
            case SC_KEYMENU:
            case SC_MONITORPOWER:
                if( !m_bWindowed )
                    return 1;
                break;
        }
        break;

    case WM_CLOSE:
        PostQuitMessage( 0 );
        return 0;
    }

	return DefWindowProc( i_hWnd, i_uMsg, i_wParam, i_lParam );
}

#endif

// ----------------------------------------------------------------------------

#ifdef LINUX_X11

CApplication::CApplication()
{
	g_pApp = this;

	m_pDisplay = 0;
	m_hWindowHandle = 0;

	m_bGotStartTime = false;
}

CApplication::~CApplication()
{
	if( m_hWindowHandle )
	{
		XDestroyWindow( m_pDisplay, m_hWindowHandle );
		m_hWindowHandle = 0;
	}

	if( m_pDisplay )
	{
		XCloseDisplay( m_pDisplay );
		m_pDisplay = 0;
	}
}

bool CApplication::bInitialize( const tCreationFlags &i_creationFlags )
{
	if( !i_creationFlags.bWindowed ) // only windowed mode is supported
		return false;

	m_strWindowTitle = i_creationFlags.sWindowTitle;
	m_iWindowWidth = i_creationFlags.iWindowWidth;
	m_iWindowHeight = i_creationFlags.iWindowHeight;
	m_bWindowed = i_creationFlags.bWindowed;

	// Create the render window -----------------------------------------------
	m_pDisplay = XOpenDisplay( 0 );
	if( !m_pDisplay )
		return false;

	int32 iScreen = DefaultScreen( m_pDisplay );

	XSetWindowAttributes WindowAttributes;
	WindowAttributes.border_pixel = BlackPixel( m_pDisplay, iScreen );
	WindowAttributes.background_pixel = BlackPixel( m_pDisplay, iScreen );
	WindowAttributes.backing_store = NotUseful;

	m_hWindowHandle = XCreateWindow( m_pDisplay, DefaultRootWindow( m_pDisplay ), 0,
		   0, m_iWindowWidth, m_iWindowHeight, 0, DefaultDepth( m_pDisplay, iScreen ),
		   InputOutput, DefaultVisual( m_pDisplay, iScreen ),
		   CWBackPixel | CWBorderPixel | CWBackingStore,
		   &WindowAttributes );

	XStoreName( m_pDisplay, m_hWindowHandle, m_strWindowTitle.c_str() );

	XSizeHints WindowSizehints;
	WindowSizehints.flags = PPosition | PMinSize | PMaxSize;
	WindowSizehints.x = 0;
	WindowSizehints.y = 0;
	WindowSizehints.min_width = m_iWindowWidth;
	WindowSizehints.min_height = m_iWindowHeight;
	WindowSizehints.max_width = m_iWindowWidth;
	WindowSizehints.max_height = m_iWindowHeight;
	XSetWMNormalHints( m_pDisplay, m_hWindowHandle, &WindowSizehints );

	m_atomCloseWindow = XInternAtom( m_pDisplay, "WM_DELETE_WINDOW", true );
	if( !m_atomCloseWindow )
		return false;

	XSetWMProtocols( m_pDisplay, m_hWindowHandle, &m_atomCloseWindow, 1 );
	XSelectInput( m_pDisplay, m_hWindowHandle, ClientMessage );

	return bCreateSubSystems( i_creationFlags );
}

#include <unistd.h> // usleep()
int CApplication::iRun()
{
	XClearWindow( m_pDisplay, m_hWindowHandle );
	XMapRaised( m_pDisplay, m_hWindowHandle );
	XFlush( m_pDisplay );

	m_bActive = true;

	// Begin application loop -------------------------------------------------

	while( bCheckMessages() )
	{
		BeginFrame();
		RenderWorld();
		EndFrame();

		usleep( 100 );
	}

	// End of application loop = termination of program -----------------------

	DestroyWorld();

	return 0;
}

void CApplication::BeginFrame()
{
	if( !m_bGotStartTime )
	{
		gettimeofday( &m_StartTime, &m_TimeZone );
		memcpy( &m_LastTime, &m_StartTime, sizeof( m_LastTime ) );
		m_bGotStartTime = true;
	}

	++m_iFrameIdent;

	m_pInput->Update();			// Get latest keyboard and mouse state

	FrameMove();
	m_pScene->FrameMove();
}

bool CApplication::bCheckMessages()
{
	if( XPending( m_pDisplay ) )
	{
		XEvent theXEvent;
		XNextEvent( m_pDisplay, &theXEvent );
		switch( theXEvent.type )
		{
		case ClientMessage:
			if( (Atom)theXEvent.xclient.data.l[0] == m_atomCloseWindow )
				return false;
		default:
			break;
		}
	}

	return true;
}

void CApplication::EndFrame()
{
	struct timeval theCurrentTime;
	gettimeofday( &theCurrentTime, &m_TimeZone );

	float64 fLastTime = (float64)m_LastTime.tv_sec + (float64)m_LastTime.tv_usec/(1000*1000);
	float64 fCurrentTime = (float64)theCurrentTime.tv_sec + (float64)theCurrentTime.tv_usec/(1000*1000);

	float32 fTimeDifference = (float32)(fCurrentTime - fLastTime);
	m_fElapsedTime += fTimeDifference;
	m_fFPS = 1.0f / fTimeDifference;

	m_fInvFPS = fTimeDifference; //1.0f / m_fFPS;
	memcpy( &m_LastTime, &theCurrentTime, sizeof( m_LastTime ) );
}

#endif

// ----------------------------------------------------------------------------

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/picasso96api.h>

CApplication::CApplication()
{
	g_pApp = this;
	m_pScreen = 0;
	m_hWindowHandle = 0;
	m_bGotStartTime = false;
}

CApplication::~CApplication()
{
	if( m_hWindowHandle )
	{
		IIntuition->CloseWindow( m_hWindowHandle );
		m_hWindowHandle = 0;
	}

	// If there is custom screen opened, close it
	if( m_pScreen )
	{
		IP96->p96CloseScreen( m_pScreen );
		m_pScreen = 0;
	}
}

bool CApplication::bInitialize( const tCreationFlags &i_creationFlags )
{
	m_strWindowTitle = i_creationFlags.sWindowTitle;
	m_iWindowWidth = i_creationFlags.iWindowWidth;
	m_iWindowHeight = i_creationFlags.iWindowHeight;
	m_bWindowed = i_creationFlags.bWindowed;

	// Create the render window -----------------------------------------------

	// Flag to select between custom and public screen
	bool bUseCustomScreen = false;

	// Fullscreen
	if ( !m_bWindowed )
	{
		// Try to find best possible screen configuration
		ULONG displayId = IP96->p96BestModeIDTags(
			P96BIDTAG_NominalWidth, m_iWindowWidth,
			P96BIDTAG_NominalHeight, m_iWindowHeight,
			P96BIDTAG_Depth, 24,
			TAG_DONE );

		if ( displayId == INVALID_ID )
		{
			FUNC_NOTIFY( "CApplication::bInitialize: couldn't find proper custom screen configuration. Will fall-back to window mode.\n" );
		}
		else
		{
			m_pScreen = IP96->p96OpenScreenTags(P96SA_DisplayID, displayId, TAG_DONE);
			if ( !m_pScreen )
			{
				FUNC_NOTIFY( "CApplication::bInitialize: couldn't open custom screen. Will fall-back to window mode.\n" );
			}
			else
				bUseCustomScreen = true;
		}
	}

	// Window on the public screen (Workbench)
	if ( !m_pScreen )
	{
		m_pScreen = IIntuition->LockPubScreen( 0 );
		if (!m_pScreen)
		{
			FUNC_FAILING( "CApplication::bInitialize: couldn't lock public screen.\n" );
			return false; // probably won't happen
		}
	}

	// Open window a public screen
	//
	if ( !(m_hWindowHandle = IIntuition->OpenWindowTags(
		0,
		WA_Title, m_strWindowTitle.c_str(),
		WA_CustomScreen, m_pScreen,
		WA_InnerWidth, m_iWindowWidth,
		WA_InnerHeight, m_iWindowHeight,
		WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS,
		WA_CloseGadget, TRUE,
		WA_DragBar, TRUE,
		WA_DepthGadget, TRUE,
		WA_RMBTrap, TRUE,
		WA_Borderless, bUseCustomScreen ? TRUE : FALSE,
		TAG_END
		) ) )
	{
		FUNC_FAILING( "CApplication::bInitialize: couldn't open window.\n" );
		return e_unknown;
	}

	// When public screen is used, we unlock our active screen and clear the pointer
	// to avoid trying to close public screen in destructor!
	if ( !bUseCustomScreen )
	{
		IIntuition->UnlockPubScreen( 0, m_pScreen );
		m_pScreen = 0;
	}

	return bCreateSubSystems( i_creationFlags );
}

#include <unistd.h> // usleep()
int CApplication::iRun()
{
	m_bActive = true;

	// Begin application loop -------------------------------------------------

	while( bCheckMessages() )
	{
		BeginFrame();
		RenderWorld();
		EndFrame();

		usleep( 100 );
	}

	// End of application loop = termination of program -----------------------

	DestroyWorld();

	return 0;
}

void CApplication::BeginFrame()
{
	if( !m_bGotStartTime )
	{
		gettimeofday( &m_StartTime, &m_TimeZone );
		memcpy( &m_LastTime, &m_StartTime, sizeof( m_LastTime ) );
		m_bGotStartTime = true;
	}

	++m_iFrameIdent;

	m_pInput->Update();			// Get latest keyboard and mouse state

	FrameMove();
	m_pScene->FrameMove();
}

bool CApplication::bCheckMessages()
{
	struct IntuiMessage* msg = 0;
	struct MsgPort* port = m_hWindowHandle->UserPort;
	while ( ( msg = (struct IntuiMessage*)IExec->GetMsg( port ) ) )
	{
		ULONG cl = msg->Class;
		UWORD code = msg->Code;

		IExec->ReplyMsg((struct Message*)msg);

		switch( cl )
		{
		// Pressing close gadget will quit application
		case IDCMP_CLOSEWINDOW: return false;
		case IDCMP_MOUSEBUTTONS:
			switch ( code )
			{
			// Not very clean but works: set mouse button flag in Input class...
			case SELECTDOWN: static_cast<CInputAmigaOS4*>(m_pInput)->m_bLMB = true; break;
			case SELECTUP: static_cast<CInputAmigaOS4*>(m_pInput)->m_bLMB = false; break;
			case MENUDOWN: static_cast<CInputAmigaOS4*>(m_pInput)->m_bRMB = true; break;
			case MENUUP: static_cast<CInputAmigaOS4*>(m_pInput)->m_bRMB = false; break;
			default: break;
			} // code
			break;
		default: break;
		} // cl
	}

	return true;
}

void CApplication::EndFrame()
{
	struct timeval theCurrentTime;
	gettimeofday( &theCurrentTime, &m_TimeZone );

	float64 fLastTime = (float64)m_LastTime.tv_sec + (float64)m_LastTime.tv_usec/(1000*1000);
	float64 fCurrentTime = (float64)theCurrentTime.tv_sec + (float64)theCurrentTime.tv_usec/(1000*1000);

	float32 fTimeDifference = (float32)(fCurrentTime - fLastTime);
	m_fElapsedTime += fTimeDifference;
	m_fFPS = 1.0f / fTimeDifference;

	m_fInvFPS = fTimeDifference; //1.0f / m_fFPS;
	memcpy( &m_LastTime, &theCurrentTime, sizeof( m_LastTime ) );
}

#endif
