
#include "../include/camera.h"
#include "../include/graphics.h"
#include "../include/application.h"
#include "../include/scene.h"

CCamera::CCamera( CGraphics *i_pParent )
{
	m_pParent = i_pParent;

	if( FUNC_FAILED( m_pParent->pGetM3DDevice()->CreateRenderTarget( &m_pRenderTarget ) ) )
		m_pRenderTarget = 0;

	m_bLockedSurfacesViewport = false;

	matMatrix44Identity( m_matWorld );
	matMatrix44Identity( m_matView );
	matMatrix44Identity( m_matProjection );
	BuildFrustum();
}

CCamera::~CCamera()
{
	SAFE_RELEASE( m_pRenderTarget );
}

bool CCamera::bCreateRenderCamera( uint32 i_iWidth, uint32 i_iHeight, m3dformat i_fmtFrameBuffer, bool i_bDepthBuffer )
{
	if( m_bLockedSurfacesViewport )
		return false;

	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Create the render texture ----------------------------------------------
	CMuli3DSurface *pColorBuffer = 0;
	if( FUNC_FAILED( pM3DDevice->CreateSurface( &pColorBuffer, i_iWidth, i_iHeight, i_fmtFrameBuffer ) ) )
		return false;

	// Create the depth texture -----------------------------------------------
	CMuli3DSurface *pDepthBuffer = 0;
	if( i_bDepthBuffer )
	{
		if( FUNC_FAILED( pM3DDevice->CreateSurface( &pDepthBuffer, i_iWidth, i_iHeight, m3dfmt_r32f ) ) )
		{
			SAFE_RELEASE( pColorBuffer );
			return false;
		}
	}

	// Set the viewport -------------------------------------------------------
	matrix44 matViewport;
	matMatrix44Viewport( matViewport, 0, 0, i_iWidth, i_iHeight, 0.0f, 1.0f );
	m_pRenderTarget->SetViewportMatrix( matViewport );
	
	m_pRenderTarget->SetColorBuffer( pColorBuffer );
	m_pRenderTarget->SetDepthBuffer( pDepthBuffer );

	SAFE_RELEASE( pDepthBuffer );
	SAFE_RELEASE( pColorBuffer );

	m_bLockedSurfacesViewport = true;	// Don't allow any more changes to surfaces/viewport!

	return true;
}

void CCamera::BeginRender()
{
	m_pParent->PushStateBlock();

	m_pParent->SetRenderTarget( m_pRenderTarget );
	m_pParent->SetCurCamera( this );
}

void CCamera::ClearToSceneColor( const m3drect *i_pRect )
{
	CScene *pScene = m_pParent->pGetParent()->pGetScene();
	m_pRenderTarget->ClearColorBuffer( pScene->vGetClearColor(), i_pRect );
	m_pRenderTarget->ClearDepthBuffer( 1.0f, i_pRect );
}

void CCamera::EndRender( bool i_bPresentToScreen )
{
	m_pParent->PopStateBlock();

	if( i_bPresentToScreen )
		m_pParent->pGetM3DDevice()->Present( m_pRenderTarget );
}

void CCamera::CalculateProjection( float32 i_fFOVAngle, float32 i_fViewDistance, float32 i_fNearClippingPlane, float32 i_fAspect )
{
	matMatrix44PerspectiveFovLH( m_matProjection, i_fFOVAngle, i_fAspect, i_fNearClippingPlane, i_fViewDistance );

	m_fFOVAngle = i_fFOVAngle;
	m_fAspect = i_fAspect;
	m_fNearClippingPlane = i_fNearClippingPlane;
	m_fViewDistance = i_fViewDistance;
}

void CCamera::CalculateView()
{
	matrix44 matRotation, matTranslation;	
	matMatrix44Translation( matTranslation, -m_vPosition );
	matMatrix44RotationQuaternion( matRotation, m_qOrientation );
	m_matView = matTranslation * matRotation;

	// Update camera axis -----------------------------------------------------
	matMatrix44Transpose( matRotation, matRotation );

	m_vDir.x = matRotation._31;
	m_vDir.y = matRotation._32;
	m_vDir.z = matRotation._33;

	m_vUp.x = matRotation._21;
	m_vUp.y = matRotation._22;
	m_vUp.z = matRotation._23;

	m_vRight.x = matRotation._11;
	m_vRight.y = matRotation._12;
	m_vRight.z = matRotation._13;

	// Re-Build Frustum -------------------------------------------------------
	BuildFrustum();
}

