
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
#	include <proto/intuition.h>
#endif

#include "mycamera.h"
#include "sphericallight.h"
#include "leaf.h"

bool CApp::bCreateWorld()
{
	m_pCamera = 0;
	m_hSphericalLight = 0;
	m_hLeaf = 0;

	// Create and setup camera ------------------------------------------------
	m_pCamera = new CMyCamera( pGetGraphics() );
	if( !m_pCamera->bCreateRenderCamera( iGetWindowWidth(), iGetWindowHeight() ) )
		return false;

	m_pCamera->CalculateProjection( M3D_PI * 0.5f, 10.0f, 0.1f );

	m_pCamera->SetPosition( vector3( 0, 0, -4 ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();

	pGetScene()->SetClearColor( vector4( 0, 0, 0.27f, 0 ) );

	// Register sphere-entity and create an instance --------------------------
	pGetScene()->RegisterEntityType( "sphericallight", CSphericalLight::pCreate );
	m_hSphericalLight = pGetScene()->hCreateEntity( "sphericallight" );
	if( !m_hSphericalLight )
		return false;

	CSphericalLight *pSphericalLight = (CSphericalLight *)pGetScene()->pGetEntity( m_hSphericalLight );
	if( !pSphericalLight->bInitialize( 0.15f, 4, 4, 4.0f, 4.0f ) )
		return false;

	// Register leaf-entity and create an instance --------------------------
	pGetScene()->RegisterEntityType( "leaf", CLeaf::pCreate );
	m_hLeaf = pGetScene()->hCreateEntity( "leaf" );
	if( !m_hLeaf )
		return false;

	CLeaf *pLeaf = (CLeaf *)pGetScene()->pGetEntity( m_hLeaf );
	if( !pLeaf->bInitialize( 3.7f, 3.7f ) )
		return false;

	return true;
}

void CApp::DestroyWorld()
{
	pGetScene()->ReleaseEntity( m_hLeaf );
	pGetScene()->ReleaseEntity( m_hSphericalLight );
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
		sprintf( szCaption, "LightFlare, FPS: %3.1f", fGetFPS() );
		#ifdef WIN32
		SetWindowText( hGetWindowHandle(), szCaption );
		#endif
		#ifdef LINUX_X11
		XStoreName( (Display *)pGetDisplay(), hGetWindowHandle(), szCaption );
		#endif
		#ifdef __amigaos4__
		IIntuition->SetWindowTitles( hGetWindowHandle(), szCaption, szCaption);
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
