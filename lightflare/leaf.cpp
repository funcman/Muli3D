
#include "leaf.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CLeafVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		// pass texcoord to pixel shader
		o_pOutput[0] = i_pInput[1];
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector3;
		case 1: return m3dsrt_vector2;
		default: return m3dsrt_unused;
		}
	}
};

class CLeafPS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return true; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		vector4 vLeafColor;
		SampleTexture( vLeafColor, 0, i_pInput[0].x, i_pInput[0].y, 0.0f );
		vVector4Lerp( io_vColor, io_vColor, vLeafColor, vLeafColor.a );
		return ( vLeafColor.a > 0.05f );
	}
};

static m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 1 )
};

CLeaf::CLeaf( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_hTexture = 0;
}

CLeaf::~CLeaf()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hTexture );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CLeaf::bInitialize( float32 i_fWidth, float32 i_fHeight )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * 4 ) ) )
		return false;

	vertexformat *pDestVertices = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDestVertices ) ) )
		return false;

	pDestVertices[0].vPosition = vector3( i_fWidth * -0.5f, i_fHeight * 0.5f, 0.0f );
	pDestVertices[0].vTex = vector2( 0.0f, 0.0f );
	pDestVertices[1].vPosition = vector3( i_fWidth * 0.5f, i_fHeight * 0.5f, 0.0f );
	pDestVertices[1].vTex = vector2( 1.0f, 0.0f );
	pDestVertices[2].vPosition = vector3( i_fWidth * 0.5f, i_fHeight * -0.5f, 0.0f );
	pDestVertices[2].vTex = vector2( 1.0f, 1.0f );
	pDestVertices[3].vPosition = vector3( i_fWidth * -0.5f, i_fHeight * -0.5f, 0.0f );
	pDestVertices[3].vTex = vector2( 0.0f, 1.0f );

	m_pVertexShader = new CLeafVS;
	m_pPixelShader = new CLeafPS;

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hTexture = pResManager->hLoadResource( "MapleLeaves.png" );
	if( !m_hTexture )
		return false;

	return true;
}

bool CLeaf::bFrameMove()
{
	return false;
}

void CLeaf::Render( uint32 i_iPass )
{
	switch( i_iPass )
	{
	case ePass_Leaf: break;
	case ePass_SphericalLight: return;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	float32 fX = 1.3f * sinf( 0.5f * pGraphics->pGetParent()->fGetElapsedTime() );

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matWorld; matMatrix44Translation( matWorld, fX, 0.0f, -1.0f );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShader->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hTexture );
	pGraphics->SetTexture( 0, pTexture->pGetTexture() );

	pGraphics->SetTextureSamplerState( 0, m3dtss_addressu, m3dta_clamp );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressv, m3dta_clamp );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglefan, 0, 2 );
}
