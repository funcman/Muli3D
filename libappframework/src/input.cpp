
#include "../include/input.h"
#include "../include/application.h"

#ifdef WIN32

CInputWin32::CInputWin32( IApplication *i_pParent ) : IInput( i_pParent )
{
	m_pDirectInput = 0;
	m_pDIDeviceKeyboard = 0;
	m_pDIDeviceMouse = 0;

	ZeroMemory( m_cKeys, sizeof( m_cKeys ) );
	ZeroMemory( &m_MouseState, sizeof( m_MouseState ) );
}

CInputWin32::~CInputWin32()
{
	UnacquireDevices();
	SAFE_RELEASE( m_pDIDeviceMouse );
	SAFE_RELEASE( m_pDIDeviceKeyboard );
	SAFE_RELEASE( m_pDirectInput );
}

bool CInputWin32::bInitialize()
{
	// Initialize DirectInput and create DI devices ---------------------------
	if( FAILED( DirectInput8Create( GetModuleHandle( 0 ),
		DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDirectInput, 0 ) ) )
	{
		return false;
	}

	// Create keyboard device -------------------------------------------------
	if( FAILED( m_pDirectInput->CreateDevice( GUID_SysKeyboard,
		&m_pDIDeviceKeyboard, 0 ) ) )
	{
		return false;
	}

	m_pDIDeviceKeyboard->SetDataFormat( &c_dfDIKeyboard );
	m_pDIDeviceKeyboard->SetCooperativeLevel( m_pParent->hGetWindowHandle(), DISCL_NONEXCLUSIVE|DISCL_FOREGROUND );

	// Create mouse device ----------------------------------------------------
	if( FAILED( m_pDirectInput->CreateDevice( GUID_SysMouse,
		&m_pDIDeviceMouse, 0 ) ) )
	{
		return false;
	}

	m_pDIDeviceMouse->SetDataFormat( &c_dfDIMouse );
	m_pDIDeviceMouse->SetCooperativeLevel( m_pParent->hGetWindowHandle(), DISCL_NONEXCLUSIVE|DISCL_FOREGROUND );

	AcquireDevices();

	return true;
}

void CInputWin32::Update()
{
	if( FAILED( m_pDIDeviceKeyboard->GetDeviceState( sizeof( m_cKeys ), (void *)m_cKeys ) ) )
	{
		// not acquired? acquire and try again ...
		m_pDIDeviceKeyboard->Acquire();
		m_pDIDeviceKeyboard->GetDeviceState( sizeof( m_cKeys ), (void *)m_cKeys );
	}

	if( FAILED( m_pDIDeviceMouse->GetDeviceState( sizeof( DIMOUSESTATE ), (void *)&m_MouseState ) ) )
	{
		// not acquired? acquire and try again ...
		m_pDIDeviceMouse->Acquire();
		m_pDIDeviceMouse->GetDeviceState( sizeof( DIMOUSESTATE ), (void *)&m_MouseState );
	}
}

void CInputWin32::AcquireDevices()
{
	m_pDIDeviceKeyboard->Acquire();
	m_pDIDeviceMouse->Acquire();
}

void CInputWin32::UnacquireDevices()
{
	m_pDIDeviceKeyboard->Unacquire();
	m_pDIDeviceMouse->Unacquire();
}

#endif

// ----------------------------------------------------------------------------

#ifdef LINUX_X11

CInputLinuxX11::CInputLinuxX11( IApplication *i_pParent ) : IInput( i_pParent )
{
	 // TODO
}

CInputLinuxX11::~CInputLinuxX11()
{
	 // TODO
}

bool CInputLinuxX11::bInitialize()
{
	 // TODO
	return true;
}

void CInputLinuxX11::Update()
{
	 // TODO
}

bool CInputLinuxX11::bKeyDown( char i_cKey )
{
	return false; // TODO
}

bool CInputLinuxX11::bKeyUp( char i_cKey )
{
	return false; // TODO
}

bool CInputLinuxX11::bButtonDown( int i_iButton )
{
	return false; // TODO
}

bool CInputLinuxX11::bButtonUp( int i_iButton )
{
	return false; // TODO
}

void CInputLinuxX11::GetMovement( int32 *i_pDX, int32 *i_pDY )
{
	*i_pDX = 0; *i_pDY = 0; // TODO
}

int CInputLinuxX11::iGetWheelMovement()
{
	return 0; //TODO
}

#endif

// ----------------------------------------------------------------------------

#ifdef __amigaos4__

CInputAmigaOS4::CInputAmigaOS4( IApplication *i_pParent ) : IInput( i_pParent )
{
	 // TODO
}

CInputAmigaOS4::~CInputAmigaOS4()
{
	 // TODO
}

bool CInputAmigaOS4::bInitialize()
{
	 // TODO
	m_bLMB = false;
	m_bRMB = false;
	return true;
}

void CInputAmigaOS4::Update()
{
	 // TODO
}

bool CInputAmigaOS4::bKeyDown( char i_cKey )
{
	return false; // TODO
}

bool CInputAmigaOS4::bKeyUp( char i_cKey )
{
	return false; // TODO
}

bool CInputAmigaOS4::bButtonDown( int i_iButton )
{
	switch( i_iButton )
	{
	case 0: return m_bLMB;
	case 1: return m_bRMB;
	}
	return false;
}

bool CInputAmigaOS4::bButtonUp( int i_iButton )
{
	switch( i_iButton )
	{
	case 0: return m_bLMB == false;
	case 1: return m_bRMB == false;
	}
	return false;
}

void CInputAmigaOS4::GetMovement( int32 *i_pDX, int32 *i_pDY )
{
	static int32 iOldX = 0, iOldY = 0;

	*i_pDX = m_pParent->hGetWindowHandle()->MouseX - iOldX;
	*i_pDY = m_pParent->hGetWindowHandle()->MouseY - iOldY;

	// Store current values for later comparison
	iOldX = m_pParent->hGetWindowHandle()->MouseX;
	iOldY = m_pParent->hGetWindowHandle()->MouseY;
}

int CInputAmigaOS4::iGetWheelMovement()
{
	return 0; //TODO
}

#endif
