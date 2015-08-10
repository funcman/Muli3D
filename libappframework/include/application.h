
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "base.h"
#include "../../libmuli3d/include/m3d.h"

struct tCreationFlags
{
	string	sWindowTitle;

#ifdef WIN32
	HICON	hIcon;
#endif

	uint16	iWindowWidth, iWindowHeight;
	bool	bWindowed;
};

class IApplication
{
public:
	IApplication();
	virtual ~IApplication();

	virtual bool bInitialize( const tCreationFlags &i_creationFlags ) = 0;
	virtual int	iRun() = 0;

	virtual bool bCreateWorld() = 0;
	virtual void DestroyWorld() = 0;

	virtual void FrameMove() = 0;		// update animated objects, etc.
	virtual void RenderWorld() = 0;		// draw objects

protected:
	bool bCreateSubSystems( const tCreationFlags &i_creationFlags );

public:
	inline float32 fGetFPS() { return m_fFPS; }
	inline float32 fGetInvFPS() { return m_fInvFPS; }
	inline float32 fGetElapsedTime() { return m_fElapsedTime; }

	windowhandle hGetWindowHandle() { return m_hWindowHandle; }
	bool bGetWindowed() { return m_bWindowed; }
	bool bGetActive() { return m_bActive; }
	string GetWindowTitle() { return m_strWindowTitle; }
	uint16 iGetWindowWidth() { return m_iWindowWidth; }
	uint16 iGetWindowHeight() { return m_iWindowHeight; }

	uint32 iGetFrameIdent() { return m_iFrameIdent; }

	inline void SetAppData( void *i_pData, uint32 i_iLength )
	{
		SAFE_DELETE_ARRAY( m_pAppData );
		m_pAppData = new byte[i_iLength];
		memcpy( m_pAppData, i_pData, i_iLength );
	}
	inline void *pGetAppData() { return m_pAppData; }

	// Subsystems -------------------------------------------------------------
	inline class IInput			*pGetInput() { return m_pInput; }
	inline class CFileIO		*pGetFileIO() { return m_pFileIO; }
	inline class CGraphics		*pGetGraphics() { return m_pGraphics; }
	inline class CResManager	*pGetResManager() { return m_pResManager; }
	inline class CScene			*pGetScene() { return m_pScene; }

protected:
	float32		m_fFPS, m_fInvFPS, m_fElapsedTime;

	windowhandle	m_hWindowHandle;
    bool			m_bWindowed, m_bActive;
    string			m_strWindowTitle;
    uint16			m_iWindowWidth, m_iWindowHeight;

	uint32	m_iFrameIdent;

	byte	*m_pAppData;

	// Subsystems -------------------------------------------------------------
	class IInput		*m_pInput;
	class CFileIO		*m_pFileIO;
	class CGraphics		*m_pGraphics;
	class CResManager	*m_pResManager;
	class CScene		*m_pScene;
};

// Platform-dependent code ----------------------------------------------------

#ifdef WIN32

class CApplication : public IApplication
{
public:
	CApplication();
	virtual ~CApplication();

	bool	bInitialize( const tCreationFlags &i_creationFlags );
	int		iRun();

	virtual bool bCreateWorld() = 0;
	virtual void DestroyWorld() = 0;

	virtual void FrameMove() = 0;		// update animated objects, etc.
	virtual void RenderWorld() = 0;		// draw objects

private:
	void BeginFrame();
	bool bCheckMessages();
	void EndFrame();

	static LRESULT CALLBACK WindowProcedure( HWND i_hWnd, UINT i_uMsg, WPARAM i_wParam, LPARAM i_lParam );
	LRESULT MessageProcedure( HWND i_hWnd, UINT i_uMsg, WPARAM i_wParam, LPARAM i_lParam );

public:

private:
	LARGE_INTEGER	m_iTicksPerSecond, m_iStartTime, m_iLastTime;
};

#endif

// ----------------------------------------------------------------------------

#ifdef LINUX_X11

#include <X11/Xatom.h>

#define _BSD_TIME
#include <sys/time.h>

class CApplication : public IApplication
{
public:
	CApplication();
	virtual ~CApplication();

	bool	bInitialize( const tCreationFlags &i_creationFlags );
	int		iRun();

	virtual bool bCreateWorld() = 0;
	virtual void DestroyWorld() = 0;

	virtual void FrameMove() = 0;		// update animated objects, etc.
	virtual void RenderWorld() = 0;		// draw objects

private:
	void BeginFrame();
	bool bCheckMessages();
	void EndFrame();

public:
	inline const Display *pGetDisplay() { return m_pDisplay; }

private:
	Display *m_pDisplay;
	Atom	m_atomCloseWindow;

	bool		m_bGotStartTime;
	struct timeval	m_StartTime, m_LastTime;
	struct timezone	m_TimeZone;
};

#endif

// ----------------------------------------------------------------------------

#ifdef __amigaos4__

//?#define _BSD_TIME
#include <sys/time.h>
#include <intuition/screens.h>
class CApplication : public IApplication
{
public:
	CApplication();
	virtual ~CApplication();

	bool	bInitialize( const tCreationFlags &i_creationFlags );
	int		iRun();

	virtual bool bCreateWorld() = 0;
	virtual void DestroyWorld() = 0;

	virtual void FrameMove() = 0;		// update animated objects, etc.
	virtual void RenderWorld() = 0;		// draw objects

private:
	void BeginFrame();
	bool bCheckMessages();
	void EndFrame();

public:

private:
	// Pointer to screen where window is opened
	struct Screen	*m_pScreen;

	bool			m_bGotStartTime;
	struct timeval	m_StartTime, m_LastTime;
	struct timezone	m_TimeZone;
};

#endif

#endif // __APPLICATION_H__
