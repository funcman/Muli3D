
#include "displacedtri.h"
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
#include "triangle.h"

bool CDisplacedTri::bCreateWorld()
{
	m_pCamera = 0;
	m_hTriangle = 0;
	m_hLight = 0;

	// Create and setup camera ------------------------------------------------
	m_pCamera = new CMyCamera( pGetGraphics() );
	if( !m_pCamera->bCreateRenderCamera( iGetWindowWidth(), iGetWindowHeight() ) )
		return false;

	m_pCamera->CalculateProjection( M3D_PI * 0.5f, 10.0f, 0.1f );

	m_pCamera->SetPosition( vector3( 0.15f, -0.2f, -0.8f ) );
	m_pCamera->SetLookAt( vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	m_pCamera->CalculateView();

	// Register triangle-entity and create an instance ------------------------
	pGetScene()->RegisterEntityType( "triangle", CTriangle::pCreate );
	m_hTriangle = pGetScene()->hCreateEntity( "triangle" );
	if( !m_hTriangle )
		return false;

	CTriangle::vertexformat vertices[3];
	vertices[0].vPosition = vector3( -1, -0.5f, 0 );
	vertices[0].vTexCoord0 = vector2( 0, 1 );
	vertices[1].vPosition = vector3( 0, 1, 0.5f );
	vertices[1].vTexCoord0 = vector2( 0.5f, 0 );
	vertices[2].vPosition = vector3( 1, 0, 0.25f );
	vertices[2].vTexCoord0 = vector2( 1, 0 );

	CTriangle *pTriangle = (CTriangle *)pGetScene()->pGetEntity( m_hTriangle );
	if( !pTriangle->bInitialize( vertices, "triangle.png", "triangle_normals.png" ) )
		return false;

	// Create the light -------------------------------------------------------
	m_hLight = pGetScene()->hCreateLight();
	if( !m_hLight )
		return false;

	CLight *pLight = pGetScene()->pGetLight( m_hLight );
	pLight->SetPosition( vector3( 0, 0, -1 ) );
	pLight->SetColor( vector4( 1, 1, 1, 1 ) );

	// Enable linear mip-filtering
	// pGetGraphics()->SetTextureSamplerState( 0, m3dtss_mipfilter, m3dtf_linear );
	// pGetGraphics()->SetTextureSamplerState( 1, m3dtss_mipfilter, m3dtf_linear );

	// Enable Muli3D's subdivision stage, which will be the base for displacement mapping
	pGetGraphics()->SetRenderState( m3drs_subdivisionmode, m3dsubdiv_simple );
	pGetGraphics()->SetRenderState( m3drs_subdivisionlevels, 5 );

	return true;
}

void CDisplacedTri::DestroyWorld()
{
	pGetScene()->ReleaseLight( m_hLight );
	pGetScene()->ReleaseEntity( m_hTriangle );
	SAFE_DELETE( m_pCamera );
}

void CDisplacedTri::FrameMove()
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
		sprintf( szCaption, "Displacement-mapped triangle, FPS: %3.1f", fGetFPS() );
		#ifdef WIN32
		SetWindowText( hGetWindowHandle(), szCaption );
		#endif
		#ifdef LINUX_X11
		XStoreName( (Display *)pGetDisplay(), hGetWindowHandle(), szCaption );
		#endif
		#ifdef __amigaos__
		IIntuition->SetWindowTitles( hGetWindowHandle(), szCaption, szCaption );
		#endif
	}

#ifdef WIN32
	if( m_hLight && GetFocus() )
	{
		POINT pt; GetCursorPos( &pt );
		ScreenToClient( hGetWindowHandle(), &pt );

		vector3 vLightPos;
		vLightPos.x = (float32)pt.x / (float32)iGetWindowWidth() * 2.0f - 1.0f;
		vLightPos.y = (float32)pt.y / (float32)iGetWindowHeight() * 2.0f - 1.0f;
		vLightPos.z = -1.0f;

		CLight *pLight = pGetScene()->pGetLight( m_hLight );
		pLight->SetPosition( vLightPos );
	}
#endif

#ifdef __amigaos4__
	ULONG bActivated = 0;
	struct Window* pWnd = hGetWindowHandle();
	IIntuition->GetWindowAttr( pWnd, WA_Activate, &bActivated, sizeof(ULONG) );
	if ( m_hLight && bActivated )
	{
		vector3 vLightPos;
		vLightPos.x = (float32)pWnd->MouseX / (float32)iGetWindowWidth() * 2.0f - 1.0f;
		vLightPos.y = (float32)pWnd->MouseY / (float32)iGetWindowHeight() * 2.0f - 1.0f;
		vLightPos.z = -1.0f;

		CLight *pLight = pGetScene()->pGetLight( m_hLight );
		pLight->SetPosition( vLightPos );
	}
#endif
}

void CDisplacedTri::RenderWorld()
{
	if( m_pCamera )
	{
		m_pCamera->BeginRender();
		m_pCamera->ClearToSceneColor();
		m_pCamera->RenderPass( -1 );
		m_pCamera->EndRender( true );
	}
}
