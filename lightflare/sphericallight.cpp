
#include "sphericallight.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CSphericalLightVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		if( fGetFloat( 0 ) )
		{
			// pass texcoord to pixelshader for flare rendering
			o_pOutput[0] = i_pInput[1];
		}
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector3;
		case 1: if( fGetFloat( 0 ) ) return m3dsrt_vector2;
		default: return m3dsrt_unused;
		}
	}
};

class CSphericalLightPS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return true; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		if( fGetFloat( 0 ) )
		{
			// render flare

			vector4 vFlareColor;
			SampleTexture( vFlareColor, 0, i_pInput[0].x, i_pInput[0].y, 0.0f );
			if( vFlareColor.a <= FLT_EPSILON ) // perform alpha test
				return false;

			io_vColor += vFlareColor * fGetFloat( 1 ); // scale and blend with backbuffer
		}
		else
			io_vColor = vGetVector( 0 );
			
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

static m3dvertexelement VertexDeclarationSphere[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 )
};

static m3dvertexelement VertexDeclarationFlare[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 1 )
};

CSphericalLight::CSphericalLight( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormatSphere = 0;
	m_pVertexBufferSphere = 0;
	m_pPrimitiveAssemblerSphere = 0;

	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_pVertexFormatFlare = 0;
	m_pVertexBufferFlare = 0;

	SetColor( vector4( 1.0f, 1.0f, 1.0f, 1 ) );

	m_iNumVertices = 0;
	m_iNumPrimitives = 0;

	m_hFlare = 0;

	m_iMaxVisiblePixels = 0;
}

CSphericalLight::~CSphericalLight()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hFlare );

	SAFE_RELEASE( m_pVertexBufferFlare );
	SAFE_RELEASE( m_pVertexFormatFlare );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );

	SAFE_RELEASE( m_pPrimitiveAssemblerSphere );
	SAFE_RELEASE( m_pVertexBufferSphere );
	SAFE_RELEASE( m_pVertexFormatSphere );
}

bool CSphericalLight::bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices, float32 i_fFlareWidth, float32 i_fFlareHeight )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormatSphere, VertexDeclarationSphere, sizeof( VertexDeclarationSphere ) ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormatFlare, VertexDeclarationFlare, sizeof( VertexDeclarationFlare ) ) ) )
		return false;

	// Construct a sphere
	m_iNumVertices = i_iStacks * i_iSlices * 4;
	m_iNumPrimitives = i_iStacks * i_iSlices * 2;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBufferSphere, sizeof( vertexformatsphere ) * m_iNumVertices ) ) )
		return false;

	vertexformatsphere *pDestVerticesSphere = 0;
	if( FUNC_FAILED( m_pVertexBufferSphere->GetPointer( 0, (void **)&pDestVerticesSphere ) ) )
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

			pDestVerticesSphere->vPosition = vector3( x[0], y[0], z[0] );
			pDestVerticesSphere++;

			pDestVerticesSphere->vPosition = vector3( x[1], y[1], z[1] );
			pDestVerticesSphere++;

			pDestVerticesSphere->vPosition = vector3( x[2], y[2], z[2] );
			pDestVerticesSphere++;

			pDestVerticesSphere->vPosition = vector3( x[3], y[3], z[3] );
			pDestVerticesSphere++;
		}
	}

	m_pPrimitiveAssemblerSphere = new CSpherePrimitiveAssembler;

	m_pVertexShader = new CSphericalLightVS;
	m_pPixelShader = new CSphericalLightPS;

	// Initialize data for the flare
	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBufferFlare, sizeof( vertexformatflare ) * 4 ) ) )
		return false;

	vertexformatflare *pDestVerticesFlare = 0;
	if( FUNC_FAILED( m_pVertexBufferFlare->GetPointer( 0, (void **)&pDestVerticesFlare ) ) )
		return false;

	pDestVerticesFlare[0].vPosition = vector3( i_fFlareWidth * -0.5f, i_fFlareHeight * 0.5f, 0.0f );
	pDestVerticesFlare[0].vTex = vector2( 0.0f, 0.0f );
	pDestVerticesFlare[1].vPosition = vector3( i_fFlareWidth * 0.5f, i_fFlareHeight * 0.5f, 0.0f );
	pDestVerticesFlare[1].vTex = vector2( 1.0f, 0.0f );
	pDestVerticesFlare[2].vPosition = vector3( i_fFlareWidth * 0.5f, i_fFlareHeight * -0.5f, 0.0f );
	pDestVerticesFlare[2].vTex = vector2( 1.0f, 1.0f );
	pDestVerticesFlare[3].vPosition = vector3( i_fFlareWidth * -0.5f, i_fFlareHeight * -0.5f, 0.0f );
	pDestVerticesFlare[3].vTex = vector2( 0.0f, 1.0f );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hFlare = pResManager->hLoadResource( "Flare.png" );
	if( !m_hFlare )
		return false;

	return true;
}

bool CSphericalLight::bFrameMove()
{
	return false;
}

void CSphericalLight::Render( uint32 i_iPass )
{
	switch( i_iPass )
	{
	case ePass_Leaf: return;
	case ePass_SphericalLight: break;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	float32 fX = 1.2f * sinf( 1.5f * pGraphics->pGetParent()->fGetElapsedTime() );

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matWorld; matMatrix44Translation( matWorld, fX, 0.0f, 0.0f );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShader->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	m_pPixelShader->SetVector( 0, m_vColor );

	m_pVertexShader->SetFloat( 0, 0.0f );
	m_pPixelShader->SetFloat( 0, 0.0f );

	pGraphics->SetVertexFormat( m_pVertexFormatSphere );
	pGraphics->SetVertexStream( 0, m_pVertexBufferSphere, 0, sizeof( vertexformatsphere ) );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->SetPrimitiveAssembler( m_pPrimitiveAssemblerSphere );

	if( !m_iMaxVisiblePixels )
	{
		// determine maximum number of pixels that can be rendered - colorwriteenable is false
		pGraphics->SetRenderState( m3drs_zenable, false );
		pGraphics->SetRenderState( m3drs_colorwriteenable, false );

		pGraphics->pGetM3DDevice()->DrawDynamicPrimitive( 0, m_iNumVertices );

		m_iMaxVisiblePixels = pGraphics->pGetM3DDevice()->iGetRenderedPixels();

		pGraphics->SetRenderState( m3drs_zenable, true );
		pGraphics->SetRenderState( m3drs_colorwriteenable, true );
	}

	pGraphics->pGetM3DDevice()->DrawDynamicPrimitive( 0, m_iNumVertices );

	uint32 iRenderedPixels = pGraphics->pGetM3DDevice()->iGetRenderedPixels();
	if( !iRenderedPixels )
		return;

	// Now render the flare ---------------------------------------------------
	m_pVertexShader->SetFloat( 0, 1.0f );
	m_pPixelShader->SetFloat( 0, 1.0f );

	pGraphics->SetVertexFormat( m_pVertexFormatFlare );
	pGraphics->SetVertexStream( 0, m_pVertexBufferFlare, 0, sizeof( vertexformatflare ) );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pFlare = (CTexture *)pResManager->pGetResource( m_hFlare );
	pGraphics->SetTexture( 0, pFlare->pGetTexture() );

	pGraphics->SetTextureSamplerState( 0, m3dtss_addressu, m3dta_clamp );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressv, m3dta_clamp );

	pGraphics->SetRenderState( m3drs_zenable, false );

	m_pPixelShader->SetFloat( 1, (float32)iRenderedPixels / (float32)m_iMaxVisiblePixels );

	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglefan, 0, 2 );
}
