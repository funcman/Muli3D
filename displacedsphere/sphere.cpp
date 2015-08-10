
#include "sphere.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CSphereVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		vector4 vTexture; // look up height from texture
		SampleTexture( vTexture, 0, i_pInput[1].x, i_pInput[1].y );
		const float32 fHeight = 0.1f * vTexture.a; // height stored in alpha channel

		// transform position (offset along normal)
		vector3 vNormal = i_pInput[0]; vNormal.normalize(); // renormalize normal - length changed due to interpolation of vertices during subdivision
		o_vPosition = (i_pInput[0] + vNormal * fHeight) * matGetMatrix( m3dsc_wvpmatrix );

		// pass texcoord to pixel shader
		o_pOutput[0] = i_pInput[1];
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector2;
		default: return m3dsrt_unused;
		}
	}
};

class CSpherePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		SampleTexture( io_vColor, 0, i_pInput[0].x, i_pInput[0].y, 0.0f );
		return true;
	}
};

class CSpherePrimitiveAssembler : public IMuli3DPrimitiveAssembler
{
	m3dprimitivetype Execute( std::vector<uint32> &o_VertexIndices, uint32 i_iNumVertices )
	{
		uint32 iCurVertex = 0;
		while( iCurVertex < i_iNumVertices )
		{
			o_VertexIndices.push_back( iCurVertex );
			o_VertexIndices.push_back( iCurVertex + 1 );
			o_VertexIndices.push_back( iCurVertex + 2 );

			o_VertexIndices.push_back( iCurVertex + 1 );
			o_VertexIndices.push_back( iCurVertex + 3 );
			o_VertexIndices.push_back( iCurVertex + 2 );

			iCurVertex += 4;
		}

		return m3dpt_trianglelist;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 1 )
};

CSphere::CSphere( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pPrimitiveAssembler = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_iNumVertices = 0;
	m_iNumPrimitives = 0;

	m_hTexture = 0;
}

CSphere::~CSphere()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hTexture );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pPrimitiveAssembler );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CSphere::bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices, string i_sTexture )
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

	vertexformat *pDestVertices = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDestVertices ) ) )
		return false;

	const float32 fStepV = 1.0f / (float32)i_iStacks;
	const float32 fStepU = 1.0f / (float32)i_iSlices;

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
			pDestVertices->vTex = vector2( fU, fV );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[1], y[1], z[1] );
			pDestVertices->vTex = vector2( fNextU, fV );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[2], y[2], z[2] );
			pDestVertices->vTex = vector2( fU, fNextV );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[3], y[3], z[3] );
			pDestVertices->vTex = vector2( fNextU, fNextV );
			pDestVertices++;
		}
	}

	m_pPrimitiveAssembler = new CSpherePrimitiveAssembler;

	m_pVertexShader = new CSphereVS;
	m_pPixelShader = new CSpherePS;

	// Load environment texture ...
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hTexture = pResManager->hLoadResource( i_sTexture );
	if( !m_hTexture )
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
	case ePass_Default: break;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matWorld; matMatrix44Identity( matWorld );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShader->SetMatrix( m3dsc_worldmatrix, pCurCamera->matGetWorldMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_viewmatrix, pCurCamera->matGetViewMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_projectionmatrix, pCurCamera->matGetProjectionMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hTexture );
	pGraphics->SetTexture( 0, pTexture->pGetTexture() );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetPrimitiveAssembler( m_pPrimitiveAssembler );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->pGetM3DDevice()->DrawDynamicPrimitive( 0, m_iNumVertices );
}
