
#include "app.h"
#include "../libappframework/include/input.h"
#include "../libappframework/include/graphics.h"

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
#include "raytracer.h"

bool CApp::bCreateWorld()
{
	m_pCamera = 0;
	m_hRaytracer = 0;
	m_hEarth = 0; m_hMoon = 0;

	// Create and setup camera ------------------------------------------------
	m_pCamera = new CMyCamera( pGetGraphics() );
	if( !m_pCamera->bCreateRenderCamera( iGetWindowWidth(), iGetWindowHeight(), m3dfmt_r32g32b32f, false ) ) // no depthbuffer is necessary
		return false;

	// m_pCamera->CalculateProjection( M3D_PI * 0.5f, 10.0f, 1 );

	m_pCamera->SetPosition( vector3( 0, 0, -1 ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();

	// Register board-entity and create an instance ---------------------------
	pGetScene()->RegisterEntityType( "raytracer", CRaytracer::pCreate );
	m_hRaytracer = pGetScene()->hCreateEntity( "raytracer" );
	if( !m_hRaytracer )
		return false;

	CRaytracer *pRaytracer = (CRaytracer *)pGetScene()->pGetEntity( m_hRaytracer );
	if( !pRaytracer->bInitialize( M3D_PI * 0.5f ) )
		return false;

	m_hEarth = pGetResManager()->hLoadResource( "earth.png" );
	if( !m_hEarth )
		return false;

	m_hMoon = pGetResManager()->hLoadResource( "moon.png" );
	if( !m_hMoon )
		return false;

	pRaytracer->bAddSphere( vector3( 0, 0, 0 ), 0.25f, vector4( 0, 0, 1, 1 ), m_hEarth );
	pRaytracer->bAddSphere( vector3( -0.05f, 0, -0.7f ), 0.05f, vector4( 1, 1, 1, 1 ), m_hMoon );
	pRaytracer->bAddSphere( vector3( 0, 0.5f, -0.15f ), 0.075f, vector4( 1, 1, 0, 1 ) );
	pRaytracer->bAddLight( vector3( 2, 0, 1 ), vector4( 1, 0.95f, 0.9f, 1 ) );
	pRaytracer->bAddLight( vector3( 0, 0, -1 ), vector4( 1, 0.75f, 1, 1 ) );

	return true;
}

void CApp::DestroyWorld()
{
	pGetResManager()->ReleaseResource( m_hMoon );
	pGetResManager()->ReleaseResource( m_hEarth );
	pGetScene()->ReleaseEntity( m_hRaytracer );
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
		sprintf( szCaption, "Raytracer, FPS: %3.1f", fGetFPS() );
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

	if( pGetInput()->bButtonDown( 0 ) )
	{
		// rotate camera around the bubble
		static float32 fRotX = 0.0f, fRotY = 0.0f;

		int32 iDeltaX, iDeltaY;

		pGetInput()->GetMovement( &iDeltaX, &iDeltaY );
		fRotX += -0.2f * iDeltaX * fGetInvFPS();
		fRotY += 0.2f * iDeltaY * fGetInvFPS();

		const float32 fCamPosX = -1.0f * sinf( fRotX );
		const float32 fCamPosY = -0.5f * sinf( fRotY );
		const float32 fCamPosZ = -1.0f * cosf( fRotX );
		m_pCamera->SetPosition( vector3( fCamPosX, fCamPosY, fCamPosZ ) );
		m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
		m_pCamera->CalculateView();
	}
}

void CApp::RenderWorld()
{
	if( m_pCamera )
	{
		m_pCamera->BeginRender();
		// m_pCamera->ClearToSceneColor();
		m_pCamera->RenderPass( -1 );
		m_pCamera->EndRender( true );
	}
}
