
#include "sphere.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CSphereVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		// transform normal (normal = position when sphere's origin = (0,0,0))
		vVector3TransformNormal( *(vector3 *)&o_pOutput[0], i_pInput[0], matGetMatrix( m3dsc_worldmatrix ) );

		// calculate per pixel light direction
		const vector3 vWorldPosition = i_pInput[0] * matGetMatrix( m3dsc_worldmatrix );
		vector3 vLightDir = (vector3)vGetVector( 1 ) - vWorldPosition;
		o_pOutput[1] = vLightDir;

		// calculate per pixel light view direction
		vector3 vViewDir = (vector3)vGetVector( 0 ) - vWorldPosition;
		o_pOutput[2] = vViewDir;
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector3;
		case 1: return m3dsrt_vector3;
		case 2: return m3dsrt_vector3;
		default: return m3dsrt_unused;
		}
	}
};

class CSpherePS : public IMuli3DPixelShader
{
public:
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		vector3 vNormal = i_pInput[0]; vNormal.normalize();
		vector3 vLightDir = i_pInput[1]; vLightDir.normalize();

		// compute fresnel term and reflection vector
		const vector3 vViewDir = i_pInput[2];
		const float32 fViewDotNormal = fSaturate( fVector3Dot( vNormal, vViewDir ) );
		vector3 vReflection = vNormal * (2.0f * fViewDotNormal) - vViewDir;
		vReflection.normalize();
		const float32 fFresnel = (1.0f - fViewDotNormal);// * (1.0f - fViewDotNormal);

		// compute diffuse and specular light
		float32 fDiffuse = fVector3Dot( vNormal, vLightDir );
		float32 fSpecular = 0.0f;
		if( fDiffuse >= 0.0f )
		{
			fSpecular = fVector3Dot( vLightDir, vReflection );
			if( fSpecular >= 0.0f )
				fSpecular = powf( fSpecular, 128.0f );
			else
				fSpecular = 0.0f;
		}
		else
			fDiffuse = 0.0f;

		vector4 vReflectionEnv;
		SampleTexture( vReflectionEnv, 0, vReflection.x, vReflection.y, vReflection.z );

		vVector4Lerp( io_vColor, io_vColor, vReflectionEnv, 1.0f - fFresnel ); // use inverse fresnel

		const vector4 &vLightColor = vGetVector( 1 );
		io_vColor += vGetVector( 0 ) * vLightColor * fDiffuse + vLightColor * fSpecular;

		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 )
};

CSphere::CSphere( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pIndexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;

	SetColor( vector4( 0.5f, 0.5f, 0.5f, 1 ) );

	m_iNumVertices = 0;
	m_iNumPrimitives = 0;

	m_hEnvironment = 0;
}

CSphere::~CSphere()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hEnvironment );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pIndexBuffer );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CSphere::bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices, string i_sEnvironment )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	// Construct a sphere
	m_iNumVertices = i_iStacks * i_iSlices * 4;
	m_iNumPrimitives = i_iStacks * i_iSlices * 2;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * m_iNumVertices ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateIndexBuffer( &m_pIndexBuffer, sizeof( uint16 ) * m_iNumPrimitives * 3, m3dfmt_index16 ) ) )
		return false;

	vertexformat *pDestVertices = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDestVertices ) ) )
		return false;

	uint16 *pDestIndices = 0;
	if( FUNC_FAILED( m_pIndexBuffer->GetPointer( 0, (void **)&pDestIndices ) ) )
		return false;

	const float32 fStepV = 1.0f / (float32)i_iStacks;
	const float32 fStepU = 1.0f / (float32)i_iSlices;

	uint32 iCurVertex = 0;

	float32 fV = 0.0f;
	for( uint32 i = 0; i < i_iStacks; ++i, fV += fStepV )
	{
		float32 fNextV = fV + fStepV;

		float32 fU = 0.0f;
		for( uint32 j = 0; j < i_iSlices; ++j, fU += fStepU )
		{
			float32 fNextU = fU + fStepU;
			
			// create a quad
			// 0 -- 1
			// |    |
			// 2 -- 3

			float32 x[4], y[4], z[4];

			x[0] = i_fRadius * sinf( fV * M3D_PI ) * cosf( fU * 2.0f * M3D_PI );
			z[0] = i_fRadius * sinf( fV * M3D_PI ) * sinf( fU * 2.0f * M3D_PI );
			y[0] = i_fRadius * cosf( fV * M3D_PI );

			x[1] = i_fRadius * sinf( fV * M3D_PI ) * cosf( fNextU * 2.0f * M3D_PI );
			z[1] = i_fRadius * sinf( fV * M3D_PI ) * sinf( fNextU * 2.0f * M3D_PI );
			y[1] = i_fRadius * cosf( fV * M3D_PI );

			x[2] = i_fRadius * sinf( fNextV * M3D_PI ) * cosf( fU * 2.0f * M3D_PI );
			z[2] = i_fRadius * sinf( fNextV * M3D_PI ) * sinf( fU * 2.0f * M3D_PI );
			y[2] = i_fRadius * cosf( fNextV * M3D_PI );

			x[3] = i_fRadius * sinf( fNextV * M3D_PI ) * cosf( fNextU * 2.0f * M3D_PI );
			z[3] = i_fRadius * sinf( fNextV * M3D_PI ) * sinf( fNextU * 2.0f * M3D_PI );
			y[3] = i_fRadius * cosf( fNextV * M3D_PI );

			pDestVertices->vPosition = vector3( x[0], y[0], z[0] );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[1], y[1], z[1] );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[2], y[2], z[2] );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[3], y[3], z[3] );
			pDestVertices++;

			*pDestIndices++ = iCurVertex;
			*pDestIndices++ = iCurVertex + 1;
			*pDestIndices++ = iCurVertex + 2;

			*pDestIndices++ = iCurVertex + 1;
			*pDestIndices++ = iCurVertex + 3;
			*pDestIndices++ = iCurVertex + 2;

			iCurVertex += 4;
		}
	}

	m_pVertexShader = new CSphereVS;
	m_pPixelShader = new CSpherePS;

	// Load environment texture ...
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hEnvironment = pResManager->hLoadResource( i_sEnvironment );
	if( !m_hEnvironment )
		return false;

	// Make sure it is a cube map!
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hEnvironment );
	if( pTexture->eGetTextureType() != eTextureType_Cube )
		return false;

	return true;
}

bool CSphere::bFrameMove()
{
	return false;
}

void CSphere::Render( uint32 i_iPass )
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

	m_pPixelShader->SetVector( 0, m_vColor );

	vector3 vCamPos = pCurCamera->vGetPosition();
	m_pVertexShader->SetVector( 0, vector4( vCamPos.x, vCamPos.y, vCamPos.z, 0 ) );

	CLight *pLight = m_pParent->pGetCurrentLight();
	
	vector3 vLightPos = pLight->vGetPosition();
	m_pVertexShader->SetVector( 1, vector4( vLightPos.x, vLightPos.y, vLightPos.z, 0 ) );
	
	m_pPixelShader->SetVector( 1, pLight->vGetColor() );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hEnvironment );
	pGraphics->SetTexture( 0, pTexture->pGetTexture() );
	
	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetIndexBuffer( m_pIndexBuffer );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist,
		0, 0, m_iNumVertices, 0, m_iNumPrimitives );
}
