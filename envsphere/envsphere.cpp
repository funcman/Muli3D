
#include "envsphere.h"
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
#include "sphere.h"

bool CEnvSphere::bCreateWorld()
{
	m_pCamera = 0;
	m_hSphere = 0;
	m_hLight = 0;

	// Create and setup camera ------------------------------------------------
	m_pCamera = new CMyCamera( pGetGraphics() );
	if( !m_pCamera->bCreateRenderCamera( iGetWindowWidth(), iGetWindowHeight() ) )
		return false;

	m_pCamera->CalculateProjection( M3D_PI * 0.5f, 10.0f, 0.1f );

	m_pCamera->SetPosition( vector3( 0, 0, -2 ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();

	// Register triangle-entity and create an instance ------------------------
	pGetScene()->RegisterEntityType( "sphere", CSphere::pCreate );
	m_hSphere = pGetScene()->hCreateEntity( "sphere" );
	if( !m_hSphere )
		return false;

	CSphere *pSphere = (CSphere *)pGetScene()->pGetEntity( m_hSphere );
	if( !pSphere->bInitialize( 1.0f, 16, 16, "majestic.cube" ) )
		return false;

	// Create the light -------------------------------------------------------
	m_hLight = pGetScene()->hCreateLight();
	if( !m_hLight )
		return false;

	CLight *pLight = pGetScene()->pGetLight( m_hLight );
	pLight->SetPosition( vector3( 1.5f, 0.25f, 0 ) );
	pLight->SetColor( vector4( 1, 1, 0.75f, 1 ) );

	// Demonstrating smooth-subdivision to get a round sphere
	// disable this and increase sphere-tesselation for better preformance (memory-usage will be higher!)
	pGetGraphics()->SetRenderState( m3drs_subdivisionmode, m3dsubdiv_smooth );
	pGetGraphics()->SetRenderState( m3drs_subdivisionlevels, 1 );
	pGetGraphics()->SetRenderState( m3drs_subdivisionpositionregister, 0 );
	pGetGraphics()->SetRenderState( m3drs_subdivisionnormalregister, 0 );	// using vertex-positions as normals (unit-sphere at origin)

	return true;
}

void CEnvSphere::DestroyWorld()
{
	pGetScene()->ReleaseLight( m_hLight );
	pGetScene()->ReleaseEntity( m_hSphere );
	SAFE_DELETE( m_pCamera );
}

void CEnvSphere::FrameMove()
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
		sprintf( szCaption, "Environment-mapped sphere, FPS: %3.1f", fGetFPS() );
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

	// rotate camera around sphere
	static float32 fRotX = 0.5f, fRotY = 0.0f;
	
	int32 iDeltaX, iDeltaY;

	pGetInput()->GetMovement( &iDeltaX, &iDeltaY );
	fRotX += -0.1f * iDeltaX * fGetInvFPS();
	fRotY += 0.1f * iDeltaY * fGetInvFPS();

	const float32 fCamPosX = -2.0f * sinf( fRotX );
	const float32 fCamPosY = -1.0f * sinf( fRotY );
	const float32 fCamPosZ = -2.0f * cosf( fRotX );
	m_pCamera->SetPosition( vector3( fCamPosX, fCamPosY, fCamPosZ ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();
}

void CEnvSphere::RenderWorld()
{
	if( m_pCamera )
	{
		m_pCamera->BeginRender();
		m_pCamera->ClearToSceneColor();
		m_pCamera->RenderPass( -1 );
		m_pCamera->EndRender( true );
	}
}
