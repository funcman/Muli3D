
#include "app.h"
#include "../libappframework/include/input.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/resmanager.h"

#ifdef WIN32
#ifdef _DEBUG
	#pragma comment( lib, "../libappframework/lib/libappframeworkd.lib" )
#else
	#pragma comment( lib, "../libappframework/lib/libappframework.lib" )
#endif
#pragma comment( lib, "../libappframework/libpng/libpng.lib" )
#pragma comment( lib, "../libappframework/zlib/zlib.lib" )
#endif

#ifdef __amigaos4__
#include <proto/intuition.h>
#endif

#include "mycamera.h"
#include "texcube.h"

bool CApp::bCreateWorld()
{
	m_pCamera = 0;
	m_hTexCube = 0;

	// Create and setup camera ------------------------------------------------
	m_pCamera = new CMyCamera( pGetGraphics() );
	if( !m_pCamera->bCreateRenderCamera( iGetWindowWidth(), iGetWindowHeight(), m3dfmt_r32g32b32f, false ) )
		return false;

	m_pCamera->CalculateProjection( M3D_PI * 0.5f, 10.0f, 1 );

	m_pCamera->SetPosition( vector3( 0, 0, -2.5f ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();

	// Register board-entity and create an instance ---------------------------
	pGetScene()->RegisterEntityType( "texcube", CTexCube::pCreate );
	m_hTexCube = pGetScene()->hCreateEntity( "texcube" );
	if( !m_hTexCube )
		return false;

	CTexCube *pTexCube = (CTexCube *)pGetScene()->pGetEntity( m_hTexCube );
	if( !pTexCube->bInitialize() )
		return false;

	return true;
}

void CApp::DestroyWorld()
{
	pGetScene()->ReleaseEntity( m_hTexCube );
	SAFE_DELETE( m_pCamera );
}

void CApp::FrameMove()
{
#ifdef WIN32
	if( pGetInput()->bKeyDown( DIK_ESCAPE ) )
		PostQuitMessage( 0 );
#endif

	if( iGetFrameIdent() % 12 == 0 )
	{
		#ifdef __amigaos4__
		static
		#endif
		char szCaption[256];
		sprintf( szCaption, "Volumetric map visualization, FPS: %3.1f", fGetFPS() );
		#ifdef WIN32
		SetWindowText( hGetWindowHandle(), szCaption );
		#endif
		#ifdef LINUX_X11
		XStoreName( (Display *)pGetDisplay(), hGetWindowHandle(), szCaption );
		#endif
		#ifdef __amigaos4__
		IIntuition->SetWindowTitles( hGetWindowHandle(), szCaption, szCaption );
		#endif
	}
}

void CApp::RenderWorld()
{
	if( m_pCamera )
	{
		m_pCamera->BeginRender();
		m_pCamera->ClearToSceneColor();
		m_pCamera->RenderPass( -1 );
		m_pCamera->EndRender( true );
	}
}
