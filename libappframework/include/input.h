
#ifndef __INPUT_H__
#define __INPUT_H__

#include "base.h"

class IInput
{
protected:
	friend class IApplication;
	friend class CApplication;
	inline IInput( class IApplication *i_pParent ) { m_pParent = i_pParent; }
	virtual ~IInput() {};

	virtual bool bInitialize() = 0;

public:
	virtual void Update() = 0;

protected:

public:
	inline class IApplication *pGetParent() { return m_pParent; }

	virtual bool bKeyDown( char i_cKey ) = 0;
	virtual bool bKeyUp( char i_cKey ) = 0;

	virtual bool bButtonDown( int i_iButton ) = 0;
	virtual bool bButtonUp( int i_iButton ) = 0;

	virtual void GetMovement( int32 *i_pDX, int32 *i_pDY ) = 0;
	virtual int iGetWheelMovement() = 0;

protected:
	class IApplication *m_pParent;
};

// Platform-dependent code ----------------------------------------------------

#ifdef WIN32

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif  // DIRECTINPUT_VERSION

#include <dinput.h>
#pragma comment( lib, "dinput8.lib" )
#pragma comment( lib, "dxguid.lib" )

class CInputWin32 : public IInput
{
protected:
	friend class IApplication;
	friend class CApplication;
	CInputWin32( class IApplication *i_pParent );
	~CInputWin32();

	bool bInitialize();

public:
	void Update();

private:
	void AcquireDevices();
	void UnacquireDevices();

public:
	inline bool bKeyDown( char i_cKey ) { return ( m_cKeys[i_cKey] & 0x80 ) ? true : false; }
	inline bool bKeyUp( char i_cKey ) { return ( m_cKeys[i_cKey] & 0x80 ) ? false : true; }

	inline bool bButtonDown( int i_iButton ) { return ( m_MouseState.rgbButtons[i_iButton] & 0x80 ) ? true : false; }
	inline bool bButtonUp( int i_iButton ) { return ( m_MouseState.rgbButtons[i_iButton] & 0x80 ) ? false : true; }

	inline void GetMovement( int32 *i_pDX, int32 *i_pDY ) { *i_pDX = m_MouseState.lX; *i_pDY = m_MouseState.lY; }
	inline int iGetWheelMovement() { return m_MouseState.lZ; }

private:
	LPDIRECTINPUT8			m_pDirectInput;
	LPDIRECTINPUTDEVICE8	m_pDIDeviceKeyboard, m_pDIDeviceMouse;

	char			m_cKeys[256];
	DIMOUSESTATE	m_MouseState;
};

#endif

// ----------------------------------------------------------------------------

#ifdef LINUX_X11

class CInputLinuxX11 : public IInput
{
protected:
	friend class IApplication;
	friend class CApplication;
	CInputLinuxX11( class IApplication *i_pParent );
	~CInputLinuxX11();

	bool bInitialize();

public:
	void Update();

private:

public:
	bool bKeyDown( char i_cKey );
	bool bKeyUp( char i_cKey );

	bool bButtonDown( int i_iButton );
	bool bButtonUp( int i_iButton );

	void GetMovement( int32 *i_pDX, int32 *i_pDY );
	int iGetWheelMovement();

private:
};

#endif

// ----------------------------------------------------------------------------

#ifdef __amigaos4__

class CInputAmigaOS4 : public IInput
{
protected:
	friend class IApplication;
	friend class CApplication;
	CInputAmigaOS4( class IApplication *i_pParent );
	~CInputAmigaOS4();

	bool bInitialize();

public:
	void Update();

private:

public:
	bool bKeyDown( char i_cKey );
	bool bKeyUp( char i_cKey );

	bool bButtonDown( int i_iButton );
	bool bButtonUp( int i_iButton );

	void GetMovement( int32 *i_pDX, int32 *i_pDY );
	int iGetWheelMovement();

private:
	bool m_bLMB; ///< State of the left mouse button.
	bool m_bRMB; ///< State of the right mouse button.
};

#endif

#endif // __INPUT_H__