eVisibility CCamera::eSphereVisible( const vector3 &i_vOrigin, float32 i_fRadius )
{
	vector3 vOrigin = i_vOrigin * matGetWorldMatrix();

	vector3 vRadius( i_fRadius, 0, 0 ); // accounts for scaling matrix! TODO: only enables scales along x-axis
	vVector3TransformNormal( vRadius, vRadius, matGetWorldMatrix() ); 
	i_fRadius = vRadius.length();

	plane *pFrustum = m_plFrustum;
	for( uint32 i = 0; i < 6; ++i, ++pFrustum )
	{
		if( *pFrustum * vOrigin < -i_fRadius )
			return eVisibility_CompletelyOut;
	}
	return eVisibility_CompletelyIn;
}

eVisibility CCamera::eBoxVisible( const vector3 &i_vLower, const vector3 &i_vUpper )
{
	vector3 vVectorA( i_vLower.x, i_vLower.y, i_vLower.z );
	vVectorA *= matGetWorldMatrix();
	vector3 vVectorB( i_vUpper.x, i_vLower.y, i_vLower.z );
	vVectorB *= matGetWorldMatrix();
	vector3 vVectorC( i_vLower.x, i_vUpper.y, i_vLower.z );
	vVectorC *= matGetWorldMatrix();
	vector3 vVectorD( i_vUpper.x, i_vUpper.y, i_vLower.z );
	vVectorD *= matGetWorldMatrix();
	vector3 vVectorE( i_vLower.x, i_vLower.y, i_vUpper.z );
	vVectorE *= matGetWorldMatrix();
	vector3 vVectorF( i_vUpper.x, i_vLower.y, i_vUpper.z );
	vVectorF *= matGetWorldMatrix();
	vector3 vVectorG( i_vLower.x, i_vUpper.y, i_vUpper.z );
	vVectorG *= matGetWorldMatrix();
	vector3 vVectorH( i_vUpper.x, i_vUpper.y, i_vUpper.z );
	vVectorH *= matGetWorldMatrix();

	#define V_IN 1
	#define V_OUT 2
	#define V_INTERSECT 3
	byte cMode = 0;

	plane *pFrustum = m_plFrustum;
	for( uint32 i = 0; i < 6; ++i, ++pFrustum )
	{
		cMode &= V_OUT; // clear IN-bit
		
		if( *pFrustum * vVectorA >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorB >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorC >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorD >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorE >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorF >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorG >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( *pFrustum * vVectorH >= 0 ) cMode |= V_IN; else cMode |= V_OUT;
		if( cMode == V_INTERSECT ) continue;

		if( cMode == V_IN )
			continue;

		return eVisibility_CompletelyOut;
	}

	if( cMode == V_INTERSECT )
		return eVisibility_Partly;
	else
		return eVisibility_CompletelyIn;
}

void CCamera::BuildFrustum()
{
	// Calculate frustum-planes -----------------------------------------------
	matrix44 matFrustum = matGetViewMatrix() * matGetProjectionMatrix();
	
	// Near
	m_plFrustum[0] = plane( matFrustum._14 + matFrustum._13, 
		matFrustum._24 + matFrustum._23, 
		matFrustum._34 + matFrustum._33, 
		matFrustum._44 + matFrustum._43 );

	// Far
	m_plFrustum[1] = plane( matFrustum._14 - matFrustum._13,
		matFrustum._24 - matFrustum._23,
		matFrustum._34 - matFrustum._33,
		matFrustum._44 - matFrustum._43 );

	// Left
	m_plFrustum[2] = plane( matFrustum._14 + matFrustum._11,
		matFrustum._24 + matFrustum._21,
		matFrustum._34 + matFrustum._31,
		matFrustum._44 + matFrustum._41 );

	// Right
	m_plFrustum[3] = plane( matFrustum._14 - matFrustum._11,
		matFrustum._24 - matFrustum._21,
		matFrustum._34 - matFrustum._31,
		matFrustum._44 - matFrustum._41 );

	// Top
	m_plFrustum[4] = plane( matFrustum._14 - matFrustum._12,
		matFrustum._24 - matFrustum._22,
		matFrustum._34 - matFrustum._32,
		matFrustum._44 - matFrustum._42 );

	// Bottom
	m_plFrustum[5] = plane( matFrustum._14 + matFrustum._12,
		matFrustum._24 + matFrustum._22,
		matFrustum._34 + matFrustum._32,
		matFrustum._44 + matFrustum._42 );

	m_plFrustum[0].normal.normalize();
	m_plFrustum[1].normal.normalize();
	m_plFrustum[2].normal.normalize();
	m_plFrustum[3].normal.normalize();
	m_plFrustum[4].normal.normalize();
	m_plFrustum[5].normal.normalize();
}
