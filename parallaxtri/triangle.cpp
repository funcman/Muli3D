
#include "triangle.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CTriangleVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		// pass texcoord to pixelshader
		o_pOutput[0] = i_pInput[3];

		// build transformation matrix to tangent space
		vector3 vNormal; vVector3TransformNormal( vNormal, i_pInput[1], matGetMatrix( m3dsc_worldmatrix ) );
		vector3 vTangent; vVector3TransformNormal( vTangent, i_pInput[2], matGetMatrix( m3dsc_worldmatrix ) );
		vector3 vBinormal; vVector3Cross( vBinormal, vNormal, vTangent );

		const matrix44 matWorldToTangentSpace(
			vTangent.x, vBinormal.x, vNormal.x, 0.0f,
			vTangent.y, vBinormal.y, vNormal.y, 0.0f,
			vTangent.z, vBinormal.z, vNormal.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f );

		// transform light direction to tangent space
		const vector3 vWorldPosition = i_pInput[0] * matGetMatrix( m3dsc_worldmatrix );
		vector3 vLightDir = (vector3)vGetVector( 1 ) - vWorldPosition;
		vector3 vLightDirTangentSpace; vVector3TransformNormal( vLightDirTangentSpace, vLightDir, matWorldToTangentSpace );
		o_pOutput[1] = vLightDirTangentSpace;

		// transform view direction to tangent space
		vector3 vViewDir = (vector3)vGetVector( 0 ) - vWorldPosition;
		vector3 vViewDirTangentSpace; vVector3TransformNormal( vViewDirTangentSpace, vViewDir, matWorldToTangentSpace );
		o_pOutput[2] = vViewDirTangentSpace;

		// compute half vector and transform to tangent space
		const vector3 vHalf = ( vViewDir.normalize() + vLightDir.normalize() ) * 0.5f;
		vector3 vHalfTangentSpace; vVector3TransformNormal( vHalfTangentSpace, vHalf, matWorldToTangentSpace );
		o_pOutput[3] = vHalfTangentSpace;
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector2;
		case 1: return m3dsrt_vector3;
		case 2: return m3dsrt_vector3;
		case 3: return m3dsrt_vector3;
		default: return m3dsrt_unused;
		}
	}
};

class CTrianglePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		// read normal from normalmap
		vector4 vTexNormal; SampleTexture( vTexNormal, 1, i_pInput[0].x, i_pInput[0].y, 0.0f );
		const vector3 vNormal( vTexNormal.x * 2.0f - 1.0f, vTexNormal.y * 2.0f - 1.0f, vTexNormal.z * 2.0f - 1.0f );

		// parallax mapping
		const vector4 &vViewDir = i_pInput[2];
		const float32 fHeight = vTexNormal.a / vViewDir.z;
		const vector2 vTexCoords( i_pInput[0].x + fHeight * vViewDir.x, i_pInput[0].y + fHeight * vViewDir.y );
		
		// sample texture
		vector4 vTex;
		SampleTexture( vTex, 0, vTexCoords.x, vTexCoords.y, 0.0f );
		
		// renormalize interpolated light direction vector
		vector3 vLightDir = i_pInput[1]; vLightDir.normalize();

		// compute diffuse light
		float32 fDiffuse = fVector3Dot( vNormal, vLightDir );
		float32 fSpecular = 0.0f;
		if( fDiffuse >= 0.0f )
		{			
			// compute specular light
			vector3 vHalf = i_pInput[3]; vHalf.normalize();
			fSpecular = fVector3Dot( vNormal, vHalf );
			if( fSpecular < 0.0f )
				fSpecular = 0.0f;
			else
				fSpecular = powf( fSpecular, 128.0f );
		}
		else
			fDiffuse = 0.0f;

		const vector4 &vLightColor = vGetVector( 0 );
		io_vColor = vTex * vLightColor * fDiffuse + vLightColor * fSpecular; // += for additive blending with backbuffer, e.g. when there are multiple lights

		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 1 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 2 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 3 ),
};

CTriangle::CTriangle( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_hTexture = 0;
	m_hNormalmap = 0;
}

CTriangle::~CTriangle()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hNormalmap );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hTexture );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CTriangle::bInitialize( const vertexformat *i_pVertices, string i_sTexture, string i_sNormalmap )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * 3 ) ) )
		return false;

	vertexformat *pDest = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDest ) ) )
		return false;

	memcpy( pDest, i_pVertices, sizeof( vertexformat ) * 3 );

	// Calculate triangle normal ...
	vector3 v01 = pDest[1].vPosition - pDest[0].vPosition;
	vector3 v02 = pDest[2].vPosition - pDest[0].vPosition;

	// Calculate triangle tangent ...
	float32 fDeltaV[2] = {
		pDest[1].vTexCoord0.y - pDest[0].vTexCoord0.y,
		pDest[2].vTexCoord0.y - pDest[0].vTexCoord0.y };
	vector3 vTangent = (v01 * fDeltaV[1]) - (v02 * fDeltaV[0]);
	pDest[0].vTangent = pDest[1].vTangent = pDest[2].vTangent = vTangent.normalize();

	m_pVertexShader = new CTriangleVS;
	m_pPixelShader = new CTrianglePS;

	// Load texture -----------------------------------------------------------
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hTexture = pResManager->hLoadResource( i_sTexture );
	if( !m_hTexture )
		return false;

	// Load normalmap ---------------------------------------------------------
	m_hNormalmap = pResManager->hLoadResource( i_sNormalmap );
	if( !m_hNormalmap )
		return false;

	return true;
}

bool CTriangle::bFrameMove()
{
	return false;
}

void CTriangle::Render( uint32 i_iPass )
{
	switch( i_iPass )
	{
	case ePass_Lighting: break;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matWorld; matMatrix44Identity( matWorld );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShader->SetMatrix( m3dsc_worldmatrix, pCurCamera->matGetWorldMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_viewmatrix, pCurCamera->matGetViewMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_projectionmatrix, pCurCamera->matGetProjectionMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	vector3 vCamPos = pCurCamera->vGetPosition();
	m_pVertexShader->SetVector( 0, pCurCamera->vGetPosition() );

	CLight *pLight = m_pParent->pGetCurrentLight();
	
	vector3 vLightPos = pLight->vGetPosition();
	m_pVertexShader->SetVector( 1, vector4( vLightPos.x, vLightPos.y, vLightPos.z, 0 ) );
	
	m_pPixelShader->SetVector( 0, pLight->vGetColor() );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hTexture );
	pGraphics->SetTexture( 0, pTexture->pGetTexture() );
	
	CTexture *pNormalmap = (CTexture *)pResManager->pGetResource( m_hNormalmap );
	pGraphics->SetTexture( 1, pNormalmap->pGetTexture() );

	for( uint32 i = 0; i < 2; ++i )
	{
		pGraphics->SetTextureSamplerState( i, m3dtss_addressu, m3dta_clamp );
		pGraphics->SetTextureSamplerState( i, m3dtss_addressv, m3dta_clamp );
	}

	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglelist, 0, 1 );
}
