
#include "displacedsphere.h"
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

bool CDisplacedSphere::bCreateWorld()
{
	m_pCamera = 0;
	m_hSphere = 0;

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
	if( !pSphere->bInitialize( 1.0f, 12, 12, "earth.png" ) )
		return false;

	// Enable Muli3D's subdivision stage, which will be the base for displacement mapping
	pGetGraphics()->SetRenderState( m3drs_subdivisionmode, m3dsubdiv_adaptive );
	pGetGraphics()->SetRenderState( m3drs_subdivisionlevels, 1 );

	const float32 fSubdividingMaxScreenArea = 14 * 14; // triangles larger than 14x14 square-pixels will be subdivided.
	pGetGraphics()->SetRenderState( m3drs_subdivisionmaxscreenarea, *(uint32 *)&fSubdividingMaxScreenArea );
	pGetGraphics()->SetRenderState( m3drs_subdivisionmaxinnerlevels, 2 );

	pGetGraphics()->SetRenderState( m3drs_fillmode, m3dfill_wireframe );
	
	return true;
}

void CDisplacedSphere::DestroyWorld()
{
	pGetScene()->ReleaseEntity( m_hSphere );
	SAFE_DELETE( m_pCamera );
}

void CDisplacedSphere::FrameMove()
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
		sprintf( szCaption, "Displacement-mapped sphere, FPS: %3.1f", fGetFPS() );
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
	static float32 fRotX = 0.0f, fRotY = 0.0f;
	
	int32 iDeltaX, iDeltaY;

	pGetInput()->GetMovement( &iDeltaX, &iDeltaY );
	fRotX += -0.1f * iDeltaX * fGetInvFPS();
	fRotY += 0.1f * iDeltaY * fGetInvFPS();

	const float32 fCamPosX = -2.0f * sinf( fRotX );
	const float32 fCamPosY = -2.0f * sinf( fRotY );
	const float32 fCamPosZ = -2.0f * cosf( fRotX );
	m_pCamera->SetPosition( vector3( fCamPosX, fCamPosY, fCamPosZ ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();
}

void CDisplacedSphere::RenderWorld()
{
	if( m_pCamera )
	{
		m_pCamera->BeginRender();
		m_pCamera->ClearToSceneColor();
		m_pCamera->RenderPass( -1 );
		m_pCamera->EndRender( true );
	}
}
